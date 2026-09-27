[![Codacy Badge](https://api.codacy.com/project/badge/Grade/f55ad01946e844b7b67c18794ad295c0)](https://app.codacy.com/manual/radiolok/bfutils?utm_source=github.com&utm_medium=referral&utm_content=radiolok/bfutils&utm_campaign=Badge_Grade_Dashboard)

# Brainfuck utils

C++ tools for two home-built Brainfuck computers:

- **BrainfuckPC**: a Von Neumann machine with extended Brainfuck. Code and data share one address space of 64K words × 16 bit. `bfpp`, `bfrun` and `bfloader` target this machine.
- **[DekatronPC](https://github.com/radiolok/dekatronpc)**: a Harvard machine built from dekatrons that runs plain Brainfuck. `dpcrun` is its C++ reference model. The DekatronPC RTL testbench links it as a golden model and checks the RTL against it step by step. bfutils is a git submodule of the dekatronpc repo.

| Tool | Status | Purpose |
|---|---|---|
| [bfpp](#bfpp--compiler) | working | Compiles Brainfuck source into a BrainfuckPC image (binary, text listing or C array) |
| [bfrun](#bfrun--brainfuckpc-emulator) | working | Runs a BrainfuckPC binary image |
| [dpcrun](#dpcrun--dekatronpc-model) | working | Interprets Brainfuck source with DekatronPC semantics; also used as a library |
| bfloader | stub | Meant to upload images over RS-232 (`-p port -i file -f`). Right now it only parses the arguments |

## Building

The tools need only a C++11 compiler and CMake 3.1+:

```Shell
git clone https://github.com/radiolok/bfutils.git
cd bfutils
mkdir build && cd build
cmake ..
make
```

The executables end up in `bin/` and the intermediate files in `build/` (both are git-ignored).

Run the integration test from the repository root after building:

```Shell
bash test/test.sh
```

The test compiles `helloworld.bfk` and `pi.bfk` with `bfpp`, runs them with `bfrun`, and runs the same sources with `dpcrun`. It checks for `Hello World!` and `3.141` in the output.

## bfpp — compiler

```
bfpp -i source.bf [-o output] [-s [-d [-l]] | -H] [-O1] [-e]
  -i  Brainfuck source file (required)
  -o  output file (defaults: a.out for binary, a.asm for text, a.hex for hex)
  -s  write a text listing instead of a binary image
  -d  add pseudo-code comments to the text listing
  -l  indent the listing comments by loop depth
  -H  write the program as a C array: uint16_t application[] = {...};
  -O1 replace [-] and [+] with a single "clear cell" instruction
  -e  extended instruction set (not implemented)
```

The compiler works in these steps:
1. **Strip:** remove every character except `> < + - . , [ ]` and `~`.
2. **Translate:** merge runs of `>`, `<`, `+` and `-` into one instruction with a count. For example, `+++` becomes `*AP += 3`.
3. **Add a prologue and epilogue:** a NOP and an AP-adjust instruction are placed in front of the program, and a HALT after it.
4. **Optimise:** with `-O1`, replace `[-]`/`[+]` with a clear-cell instruction.
5. **Link:** resolve every `[` and `]` to a relative jump to its matching bracket.

Example listing (`bfpp -s -d -O1` on `++[->+<]>[-].`):

```
IP:0x0001   CMD:0x2002 *AP += 2
IP:0x0002   CMD:0x6005 (*AP==0)? IP += 5: PASS
IP:0x0003   CMD:0x3fff *AP -= 1
...
IP:0x0009   CMD:0x1010 *AP = 0
IP:0x000a   CMD:0x1002 putc
```

## Instruction encoding (BrainfuckPC)

Each instruction is one 16-bit word:
- Bits `[15:12]` hold the opcode.
- Bits `[12:0]` hold a signed 13-bit count or jump offset. Bit 12 is the sign bit, so a negative value moves the opcode up by one: `ADD` becomes `SUB`, `RIGHT` becomes `LEFT`, and so on.

| Word | Mnemonic | Brainfuck | Meaning |
|---|---|---|---|
| `0x0000` | NOP | | no operation |
| `0x1002` | IO putc | `.` | output `*AP` |
| `0x1001` | IO getc | `,` | input into `*AP` |
| `0x1010` | IO clr.data | `[-]` with `-O1` | `*AP = 0` |
| `0x1800` | IO halt | | stop |
| `0x2nnn` / `0x3nnn` | ADD / SUB | `+` / `-` | `*AP += n` (the SUB range carries a negative n) |
| `0x4nnn` / `0x5nnn` | RIGHT / LEFT | `>` / `<` | `AP += n` |
| `0x6nnn` / `0x7nnn` | JZ | `[` | `if (*AP == 0) IP += n` |
| `0x8nnn` / `0x9nnn` | JNZ | `]` | `if (*AP != 0) IP += n` (n is negative) |

Other IO sub-codes that `bfrun` recognises:

| Word | Meaning |
|---|---|
| `0x1200` | 8-bit mode |
| `0x1400` | 16-bit mode |
| `0x1100` | pause |

`common/bfutils.h` also reserves `0xAnnn` for XOR and `0xF000` for HALT, but neither is used.

## Binary image format

All 16-bit fields are big-endian.

The file starts with a header (10 bytes):

```C
uint16_t Magic;       // "BF" = 0x4246
uint8_t  Machine;     // 0 = 8-bit data, 1 = 16-bit data
uint8_t  HeaderSize;  // bytes: header + all section headers
uint8_t  SectionNum;
uint8_t  flags;
uint16_t IpEntry;     // IP after loading
uint16_t ApEntry;     // AP after loading
```

`SectionNum` section headers follow, 10 bytes each, and then the section data:

```C
uint16_t FileBase;    // offset of the section data in the file
uint16_t MemoryBase;  // load address, in words
uint16_t FileSize;    // bytes of data stored in the file
uint16_t MemorySize;  // words to reserve in memory
uint8_t  pad;         // written before `type` on disk
uint8_t  type;        // 1 = code, 2 = data
```

`bfpp` always writes two sections:
- **Code** at address 0.
- **Data** right after the code. The data section has no bytes in the file and fills the rest of the 64K space.

`ApEntry` is set to the middle of memory: `0x7FFF - code_size/2`.

## bfrun — BrainfuckPC emulator

```
bfrun -f image.out [-x] [-s] [-d]
  -x  16-bit mode: jumps test all 16 bits of *AP (default: the low 8 bits)
  -s  print the number of instructions retired
  -d  trace every instruction to stderr (IP, AP, *AP)
  -p  protected mode (not implemented)
```

`bfrun` loads every section into one 64K-word memory and starts at `IpEntry`/`ApEntry`. It stops when IP runs past the end of the code section. Program output goes to stdout.

## dpcrun — DekatronPC model

```
dpcrun -f source.bf
```

Unlike `bfrun`, `dpcrun` reads **Brainfuck source directly**; no `bfpp` step is needed. It follows DekatronPC's model:
- 30000 data cells of 8 bits each.
- A loop counter with a range of 0–999. It counts nesting depth while the model searches for a matching bracket.
- The character `0` is accepted as "clear cell".

Program output goes to **stderr**, so it doesn't mix with a testbench log. At the end the model prints `IRET:<instructions retired>` to stdout.

It can also be used as a **library**. `dpcrun/dpcrun.h` provides:
- `CppMachine`: holds the code memory, the data memory, the loop counter, and the `IRET`/`CLK_UNHALTED` counters.
- `stepCpp()`: executes one instruction.

The standalone `main()` is compiled only with `-DEXEC`, which CMake sets for the `dpcrun` target. The DekatronPC Verilator testbench (`rtl/tests/DekatronPC.sv/DekatronPC_tb.cpp` in the dekatronpc repo) builds `dpcrun.cpp` without `EXEC`. It calls `stepCpp()` after every instruction the RTL retires and compares IP, AP, the data value and the loop counter.

## Sample programs

These are in `common/`:
- `helloworld.bfk`
- `pi.bfk`
- `fibonacci.bfk`
- `fractal.bfk`
- `ctrlio_data_clr.bfk`: exercises `[-]`, which becomes clr.data with `-O1`.

## Known issues

- **Wrong AP prologue:** `bfpp` means to add `AP += 0xFF00`, but the value doesn't fit in the 13-bit field and is encoded as `AP -= 256` (`0x5F00`). The `-d` listing still prints `AP += 256`.
- **`-O1` drops the prologue and HALT:** the leading NOP and trailing HALT are lost, and the compiler prints `Invalid opcode` warnings for them.
- **`-d` listing warnings:** NOP and HALT have no pseudo-code text, so the listing prints `Unknown Opcode` for them.
- **HALT is ignored:** `bfrun` doesn't act on HALT (`0x1800`) when running an image; it stops only at the end of the code section.
- **Unused flags:** `bfrun -H` and `dpcrun -s` are parsed but do nothing.
- **Section pad byte:** the pad byte in section headers isn't initialised.
- **Dead CI:** `.travis.yml` is left over from Travis CI (travis-ci.org), which no longer runs.

## License

BSD 2-Clause, see [LICENSE](LICENSE).
