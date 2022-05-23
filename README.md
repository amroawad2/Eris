# Eris

Eris is a Fault Injection and Tracking Framework for Reliability analysis of RTL designs convertible to FIRRTL. Current support include conversion from **Chisel** to FIRRTL through Chisel driver and **Verilog** and **System Verilog** to FIRRTL from yosys. <br /> 
FIRRTL is a hardware intermediate representation (IR) language for digital circuits designed as a platform for writing circuit-level transformations. More details on FIRRTL can be viewed in the [link here](https://github.com/chipsalliance/firrtl)

## Eris Flow
 
The RTL design is lowered into a FIRRTL representation using the *Chisel* compiler or *Yosys* in case of Verilog and System Verilog. The FIRRTL representation is subsequently lowered to a structured netlist and input to *ESSENT* to build a cycle-accurate C-model of the design. The generated C model is instrumented to enable FI and tracking capabilities according to user-defined fault parameters. The instrumented C model is compiled with a harness to produce an executable that runs fault simulations. With the current applications and benchmarks, we categorize the outcome as benign, silent data corruptions (SDC) and detectable unrecoverable error (DUE) which is currently classified as crash or hang. 

![Eris Tool Flow](https://github.com/shubhamn94/Eris_temp/blob/master/Eris_Flow.png)

## Emit C-model header
*Essent* tool has been added a submodule. Please follow the README instructions provided in the Essent repo to generate C-model header from FIRRTL file. 


## Instrument C-model header
Build the parser and Global register list generator script
```bash
make all
```
We assume that the Essent C-model is already available and got generated from the structured netlist emitted from FIRRTL. Current example is chosen for Rocketchip SoC where instrumented header is generated. 
```bash
./header_parser -i TestHarness.h -o TestHarness_instr.h
```
Replace the Essent headers(*uint.h* & *sint.h*) with  with *"fault_uint.h"* and *"fault_sint.h"* kept in working directory and add print_fault_list() API in a destructor of main class.
Add the following lines at the start of eval() API of instrumented header

## Extract state retaining elements from C-model header
```bash
./gen_global_reglist -i TestHarness.h
```
The emitted file is referred as *struct_regs.txt*


## Generate fault description files
Fault description files can either be manually written or can be auto-generated through following steps. <br />
Prerequsite : python 3.6
```bash
python3 fault_gen_file.py -i struct_regs.txt -f str_idx_map_final.log -h TestHarness.h 
```
Please enter the details as per requirement to generate number of the fault files, type of faults etc. accordingly


## Run fault simulation
The Harness is compiled with C-model header to produce an executable which can take workload or benchmark and the fault description file to run fault simulation. Harness or C-testbench provided by user for the given RTL design. In the current rocketchip example *examples/rocketchip/emulator* is the testbench exercising C-model header *TestHarness*.
Replace the <uint.h> and <sint.h> headers in the instrumented testharness with "fault_uint.h" and "fault_sint.h" and generate the executable with the updated testharness. This compilation command with change with design. <br />
```bash
g++ -I riscv-local/include/ -I../ emulator.cc -o emulator -L riscv-local/lib -Wl,-rpath,riscv-local/lib -lfesvr -lpthread 
```
The generated executable is provided with hex file of RISCV benchmark and fault description file as argument to run fault simulation
```bash
./emulator --cycle-count mt-matmul.riscv fault_file.txt > output.log
```

Multiple simulations can be run in parallel within the available system or separately over HPC cluster with varying benchmarks and thousands of fault description file generated through *fault_gen_file.py*. 

## Analyzing the generated output
The generated output in *output.log* can be:
* benign, 
* have incorrect checksum, essentially, SDC,
* may result into crash or hang, essentially, DUE. Crash could be *segmentation fault* which will trigger SIGSEV handler or exception or,
* may lead to garbled outcome which may generate continuum of random data prints on the output log with/without checksum.  

The checksum is computed by XOR'ing all the program data output. 

### Fault tracking log analysis
Along with the simulation outcome, fault tracking log is generated storing fault propagation information through tracking metadata. The tracking log can be analyzed to form fault tree and Propagation Factor of each node within the tree. Only the P.F of state retaining elements or essentially global or data structure elements are analyzed for vulnerabilities. A simplified example of tree structure is presented below in the figure. It could also have *cyclic dependencies* which is broken down while analysis.

<img src="https://github.com/shubhamn94/Eris_temp/blob/master/fault_tracking_tree.png" width="400" height="200" />

It follows the below equation to determine Propagation Factor. Hotspots are the nodes in the tree having outsized Propagation factor, and is partly denoted as intensity of color of each node in the above figure. 

![Fault Tree Equation](https://github.com/shubhamn94/Eris_temp/blob/master/fault_tree_equation.png)

```bash
Copy the fault_tracking.log to fault_tree_analysis/ 
cd fault_tree_analysis
./automation.sh
```
The propagation factor is stored in *fp_factor.log* corresponding to the unique index of each node for a given fault tree. It should be noted that for simplicity we are injecting only one fault per simulation so that there's no overlapping tree's which could possibly lead to contructive or destructive inteference causing masking effect. The vulnerablility for each node due to a faulty register is analyzed independent of other faults.
Only the P.F of state retaining node present in struct_regs.txt should be considered for vulnerability determination and mitigation.
