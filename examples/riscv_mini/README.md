## Generate RISCV executable

* Replace *Tile.h* in *top.cc* with *Tile_instr.h* which is essentially the modified header for fault simulation
* Replace <uint.h> with "fault_uint.h"


```bash
Compile command: g++ top.cc mm.cc -o riscv_mini
```

This will create an executable called riscv_mini which can be used for fault injection.

Generate state retaining elements: 
```bash 
./gen_global_regs -i Tile.h
or 
./gen_global_regs_weighted -i Tile.h
```
Generate fault files: 
```bash
fault_file_gen.py -f struct_regs.txt -c <exec_cycles>
```
```bash
Usage: ./riscv_mini "testbench" "fault file"
Example: ./riscv_mini towers.riscv.hex fault_file_0.txt
```


This will cause the faults detailed in fault_file_0.txt to be injected during the runtime of the riscv_mini program.

-----This section can be ignored if you'd just like to run simulation tests---------
* Replace #include <uint.h> and #include <sint.h> with #include "fault_uint.h" and #include "fault_sint.h" respectively
* In top.cpp replace #include <uint.h> with #include "fault_uint.h" and #include <mm.h> with #include "mm.h"
* At the start of main in top.cpp add the line: "fault_file = argv[2];"


**Current manual replacememts will later be automated.
