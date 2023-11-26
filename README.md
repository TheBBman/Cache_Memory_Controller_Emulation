Emulation of multi-level cache memory controller in C++.

Cache consists of 16-set direct-mapped L1, 4 entry fully associative victim cache, and 16-set 8-way set associative L2. 

Trace contains memory instruction files to test out the emulator. Each line in a trace has four values (MemR, MemW, adr, data). The first two are either zero or one indicating whether this is a LOAD instruction or STORE (only one of them should be equal to one). The address is a number between 0 and 4095, and the data is a signed integer value. 