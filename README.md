[![Codacy Badge](https://api.codacy.com/project/badge/Grade/f55ad01946e844b7b67c18794ad295c0)](https://app.codacy.com/manual/radiolok/bfutils?utm_source=github.com&utm_medium=referral&utm_content=radiolok/bfutils&utm_campaign=Badge_Grade_Dashboard)

# Brainfuck utils

C++ tools for two home-built Brainfuck computers:

- **BrainfuckPC**: a Von Neumann machine with extended Brainfuck. Code and data share one address space of 64K words × 16 bit. `bfpp`, `bfrun` and `bfloader` target this machine.
- **[DekatronPC](https://github.com/radiolok/dekatronpc)**: a Harvard machine built from dekatrons that runs plain Brainfuck. `dpcrun` is its C++ reference model. The DekatronPC RTL testbench links it as a golden model and checks the RTL against it step by step. bfutils is a git submodule of the dekatronpc repo.

| Tool | Status | Purpose |
|---|---|---|
| [bfpp](#bfpp--compiler) | working | Compiles Brainfuck source into a BrainfuckPC image (binary, text listing or C array) |
| [bfrun](#bfrun--brainfuckpc-emulator) | working | Runs a BrainfuckPC binary image |
| [dpcrun](#dpcrun--dekatronpc-model) | working | DekatronPC golden model: full ISA (Debug + Brainfuck), MemLock, SOT/EOT, resets; also used as a library |
| bfloader | stub | Meant to upload images over RS-232 (`-p port -i file -f`). Right now it only parses the arguments |

## Building

The tools need only a C++11 compiler and CMake 3.10+:

```Shell
git clone https://github.com/radiolok/bfutils.git
cd bfutils
mkdir build && cd build
cmake ..
make
```

The executables end up in `bin/` and the intermediate files in `build/` (both are git-ignored).

Run the tests from the repository root after building:

```Shell
ctest --test-dir build --output-on-failure   # dpcrun unit tests
bash test/test.sh                            # integration test
```

The unit tests (`dpcrun/test/test_dpcrun.cpp`) cover the dpcrun model instruction by instruction. The integration test compiles `helloworld.bfk` and `pi.bfk` with `bfpp`, runs them with `bfrun`, and runs the same sources with `dpcrun`. It checks for `Hello World!` and `3.141` in the output.

GitHub Actions (`.github/workflows/ci.yml`) builds with gcc and clang and runs both.

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
dpcrun -f source.bfk [-b boot.bfk] [-B] [-a 99999] [-n max] [-s]
```

`dpcrun` is the instruction-level golden model of DekatronPC. It mirrors the current RTL (`MachineCtrl`, `IpLine`, `ApLine`), including the places where the RTL and the DekatronPC requirements (TRS) still disagree; those are marked `RTL:` in the source.

What it models:
- **Program memory**: 100 000 4-bit opcodes (never ASCII). Addresses 99900–99999 are the write-protected bootloader ROM.
- **Both ISAs**: decoding is on `{mode, opcode}`; all 32 combinations are covered. Hard Reset starts in Debug ISA at 99900, Soft Reset in Brainfuck ISA at 0.
- **Counters**: IP 0–99999, AP 0–29999 (or up to 99999 with `-a`), data 0–255, loop nesting 0–999. A loop scan stops *on* the matching bracket, which is then executed; nesting past 999 is a hardware error that halts the machine.
- **MemLock**: the data counter holds the cell during `+`/`-`, is flushed on `>`/`<`/`CLRA`/`CLRML`, and the cell is only read when its value is needed (lazy read). `memReads()`/`memWrites()` count the data memory accesses.
- **Program loading**: `SOT` switches to loading, opcodes are written from the next address, `EOT` ends it (Soft Reset or halt).

The command line turns source text into opcodes with the same symbol table as DekatronPC's `generate_rom.py` (`+-<>[].,` plus `N H 0 M G P D B` and the Debug symbols `E S { } L I A R r`), puts the program at 0 followed by HALT, and runs it after a Soft Reset. `-b` loads a bootloader into the ROM, `-B` starts from Hard Reset instead, `-s` traces every instruction to stderr. Program output goes to stdout, followed by `IRET`, the memory access counts and the final status.

It is also a **library** (`dpcmodel` in CMake). `dpcrun/dpcrun.h` provides:
- `dpc::Machine`: the machine. `loadCode()`, `setBootRom()`, `hardReset()`, `softReset()`, `run()`, `step()`, plus getters for every counter and flag. `txData()` is exactly what the RTL drives on `tx_data_bcd`.
- `dpc::Config`: the panel switches (`runOnHardRst`, `softRstOnEot`, `echoMode`, …) and the AP limit.
- `dpc::assemble()`, `opcodeToSymbol()`, `mnemonic()`: the loader and printing helpers.

The standalone `main()` is compiled only with `-DEXEC`. The DekatronPC Verilator testbench (`rtl/tests/DekatronPC.sv/DekatronPC_tb.cpp` in the dekatronpc repo) builds `dpcrun.cpp` without `EXEC`, calls `step()` after every instruction the RTL retires, and compares IRET, IP, AP, `tx_data_bcd`, the loop counter and the terminal output.

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
- **Unused flag:** `bfrun -H` is parsed but does nothing.
- **Section pad byte:** the pad byte in section headers isn't initialised.

## License

BSD 2-Clause, see [LICENSE](LICENSE).
