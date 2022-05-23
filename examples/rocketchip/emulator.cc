#include <fesvr/dtm.h>
#include <iostream>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "Testharness_instr.h"

//---------------------
using std::string;
//----------------------

// TODO: add in support for JTAG remote bit bang
// TODO: add in support for VM_TRACE and VCD output

// For option parsing, which is split across this file, Verilog, and
// FESVR's HTIF, a few external files must be pulled in. The list of
// files and what they provide is enumerated:
//
// $RISCV/include/fesvr/htif.h:
//   defines:
//     - HTIF_USAGE_OPTIONS
//     - HTIF_LONG_OPTIONS_OPTIND
//     - HTIF_LONG_OPTIONS
// $(ROCKETCHIP_DIR)/generated-src(-debug)?/$(CONFIG).plusArgs:
//   defines:
//     - PLUSARG_USAGE_OPTIONS
//   variables:
//     - static const char * verilog_plusargs

dtm_t* dtm;

static uint64_t trace_count = 0;
bool verbose;
bool done_reset;

void handle_sigterm(int sig) {
  dtm->stop();
}

void tick_dtm(TestHarness *tile, bool done_reset) {
  if (done_reset) {
    dtm_t::resp resp_bits;
    resp_bits.resp = tile->SimDTM$$inst.debug_resp_bits_resp.as_single_word();
    resp_bits.data = tile->SimDTM$$inst.debug_resp_bits_data.as_single_word();

    dtm->tick(tile->SimDTM$$inst.debug_req_ready.as_single_word(),
              tile->SimDTM$$inst.debug_resp_valid.as_single_word(),
              resp_bits);

    tile->SimDTM$$inst.debug_resp_ready = UInt<1>(dtm->resp_ready());
    tile->SimDTM$$inst.debug_req_valid = UInt<1>(dtm->req_valid());
    tile->SimDTM$$inst.debug_req_bits_addr = UInt<7>(dtm->req_bits().addr);
    tile->SimDTM$$inst.debug_req_bits_op = UInt<2>(dtm->req_bits().op);
    tile->SimDTM$$inst.debug_req_bits_data = UInt<32>(dtm->req_bits().data);

    tile->SimDTM$$inst.exit = UInt<32>(dtm->done() ? (dtm->exit_code() << 1 | 1) : 0);
  } else {
    tile->SimDTM$$inst.debug_req_valid = UInt<1>(0);
    tile->SimDTM$$inst.debug_resp_ready = UInt<1>(0);
    tile->SimDTM$$inst.exit = UInt<32>(0);
  }
}

