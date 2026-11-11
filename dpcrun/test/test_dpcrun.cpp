// Unit tests for the DekatronPC golden model (dpcrun).
//
// Each test names the TRS requirement it covers. Tests marked "RTL:" pin
// behaviour where the model follows the current RTL rather than the TRS
// text; if the RTL is changed, change the test together with the model.
//
// No framework on purpose: the file builds with a bare C++11 compiler.

#include "dpcrun.h"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

using namespace dpc;

//----------------------------------------------------------------------
// Minimal harness
//----------------------------------------------------------------------
namespace {

struct TestCase {
    const char* name;
    void (*fn)();
};

std::vector<TestCase>& registry()
{
    static std::vector<TestCase> r;
    return r;
}

struct Registrar {
    Registrar(const char* name, void (*fn)()) { registry().push_back({name, fn}); }
};

int g_failures = 0;
int g_checks   = 0;

#define TEST(name)                                           \
    static void name();                                      \
    static Registrar name##_reg(#name, name);                \
    static void name()

#define CHECK(cond)                                                          \
    do {                                                                     \
        ++g_checks;                                                          \
        if (!(cond)) {                                                       \
            ++g_failures;                                                    \
            std::printf("  %s:%d: CHECK(%s) failed\n", __FILE__, __LINE__,   \
                        #cond);                                              \
        }                                                                    \
    } while (0)

#define CHECK_EQ(a, b)                                                       \
    do {                                                                     \
        ++g_checks;                                                          \
        long long va_ = static_cast<long long>(a);                           \
        long long vb_ = static_cast<long long>(b);                           \
        if (va_ != vb_) {                                                    \
            ++g_failures;                                                    \
            std::printf("  %s:%d: CHECK_EQ(%s, %s) failed: %lld != %lld\n",  \
                        __FILE__, __LINE__, #a, #b, va_, vb_);               \
        }                                                                    \
    } while (0)

// Machine after Soft Reset + Run with `src` at address 0 (BF ISA).
Machine bfMachine(const std::string& src, Config cfg = Config())
{
    Machine m(cfg);
    m.loadCode(assemble(src));
    m.softReset();
    m.run();
    return m;
}

// Runs `n` instructions, each expected to retire without stopping.
void steps(Machine& m, int n)
{
    for (int i = 0; i < n; ++i)
        CHECK(m.step() == Status::Ok);
}

std::string readFile(const std::string& path)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

const char* const BOOTLOADER = "0<{0<}NNNS";   // rtl/programs/bootloader.bfk

} // namespace

//======================================================================
// Loader (REQ-IPV2-005, rtl/run/generate_rom.py table)
//======================================================================

TEST(loader_symbol_table)
{
    std::vector<uint8_t> c = assemble("NH+-><[].,0SAP{}");
    CHECK_EQ(c.size(), 16u);
    for (size_t i = 0; i < c.size(); ++i)
        CHECK_EQ(c[i], i);
    CHECK_EQ(symbolToOpcode('E'), OP_LD);       // EOT is LD too
    // Codes of the old two-mode ISA are gone
    for (char g : std::string("\aDBMGLIRr"))
        CHECK_EQ(symbolToOpcode(g), -1);
}

TEST(loader_skips_comments_keeps_nop)
{
    // 'N' is opcode 0 and must not be dropped as "false"
    std::vector<uint8_t> c = assemble("a+ b\nN-\t# x");
    std::vector<uint8_t> want = {OP_INC, OP_NOP, OP_DEC};
    CHECK(c == want);
}

TEST(opcode_to_symbol_roundtrip)
{
    for (uint8_t op = 0; op < 16; ++op)
        CHECK_EQ(symbolToOpcode(opcodeToSymbol(op)), op);
    CHECK(std::string(mnemonic(0xB)) == "LD");
    CHECK(std::string(mnemonic(0xD)) == "MEMLOCK");
}

//======================================================================
// Power-on and resets (REQ-RST-*)
//======================================================================

TEST(power_on_state)
{
    Machine m;
    CHECK(m.halted());
    CHECK_EQ(m.ip(), 0u);
    CHECK_EQ(m.iret(), 0u);
    CHECK(m.step() == Status::Halted);
}

TEST(hard_reset_state)
{
    // REQ-RST-001/003, REQ-BOOT-002: IP = 99990
    Machine m = bfMachine("+++>++");
    steps(m, 5);
    m.hardReset();
    CHECK_EQ(m.ip(), BOOT_BASE);
    CHECK_EQ(m.ip(), 99990u);
    CHECK_EQ(m.ap(), 0u);
    CHECK_EQ(m.dataCounter(), 0u);
    CHECK_EQ(m.loopCount(), 0u);
    CHECK(!m.memLock());
    CHECK(m.halted());          // RunOnHardRst = 0
    CHECK_EQ(m.iret(), 5u);     // IRET resets only with rst_n
}

TEST(soft_reset_state)
{
    // REQ-RST-012
    Config cfg;
    cfg.runOnSoftRst = true;
    Machine m = bfMachine("+>", cfg);
    steps(m, 2);
    m.softReset();
    CHECK_EQ(m.ip(), 0u);
    CHECK_EQ(m.ap(), 0u);
    CHECK(!m.halted());         // RunOnSoftRst = 1
}

TEST(reset_drops_locked_counter)
{
    // The data counter is not written back on reset; memory is retained
    Machine m = bfMachine("+>+++");
    steps(m, 5);
    CHECK(m.memLock());
    m.softReset();
    CHECK_EQ(m.cell(0), 1u);    // flushed by '>'
    CHECK_EQ(m.cell(1), 0u);    // 3 was only in the counter
}

TEST(reset_clears_loop_overflow)
{
    // No CLRL in ISA16: only a reset clears the overflow
    std::string src = std::string(100, '[') + std::string(100, ']');
    Machine m = bfMachine(src);
    m.runUntilHalt();
    CHECK(m.loopOverflow());
    m.softReset();
    CHECK(!m.loopOverflow());
    CHECK_EQ(m.loopCount(), 0u);
}

TEST(first_fetch_after_reset_does_not_increment)
{
    Machine m = bfMachine("+H");
    steps(m, 1);
    CHECK_EQ(m.ip(), 0u);
    CHECK_EQ(m.insn(), OP_INC);
    CHECK_EQ(m.cellValue(), 1u);
}

//======================================================================
// Instruction table (REQ-ISA-*, doc/isa16.md)
//======================================================================

TEST(halt_advances_ip)
{
    // After HALT at N, IpLine steps to N+1 so Run continues from there
    Machine m = bfMachine("NH+H");
    CHECK(m.step() == Status::Ok);
    CHECK(m.step() == Status::Halted);
    CHECK(m.halted());
    CHECK_EQ(m.ip(), 2u);
    CHECK_EQ(m.iret(), 2u);
    m.run();
    CHECK(m.step() == Status::Ok);
    CHECK_EQ(m.ip(), 2u);
    CHECK_EQ(m.cellValue(), 1u);
    CHECK(m.step() == Status::Halted);
}

TEST(nop_codes)
{
    // NOP and { } decode as nothing: they change nothing but IP/IRET
    // ({ at AP = 0 is followed by its } so that its later scan ends)
    const uint8_t nops[] = {OP_NOP, OP_LABEG, OP_LAEND};
    for (uint8_t op : nops) {
        Machine m;
        m.loadCode({op, OP_LAEND});
        m.softReset();
        m.run();
        steps(m, 1);
        CHECK(!m.memLock());
        CHECK(!m.loading());
        CHECK(!m.halted());
        CHECK_EQ(m.ap(), 0u);
        CHECK_EQ(m.bells(), 0u);
    }
}

TEST(all_16_codes_decode)
{
    // Every opcode retires or halts; nothing throws or hangs
    for (uint8_t op = 0; op < 16; ++op) {
        Machine m;
        m.loadCode({op, OP_HALT, OP_LAEND, OP_LEND});
        m.softReset();
        m.run();
        m.pushInput('x');
        Status s = m.runUntilHalt(10);
        CHECK(s == Status::Halted || s == Status::WaitInput || s == Status::Ok);
        CHECK(m.iret() >= 1u);
    }
}

//======================================================================
// Data and address (REQ-ISA-BF-*, REQ-ML-*)
//======================================================================

TEST(data_wraps_0_255)
{
    Machine m = bfMachine("-");
    steps(m, 1);
    CHECK_EQ(m.dataCounter(), 255u);
    Machine m2 = bfMachine("");
    m2.setCell(0, 255);
    m2.loadCode(assemble("+"));
    steps(m2, 1);
    CHECK_EQ(m2.dataCounter(), 0u);
}

TEST(ap_wraps_at_top)
{
    Machine m = bfMachine("<><");
    steps(m, 1);
    CHECK_EQ(m.ap(), AP_TOP_BF);
    steps(m, 1);
    CHECK_EQ(m.ap(), 0u);

    Config cfg;
    cfg.apTop = 99999;   // OPEN-001 alternative
    Machine m2 = bfMachine("<", cfg);
    steps(m2, 1);
    CHECK_EQ(m2.ap(), 99999u);
}

TEST(memlock_taken_before_first_step)
{
    // REQ-ML-001/002: one read, then + - work in the counter only
    Machine m = bfMachine("++-+");
    m.setCell(0, 10);
    steps(m, 4);
    CHECK(m.memLock());
    CHECK_EQ(m.dataCounter(), 12u);
    CHECK_EQ(m.cell(0), 10u);   // not written back yet
    CHECK_EQ(m.cellValue(), 12u);
    CHECK_EQ(m.memReads(), 1u);
    CHECK_EQ(m.memWrites(), 0u);
}

TEST(ap_move_flushes_locked_counter)
{
    // REQ-ML-005: the counter goes into the OLD cell before AP steps
    Machine m = bfMachine("++>+<");
    steps(m, 3);
    CHECK_EQ(m.cell(0), 2u);
    CHECK_EQ(m.ap(), 1u);
    CHECK(!m.memLock());
    steps(m, 2);
    CHECK_EQ(m.cell(1), 1u);
    CHECK_EQ(m.ap(), 0u);
    CHECK_EQ(m.cellValue(), 2u);
}

TEST(lazy_read_pointer_moves_do_not_touch_memory)
{
    Machine m = bfMachine(">>>>><<");
    steps(m, 7);
    CHECK_EQ(m.memReads(), 0u);
    CHECK_EQ(m.memWrites(), 0u);
    CHECK_EQ(m.ap(), 3u);
}

TEST(tx_shows_counter_after_move)
{
    // tx_data_bcd is always the data counter; after a move it still holds
    // the previous cell until COUT (or +) loads the new one
    Machine m = bfMachine("+++>");
    steps(m, 4);
    CHECK_EQ(m.txData(), 3u);
    CHECK_EQ(m.cellValue(), 0u);
}

TEST(cout_loads_cell_into_counter)
{
    // OPEN-017: without MemLock COUT loads the cell into the counter,
    // reading it only if the memory register is on another cell; MemLock
    // stays as it was
    Machine m = bfMachine("+.>.<..");
    m.setCell(0, 64);
    m.setCell(1, 'B');
    steps(m, 3);                        // + . >  (flush of 'A')
    CHECK(!m.memLock());
    uint64_t r0 = m.memReads();
    steps(m, 1);                        // . at cell 1: read, load
    CHECK_EQ(m.memReads(), r0 + 1);
    CHECK_EQ(m.txData(), uint32_t('B'));
    CHECK(!m.memLock());
    steps(m, 2);                        // < .  : read cell 0 again
    CHECK_EQ(m.memReads(), r0 + 2);
    CHECK_EQ(m.txData(), uint32_t('A'));
    steps(m, 1);                        // . : register on the cell, no read
    CHECK_EQ(m.memReads(), r0 + 2);
    CHECK(m.output() == "ABAA");
}

TEST(clrd_sets_memlock)
{
    // REQ-ML-009: [-] must really clear the cell after a move
    Machine m = bfMachine("0>");
    m.setCell(0, 42);
    steps(m, 1);
    CHECK(m.memLock());
    CHECK_EQ(m.cellValue(), 0u);
    steps(m, 1);
    CHECK_EQ(m.cell(0), 0u);
}

TEST(store_copies_cell)
{
    // MemLock takes over the current cell with the counter's old value
    // without a load; the next move writes it there: '>' MemLock '<' copies
    Machine m = bfMachine(".>P<");
    m.setCell(0, 99);
    steps(m, 3);
    CHECK(m.memLock());
    CHECK_EQ(m.cellValue(), 99u);
    CHECK_EQ(m.memWrites(), 0u);        // MemLock itself does not write
    steps(m, 1);
    CHECK_EQ(m.cell(1), 99u);
    CHECK_EQ(m.cell(0), 99u);
    CHECK(!m.memLock());
    CHECK_EQ(m.memWrites(), 1u);
}

TEST(store_with_lock_changes_nothing)
{
    Machine m = bfMachine("+++P>");
    steps(m, 4);
    CHECK(m.memLock());
    CHECK_EQ(m.memWrites(), 0u);
    steps(m, 1);
    CHECK_EQ(m.cell(0), 3u);
    CHECK_EQ(m.memWrites(), 1u);
}

TEST(cout_outputs_cell)
{
    // REQ-ISA-BF-005: . reads the counter under MemLock, memory otherwise
    Machine m = bfMachine("+.>.");
    m.setCell(0, 64);
    m.setCell(1, 'B');
    steps(m, 4);
    CHECK(m.output() == "AB");
}

TEST(cin_sets_memlock_and_echoes)
{
    Machine m = bfMachine(",>");
    m.pushInput('q');
    steps(m, 1);
    CHECK(m.memLock());
    CHECK_EQ(m.dataCounter(), 'q');
    CHECK(m.output() == "q");           // EchoMode = 1
    steps(m, 1);
    CHECK_EQ(m.cell(0), 'q');
}

TEST(cin_waits_for_input)
{
    Config cfg;
    cfg.echoMode  = false;
    cfg.bellOnCin = true;
    Machine m = bfMachine(",H", cfg);
    CHECK(m.step() == Status::WaitInput);
    CHECK_EQ(m.iret(), 1u);             // decoded, waiting in S_CIN_WAIT
    CHECK(m.step() == Status::WaitInput);
    m.pushInput(5);
    CHECK(m.step() == Status::Ok);
    CHECK_EQ(m.iret(), 1u);
    CHECK_EQ(m.bells(), 1u);
    CHECK(m.output().empty());
    CHECK_EQ(m.cellValue(), 5u);
}

TEST(clra)
{
    // Locked counter is flushed to the old cell first
    Machine m = bfMachine(">>++A");
    steps(m, 5);
    CHECK_EQ(m.ap(), 0u);
    CHECK_EQ(m.cell(2), 2u);
    CHECK(!m.memLock());
}

TEST(clra_after_store_writes_copy)
{
    Machine m = bfMachine("+>PA");
    steps(m, 4);
    CHECK_EQ(m.ap(), 0u);
    CHECK(!m.memLock());
    CHECK_EQ(m.cell(1), 1u);
}

TEST(bell_on_halt)
{
    Config cfg;
    cfg.bellOnHalt = true;
    Machine m = bfMachine("H", cfg);
    m.step();
    CHECK_EQ(m.bells(), 1u);
}

//======================================================================
// Loops (REQ-ISA-BF-001, REQ-CNT-007)
//======================================================================

TEST(lbeg_skip_stops_on_matching_bracket)
{
    // [ with zero: the scan stops ON ], which is then decoded
    Machine m = bfMachine("[+[+]+]+");
    steps(m, 1);
    CHECK_EQ(m.ip(), 0u);
    steps(m, 1);
    CHECK_EQ(m.ip(), 6u);
    CHECK_EQ(m.insn(), OP_LEND);
    CHECK_EQ(m.loopCount(), 0u);        // self-cleaning
    CHECK_EQ(m.iret(), 2u);
    steps(m, 1);
    CHECK_EQ(m.ip(), 7u);
    CHECK_EQ(m.cellValue(), 1u);
}

TEST(bf_loop_counts_down)
{
    Machine m = bfMachine("+++[->++<]H");
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK_EQ(m.cell(0), 0u);
    CHECK_EQ(m.cell(1), 6u);
    CHECK_EQ(m.loopCount(), 0u);
}

TEST(bracket_test_reads_once)
{
    // AP_TEST before [ reads the cell only if it is not at hand
    Machine m = bfMachine(">[");
    m.setCell(1, 1);
    steps(m, 2);
    CHECK_EQ(m.memReads(), 1u);
    steps(m, 1);                        // enters the body: NOP after '['
    CHECK_EQ(m.memReads(), 1u);
}

TEST(ap_loops_test_ap)
{
    // { skips when AP == 0; } repeats while AP != 0; no memory read
    Machine m = bfMachine("{+}>>>{<}H");
    CHECK(m.runUntilHalt(100) == Status::Halted);
    CHECK_EQ(m.cellValue(), 0u);        // body of the first loop skipped
    CHECK_EQ(m.ap(), 0u);               // second loop ran AP down to 0
    CHECK_EQ(m.memReads(), 0u);
}

TEST(mixed_brackets_scan_together)
{
    // The scan counts [ ] and { } together (IpLine sees only x11x)
    Machine z = bfMachine("[{+}]+H");
    CHECK(z.runUntilHalt(100) == Status::Halted);
    CHECK_EQ(z.cellValue(), 1u);        // [ at zero skipped the whole { + }
    Machine b = bfMachine(">>+{[-]<}H");
    CHECK(b.runUntilHalt(100) == Status::Halted);
    CHECK_EQ(b.ap(), 0u);               // } scanned back over [ ] to {
    CHECK_EQ(b.cell(2), 0u);
    CHECK_EQ(b.cell(1), 0u);
}

TEST(loop_depth_99_ok)
{
    std::string src = std::string(99, '[') + std::string(99, ']') + "H";
    Machine m = bfMachine(src);
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK(!m.loopOverflow());
    CHECK_EQ(m.loopCount(), 0u);
    CHECK_EQ(m.ip(), 199u);
}

TEST(loop_overflow_stops)
{
    // REQ-CNT-007, TC-N2: the 100th level overflows; caught before the step,
    // the scan is aborted and the machine stops (not halted) on the bracket
    Config cfg;
    cfg.bellOnError = true;
    std::string src = std::string(100, '[') + std::string(100, ']') + "H";
    Machine m = bfMachine(src, cfg);
    CHECK(m.step() == Status::Ok);
    CHECK(m.step() == Status::Halted);
    CHECK(m.loopOverflow());
    CHECK(!m.halted());                 // stopped, not in the halt state
    CHECK_EQ(m.loopCount(), 99u);       // the increment is not issued
    CHECK_EQ(m.bells(), 1u);
    CHECK_EQ(m.ip(), 99u);              // stands on the bracket, no halt step
    CHECK_EQ(m.iret(), 1u);
}

TEST(loop_overflow_run_does_not_restart)
{
    // Only a reset clears the overflow: Run and further steps do nothing
    Config cfg;
    cfg.bellOnError = true;
    std::string src = std::string(100, '[') + std::string(100, ']') + "H";
    Machine m = bfMachine(src, cfg);
    m.runUntilHalt();
    CHECK(m.loopOverflow());
    m.run();
    CHECK(m.step() == Status::Halted);
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK_EQ(m.ip(), 99u);
    CHECK_EQ(m.bells(), 1u);
    CHECK_EQ(m.iret(), 1u);
    m.softReset();
    CHECK(!m.loopOverflow());
}

TEST(unmatched_bracket_overflows)
{
    // A lone [ is counted again on every turn through program memory, so
    // the scan ends with the REQ-CNT-007 overflow, not a hang
    Machine m = bfMachine("[+H");
    CHECK(m.step() == Status::Ok);
    CHECK(m.step() == Status::Halted);
    CHECK(m.loopOverflow());
    CHECK_EQ(m.ip(), 0u);               // stopped on its own [ (TC-N2: no halt step)
}

TEST(backward_scan_wraps_through_bootloader)
{
    // An unmatched ] scans back through 0 into the bootloader decade; a
    // { there counts as an opening bracket (x11x)
    Machine m = bfMachine("+]H");
    std::vector<uint8_t> rom(BOOT_SIZE, OP_NOP);
    rom[BOOT_SIZE - 1] = OP_LABEG;      // at 99999
    m.setBootRom(rom);
    steps(m, 3);
    CHECK_EQ(m.ip(), IP_SIZE - 1);
    CHECK_EQ(m.insn(), OP_LABEG);
}

//======================================================================
// LD (SOT/EOT) program loading (REQ-LOAD-*, REQ-VER-018)
//======================================================================

TEST(ld_loads_after_itself)
{
    // REQ-LOAD-001/002/006/007/008: a program executes LD and loads at
    // the address after it; LD in the stream is EOT, not stored,
    // SoftRstOnEOT gives Soft Reset
    Machine m = bfMachine("NS");
    m.pushInsns({OP_INC, OP_AINC, OP_COUT, OP_LD});
    steps(m, 2);
    CHECK(m.loading());
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK(!m.loading());
    CHECK_EQ(m.code(2), OP_INC);
    CHECK_EQ(m.code(3), OP_AINC);       // '>' loads: EOT is a separate code
    CHECK_EQ(m.code(4), OP_COUT);
    CHECK_EQ(m.code(5), OP_NOP);        // EOT not written
    CHECK_EQ(m.ip(), 0u);
    CHECK_EQ(m.iret(), 6u);             // 2 + 3 opcodes + EOT
}

TEST(ld_at_top_wraps_to_zero)
{
    // REQ-LOAD-005: LD at the top address, the load wraps to 0
    Machine m;
    std::vector<uint8_t> rom(BOOT_SIZE, OP_NOP);
    rom[BOOT_SIZE - 1] = OP_LD;         // LD at 99999
    m.setBootRom(rom);
    m.hardReset();
    m.run();
    m.pushInsns({OP_INC, OP_DEC, OP_AINC, OP_LD});
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK_EQ(m.code(0), OP_INC);
    CHECK_EQ(m.code(1), OP_DEC);
    CHECK_EQ(m.code(2), OP_AINC);
    CHECK_EQ(m.ip(), 0u);
}

TEST(load_into_bootloader_is_ignored)
{
    // REQ-MEM-IP-007: the ROM decade is write-protected
    std::vector<uint8_t> rom = assemble("+");
    Machine n;
    n.setBootRom(rom);
    n.hardReset();                      // IP = 99990
    n.startLoading();                   // loads at the current IP
    n.pushInsns({OP_DEC, OP_LD});
    CHECK(n.runUntilHalt() == Status::Halted);
    CHECK_EQ(n.code(BOOT_BASE), OP_INC);     // unchanged
    CHECK_EQ(n.code(BOOT_BASE + 1), OP_NOP);
}

TEST(panel_loading_starts_at_current_ip)
{
    Machine m;
    m.softReset();
    m.startLoading();
    m.pushInsns({OP_INC, OP_INC});
    steps(m, 2);
    CHECK_EQ(m.code(0), OP_INC);
    CHECK_EQ(m.code(1), OP_INC);
    CHECK_EQ(m.ip(), 1u);
    CHECK(m.step() == Status::WaitInput);
    CHECK_EQ(m.ip(), 2u);               // IP moved before waiting (S_INSN_IN)
    m.pushInsn(OP_AINC);
    CHECK(m.step() == Status::Ok);
    CHECK(m.loading());
    CHECK_EQ(m.code(2), OP_AINC);
}

TEST(eot_without_soft_reset_halts)
{
    // REQ-LOAD-009
    Config cfg;
    cfg.softRstOnEot = false;
    Machine m = bfMachine("NS", cfg);
    m.pushInsns({OP_INC, OP_LD});
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK(!m.loading());
    CHECK_EQ(m.ip(), 4u);               // EOT at 3, halt step +1
}

//======================================================================
// Bootloader and full programs (REQ-BOOT-*, REQ-VER-019)
//======================================================================

TEST(bootloader_clears_data_ram_and_loads)
{
    // REQ-BOOT-003/005: { } walk AP through all of memory, then LD at
    // 99999 loads from address 0
    Config cfg;
    cfg.runOnHardRst = true;
    cfg.runOnSoftRst = true;
    Machine m(cfg);
    m.setBootRom(assemble(BOOTLOADER));
    for (uint32_t a = 0; a <= AP_TOP_BF; a += 997)
        m.setCell(a, static_cast<uint8_t>(a | 1));
    m.setCell(AP_TOP_BF, 1);
    m.hardReset();
    m.pushInsns(assemble("++.HE"));
    m.pushInput(0);
    CHECK(m.runUntilHalt() == Status::Halted);
    bool allZero = true;
    for (uint32_t a = 1; a <= AP_TOP_BF; ++a)
        allZero = allZero && (m.cell(a) == 0);
    CHECK(allZero);
    CHECK_EQ(m.code(0), OP_INC);
    CHECK_EQ(m.code(3), OP_HALT);
    CHECK(m.output() == std::string(1, '\x02'));   // the loaded program ran
    CHECK(!m.loopOverflow());
}

TEST(helloworld)
{
    std::string src = readFile(BFUTILS_COMMON "/helloworld.bfk");
    CHECK(!src.empty());
    std::vector<uint8_t> code = assemble(src);
    code.push_back(OP_HALT);
    Machine m;
    m.loadCode(code);
    m.softReset();
    m.run();
    CHECK(m.runUntilHalt(1000000) == Status::Halted);
    CHECK(m.output().find("Hello World!") != std::string::npos);
    CHECK_EQ(m.ip(), code.size());
}

TEST(pi_memory_accesses_match_rtl)
{
    // ApLine.sv header: 99 077 data memory accesses on pi.bfk with lazy read
    std::string src = readFile(BFUTILS_COMMON "/pi.bfk");
    std::vector<uint8_t> code = assemble(src);
    code.push_back(OP_HALT);
    Machine m;
    m.loadCode(code);
    m.softReset();
    m.run();
    CHECK(m.runUntilHalt(10000000) == Status::Halted);
    CHECK(m.output().find("3.141") == 0);
    CHECK_EQ(m.memReads() + m.memWrites(), 99077u);
}

//======================================================================

int main(int argc, char** argv)
{
    const char* only = (argc > 1) ? argv[1] : nullptr;
    int run = 0, failedTests = 0;
    for (const TestCase& t : registry()) {
        if (only && std::string(t.name) != only)
            continue;
        int before = g_failures;
        t.fn();
        ++run;
        bool ok = (g_failures == before);
        if (!ok)
            ++failedTests;
        std::printf("[%s] %s\n", ok ? " OK " : "FAIL", t.name);
    }
    std::printf("\n%d tests, %d checks, %d failed tests\n", run, g_checks, failedTests);
    return (failedTests || run == 0) ? 1 : 0;
}
