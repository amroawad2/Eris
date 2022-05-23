#include "VecShiftReg_parsed.h"
#include <string.h>

int main() {
  VecShiftRegister *dut = new VecShiftRegister;
  dut->reset = UInt<1>(1);
  dut->eval(false, true, false);
  for (int i = 0; i < 1; i++) 
      dut->eval(false, true, false);
  dut->reset = UInt<1>(0);
  dut->io_shift = UInt<1>(1);
  dut->io_load = UInt<1>(1);
  dut->io_ins_0 = UInt<4>(0);
  dut->io_ins_1 = UInt<4>(7);
  dut->io_ins_2 = UInt<4>(7);
  dut->io_ins_3 = UInt<4>(5);
  dut->eval(false, true, false);
  // Actual simulation for 10k cycles
  for (int i = 0; i < 24; i++)       
      dut->eval(true, true, true);
  delete dut;
  return 0;
}
