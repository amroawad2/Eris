#ifndef VECSHIFTREGISTER_H_
#define VECSHIFTREGISTER_H_

#include <iostream>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <uint.h>
#include <sint.h>
#define UNLIKELY(condition) __builtin_expect(static_cast<bool>(condition), 0)
using namespace std;

typedef struct VecShiftRegister {
  UInt<4> delays_0;
  UInt<4> delays_1;
  UInt<4> delays_2;
  UInt<4> delays_3;
  UInt<1> clock;
  UInt<1> reset;
  UInt<4> io_ins_0;
  UInt<4> io_ins_1;
  UInt<4> io_ins_2;
  UInt<4> io_ins_3;
  UInt<1> io_load;
  UInt<1> io_shift;
  UInt<4> io_out;

  VecShiftRegister() {
    delays_0.rand_init();
    delays_1.rand_init();
    delays_2.rand_init();
    delays_3.rand_init();
    reset.rand_init();
    io_ins_0.rand_init();
    io_ins_1.rand_init();
    io_ins_2.rand_init();
    io_ins_3.rand_init();
    io_load.rand_init();
    io_shift.rand_init();
    io_out.rand_init();
  }

  void eval(bool update_registers, bool verbose, bool done_reset) {
    UInt<4> _GEN_0 = io_shift ? io_ins_0 : delays_0;
    UInt<4> _GEN_1 = io_shift ? delays_0 : delays_1;
    UInt<4> _GEN_2 = io_shift ? delays_1 : delays_2;
    UInt<4> _GEN_3 = io_shift ? delays_2 : delays_3;
    io_out = delays_3;
    if (update_registers) delays_0 = io_load ? io_ins_0 : _GEN_0;
    if (update_registers) delays_1 = io_load ? io_ins_1 : _GEN_1;
    if (update_registers) delays_2 = io_load ? io_ins_2 : _GEN_2;
    if (update_registers) delays_3 = io_load ? io_ins_3 : _GEN_3;
    if(verbose) cout<<" "<<delays_3<<" "<<delays_2<<" "<<delays_1<<" "<<delays_0<<'\n';
  }
} VecShiftRegister;

#endif  // VECSHIFTREGISTER_H_