static void usage(const char * program_name) {
  printf("Usage: %s [EMULATOR OPTION]... [VERILOG PLUSARG]... [HOST OPTION]... BINARY [TARGET OPTION]...\n",
         program_name);
  fputs("\
Run a BINARY on the Rocket Chip emulator.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
\n\
EMULATOR OPTIONS\n\
  -c, --cycle-count        Print the cycle count before exiting\n\
       +cycle-count\n\
  -h, --help               Display this help and exit\n\
  -m, --max-cycles=CYCLES  Kill the emulation after CYCLES\n\
       +max-cycles=CYCLES\n\
  -s, --seed=SEED          Use random number seed SEED\n\
  -V, --verbose            Enable all Chisel printfs (cycle-by-cycle info)\n\
       +verbose\n\
", stdout);
  fputs("\
  -v, --vcd=FILE,          Write vcd trace to FILE (or '-' for stdout)\n\
  -x, --dump-start=CYCLE   Start VCD tracing at CYCLE\n\
       +dump-start\n\
", stdout);
  // fputs("\n" PLUSARG_USAGE_OPTIONS, stdout);
  fputs("\n" HTIF_USAGE_OPTIONS, stdout);
  printf("\n"
"EXAMPLES\n"
"  - run a bare metal test:\n"
"    %s $RISCV/riscv64-unknown-elf/share/riscv-tests/isa/rv64ui-p-add\n"
"  - run a bare metal test showing cycle-by-cycle information:\n"
"    %s +verbose $RISCV/riscv64-unknown-elf/share/riscv-tests/isa/rv64ui-p-add 2>&1 | spike-dasm\n"
"  - run an ELF (you wrote, called 'hello') using the proxy kernel:\n"
"    %s pk hello\n",
         program_name, program_name, program_name);
}

int main(int argc, char** argv) {
  unsigned random_seed = (unsigned)time(NULL) ^ (unsigned)getpid();
  uint64_t max_cycles = -1;
  int ret = 0;
  bool print_cycles = false;
  char ** htif_argv = NULL;
  int verilog_plusargs_legal = 1;

  char* ptr;
  //--------------------
  fault_file = argv[3]; //TODO:Consider other options being used
  //unsigned long idx = strtol(argv[4], &ptr, 10);
  //int start_cycle = atoi(argv[5]);
  //int end_cycle = atoi(argv[6]);
  //int width = atoi(argv[7]);
  //last_cycle = std::stoi(argv[3]);
  //open_golden();
  //--------------------
  
  while (1) {
    static struct option long_options[] = {
      {"cycle-count", no_argument,       0, 'c' },
      {"help",        no_argument,       0, 'h' },
      {"max-cycles",  required_argument, 0, 'm' },
      {"seed",        required_argument, 0, 's' },
      {"verbose",     no_argument,       0, 'V' },
      HTIF_LONG_OPTIONS
    };
    int option_index = 0;
    int c = getopt_long(argc, argv, "-chm:s:V", long_options, &option_index);
    if (c == -1) break;
 retry:
    switch (c) {
      // Process long and short EMULATOR options
      case '?': usage(argv[0]);             return 1;
      case 'c': print_cycles = true;        break;
      case 'h': usage(argv[0]);             return 0;
      case 'm': max_cycles = atoll(optarg); break;
      case 's': random_seed = atoi(optarg); break;
      case 'V': verbose = true;             break;
#if VM_TRACE
      case 'v': {
        vcdfile = strcmp(optarg, "-") == 0 ? stdout : fopen(optarg, "w");
        if (!vcdfile) {
          std::cerr << "Unable to open " << optarg << " for VCD write\n";
          return 1;
        }
        break;
      }
      case 'x': start = atoll(optarg);      break;
#endif
      // Process legacy '+' EMULATOR arguments by replacing them with
      // their getopt equivalents
      case 1: {
        std::string arg = optarg;
        if (arg.substr(0, 1) != "+") {
          optind--;
          goto done_processing;
        }
        if (arg == "+verbose")
          c = 'V';
        else if (arg.substr(0, 12) == "+max-cycles=") {
          c = 'm';
          optarg = optarg+12;
        }
        else if (arg.substr(0, 12) == "+cycle-count")
          c = 'c';
        // If we don't find a legacy '+' EMULATOR argument, it still could be
        // a VERILOG_PLUSARG and not an error.
        // else if (verilog_plusargs_legal) {
        //   const char ** plusarg = &verilog_plusargs[0];
        //   int legal_verilog_plusarg = 0;
        //   while (*plusarg && (legal_verilog_plusarg == 0)){
        //     if (arg.substr(1, strlen(*plusarg)) == *plusarg) {
        //       legal_verilog_plusarg = 1;
        //     }
        //     plusarg ++;
        //   }
        //   if (!legal_verilog_plusarg) {
        //     verilog_plusargs_legal = 0;
        //   } else {
        //     c = 'P';
        //   }
        //   goto retry;
        // }
        // If we STILL don't find a legacy '+' argument, it still could be
        // an HTIF (HOST) argument and not an error. If this is the case, then
        // we're done processing EMULATOR and VERILOG arguments.
        else {
          static struct option htif_long_options [] = { HTIF_LONG_OPTIONS };
          struct option * htif_option = &htif_long_options[0];
          while (htif_option->name) {
            if (arg.substr(1, strlen(htif_option->name)) == htif_option->name) {
              optind--;
              goto done_processing;
            }
            htif_option++;
          }
          std::cerr << argv[0] << ": invalid plus-arg (Verilog or HTIF) \""
                    << arg << "\"\n";
          c = '?';
        }
        goto retry;
      }
      case 'P': break; // Nothing to do here, Verilog PlusArg
      // Realize that we've hit HTIF (HOST) arguments or error out
      default:
        if (c >= HTIF_LONG_OPTIONS_OPTIND) {
          optind--;
          goto done_processing;
        }
        c = '?';
        goto retry;
    }
  }

done_processing:
  if (optind == argc) {
    std::cerr << "No binary specified for emulator\n";
    usage(argv[0]);
    return 1;
  }
  int htif_argc = 1 + argc - optind;
  htif_argv = (char **) malloc((htif_argc) * sizeof (char *));
  htif_argv[0] = argv[0];
  for (int i = 1; optind < argc;) htif_argv[i++] = argv[optind++];

  if (verbose)
    fprintf(stderr, "using random seed %u\n", random_seed);

  srand(random_seed);
  srand48(random_seed);

  printf("Before Rocket init\n");
  std::cout << sizeof(TestHarness) << "\n";
  //printf("After sizeof\n");
  //return 0;
  TestHarness *tile = new TestHarness;
  printf("After Rocket Init\n");
  //load_faults();
  
    //return 0;
    
  dtm = new dtm_t(htif_argc, htif_argv);

  signal(SIGTERM, handle_sigterm);

  // The initial block in AsyncResetReg is either racy or is not handled
  // correctly by Verilator when the reset signal isn't a top-level pin.
  // So guarantee that all the AsyncResetRegs will see a rising edge of
  // the reset signal instead of relying on the initial block.
  // ESSENT currently does not treat Async differently
  int async_reset_cycles = 2;

  // Rocket-chip requires synchronous reset to be asserted for several cycles.
  int sync_reset_cycles = 10;

  while (trace_count < max_cycles) {
    if (done_reset && (dtm->done() || tile->io_success))
      break;
    tile->reset = UInt<1>(trace_count < async_reset_cycles + sync_reset_cycles);
    done_reset = !tile->reset;
    tile->eval(true, verbose, done_reset);
    tick_dtm(tile, done_reset);
    trace_count++;
  }
  
  //std::cout << trace_count << "\n";

  if (dtm->exit_code()) {
    fprintf(stderr, "*** FAILED *** via dtm (code = %d, seed %d) after %" PRIu64 " cycles\n", dtm->exit_code(), random_seed, trace_count);
    ret = dtm->exit_code();
  }
  else if (trace_count == max_cycles) {
    fprintf(stderr, "*** FAILED *** via trace_count (timeout, seed %d) after %" PRIu64 " cycles\n", random_seed, trace_count);
    ret = 2;
  }
  else if (verbose || print_cycles) {
    fprintf(stderr, "Completed after %" PRIu64 " cycles\n", trace_count);
  }

  if (dtm) delete dtm;
  if (tile) delete tile;
  if (htif_argv) free(htif_argv);
  return ret;
}
