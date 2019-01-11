[![Build Status](https://travis-ci.org/radiolok/bfutils.svg?branch=master)](https://travis-ci.org/radiolok/bfutils)

# Brainfuck utils

Brainfuck programmig tools for Von-Neumann extended brainfuck computer.

Shared address space, 64 KWords * 16 bit each Word

# bfpp

Brainfuck compiler. Usage:
```
bfpp -i source_file [-o output_file] [-e] [-s [-d]]
-i - path to brainfuck source file
-o - path to output file. By default: a.out for binary, a.asm for text
-e - enabling extended instruction set [CURRENTLY UNSUPPORTED]
-s - save binary as text
-d - add debug symbols into text output
```
# bfrun
Brainfuck emulator. Usage:
```
bfrun -f BF_binary [-p -x]
-p - switch emulator to protected mode. AP and IP should be in their sections
-s - switch emulator to 16 bit mode. By default 8-bit classic mode.
```

# Binary

It is a binary Big-Endian uint16_t file which contain BfHeader and two sections - code section and data section. 

# BF Header

```
uint16_t Magic = "BF";
uint16_t ImageBase;//Address Of Image Started
struct{
  uint16_t Pos;//Position of Code segment. After loading IP = Pos
  uint16_t Length;//Length of Segment to load
}Code;
struct{
  uint16_t Pos;//Position of Data segment. After loading AP = Pos
  uint16_t Length;//Length of Segment to load
}Data;
```

# bfutils building using cmake

Currently bfutils doesn't require any external libraries. Just clone repo and build it with CMake:

```
git clone https://github.com/radiolok/bfutils.git
mkdir bfutils/build
cd bfutils/build
cmake ../
```
