# riscv-c2g-table

In RISC-V architecture C extension describes compressed, 16 bit, instruction format.  
Each of these 16 bit instructions map to a full 32 bit one.  
Here i made a set of 3 tables that convert C instructions to their G representation.

## Purpose

Two main uses i had in mind while making these:

 * Lazy emulator writers. Instead of having to grind through the rather convoluted specs of the C extension
one can simply use the provided tables to look up the appropriate 32 bit instruction, thus allowing people to add RVC support in a few lines of code.
 * Testing. At the moment (20180911) there is almost no test coverage for C extension.
There is the rv64uc-p-rvc test which covers the basics and there is the riscv-torture which can produce some C instructions if asked nicely.
Once all these passed, i still had issues with running code and nothing else to help me.
However, since there is a total of 49152 potential C instructions, it is possible to test all of them exhaustively.
More precisely, to test the conversion between them and the G set (for which there exist a rather extensive testing coverage), thus reducing the problem to a solved one.

## Usage

There are 3 tables, one for each quadrant. The first 2 bits of the instruction determine which quadrant it is (with 11 indicating 32 bit instruction).  
To get the G instruction, pick the right quadrant and index with the remaining bits (equivalent to adding inst & 0xFFFC bytes to the start of the table).

The tables are plain numbers rather than, say, macro-composed instructions with named constants, in order to make translation to other languages as simple as a few text replaces.

The riscv_c2g_gen.c file is the actual decoder which was used to generate the tables.  
It's a function that takes a C word and outputs the corresponding G dword, or zero if the instruction is invalid.

## Limitations

 * No FPU instructions. The table was made for use with my emulator, and i haven't implemented F and D extensions yet, so i have no way of testing these.
 * No negative cases. The table should convert all valid instructions correctly, but does not necessarily convert all invalid instructions to an invalid one.

## Correctness

The table was used in my own rv64imacsu variant emulator and it passed all available tests, booted linux and ran a rather extensive buildroot userspace for several hours,
with no errors or anomalous behaviour detected.  
That's about as thorough as i can test it and odds are there are no errors in it, but it's not formally proven.

As stated earlier, there have been next to no negative tests, and the table would likely return correct instructions for some of the incorrect C opcodes.
