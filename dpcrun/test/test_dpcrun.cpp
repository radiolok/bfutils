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

const char* const BOOTLOADER = "DA0B<D{0B<D}BH";   // rtl/programs/bootloader.bfk

} // namespace

//======================================================================
// Loader (REQ-IPV2-005, rtl/run/generate_rom.py table)
//======================================================================

TEST(loader_symbol_table)
{
    std::vector<uint8_t> c = assemble("NH+-><[].,0MGPDB");
    CHECK_EQ(c.size(), 16u);
    for (size_t i = 0; i < c.size(); ++i)
        CHECK_EQ(c[i], i);

    std::vector<uint8_t> d = assemble("\aES{}LIArr");
    std::vector<uint8_t> want = {0x2, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xB, 0xD, 0xD};
    CHECK(d == want);
    CHECK_EQ(symbolToOpcode('R'), DBG_HRST);
}

TEST(loader_skips_comments_keeps_nop)
{
    // 'N' is opcode 0 and must not be dropped as "false"
    std::vector<uint8_t> c = assemble("a+ b\nN-\t# x");
    std::vector<uint8_t> want = {BF_INC, OP_NOP, BF_DEC};
    CHECK(c == want);
}

TEST(opcode_to_symbol_roundtrip)
{
    for (int mode = 0; mode < 2; ++mode)
        for (uint8_t op = 0; op < 16; ++op) {
            if (mode == ISA_DEBUG && op == DBG_RSVD)
                continue;   // reserved code has no own symbol
            CHECK_EQ(symbolToOpcode(opcodeToSymbol(op, mode)), op);
        }
    CHECK(std::string(mnemonic(0xC, ISA_DEBUG)) == "HRST");
    CHECK(std::string(mnemonic(0xC, ISA_BF)) == "LOAD");
}

//======================================================================
// Power-on and resets (REQ-RST-*)
//======================================================================

TEST(power_on_state)
{
    Machine m;
    CHECK(m.halted());
    CHECK_EQ(m.insnMode(), ISA_DEBUG);
    CHECK_EQ(m.ip(), 0u);
    CHECK_EQ(m.iret(), 0u);
    CHECK(m.step() == Status::Halted);
}

TEST(hard_reset_state)
{
    // REQ-RST-001/003, REQ-BOOT-002
    Machine m = bfMachine("+++>++");
    steps(m, 5);
    m.hardReset();
    CHECK_EQ(m.ip(), BOOT_BASE);
    CHECK_EQ(m.ap(), 0u);
    CHECK_EQ(m.dataCounter(), 0u);
    CHECK_EQ(m.loopCount(), 0u);
    CHECK(!m.memLock());
    CHECK_EQ(m.insnMode(), ISA_DEBUG);
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
    CHECK_EQ(m.insnMode(), ISA_BF);
    CHECK(!m.halted());         // RunOnSoftRst = 1
}

TEST(reset_drops_dirty_counter)
{
    // The data counter is not written back on reset; memory is retained
    Machine m = bfMachine("+>+++");
    steps(m, 5);
    CHECK(m.memLock());
    CHECK(m.dirty());
    m.softReset();
    CHECK_EQ(m.cell(0), 1u);    // flushed by '>'
    CHECK_EQ(m.cell(1), 0u);    // 3 was only in the counter
}

TEST(first_fetch_after_reset_does_not_increment)
{
    Machine m = bfMachine("+H");
    steps(m, 1);
    CHECK_EQ(m.ip(), 0u);
    CHECK_EQ(m.insn(), BF_INC);
    CHECK_EQ(m.cellValue(), 1u);
}

//======================================================================
// Common instructions (REQ-ISA-002/003/004)
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

TEST(isa_switch)
{
    Machine m = bfMachine("DB");
    CHECK_EQ(m.insnMode(), ISA_BF);
    steps(m, 1);
    CHECK_EQ(m.insnMode(), ISA_DEBUG);
    steps(m, 1);
    CHECK_EQ(m.insnMode(), ISA_BF);
}

TEST(full_decode_table_nop_codes)
{
    // Codes that must not change anything but IP/IRET: NOP in both ISAs,
    // reserved 0x3 and EOT outside loading in Debug (REQ-CTLV2-006)
    const uint8_t dbgNops[] = {OP_NOP, DBG_RSVD, DBG_EOT};
    for (uint8_t op : dbgNops) {
        Machine m;
        m.loadCode({OP_ISA0, op});
        m.softReset();
        m.run();
        steps(m, 2);
        CHECK_EQ(m.ip(), 1u);
        CHECK_EQ(m.ap(), 0u);
        CHECK(!m.memLock());
        CHECK(!m.loading());
        CHECK(!m.halted());
        CHECK_EQ(m.bells(), 0u);
        CHECK_EQ(m.insnMode(), ISA_DEBUG);
    }
}

TEST(all_32_codes_decode)
{
    // Every {mode, opcode} pair retires or halts; nothing throws or hangs
    for (int mode = 0; mode < 2; ++mode)
        for (uint8_t op = 0; op < 16; ++op) {
            Machine m;
            m.loadCode({mode == ISA_BF ? OP_ISA1 : OP_ISA0, op, OP_HALT});
            m.softReset();
            m.run();
            m.pushInput('x');
            Status s = m.runUntilHalt(10);
            CHECK(s == Status::Halted || s == Status::WaitInput || s == Status::Ok);
            CHECK(m.iret() >= 2u);
        }
}

//======================================================================
// Brainfuck ISA: data and address (REQ-ISA-BF-*, REQ-ML-*)
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
    CHECK(m.dirty());
    CHECK_EQ(m.dataCounter(), 12u);
    CHECK_EQ(m.cell(0), 10u);   // not written back yet
    CHECK_EQ(m.cellValue(), 12u);
    CHECK_EQ(m.memReads(), 1u);
    CHECK_EQ(m.memWrites(), 0u);
}

TEST(ap_move_flushes_dirty_counter)
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

TEST(tx_register_is_stale_after_move)
{
    // tx_data_bcd shows the memory register, which still holds the last
    // touched cell until the new one is read
    Machine m = bfMachine("+++>");
    steps(m, 4);
    CHECK_EQ(m.txData(), 3u);
    CHECK_EQ(m.cellValue(), 0u);
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

TEST(load_overwrites_counter_keeps_lock)
{
    // REQ-ISA-BF-003, REQ-ML-004
    Machine m = bfMachine("+++G");
    m.setCell(0, 7);
    steps(m, 3);
    CHECK_EQ(m.dataCounter(), 10u);
    steps(m, 1);
    CHECK_EQ(m.dataCounter(), 7u);
    CHECK(m.memLock());
    CHECK_EQ(m.cellValue(), 7u);
}

TEST(store_copies_counter)
{
    // REQ-ISA-BF-004: LOAD at A, move, STORE at B copies A to B
    Machine m = bfMachine("G>>P");
    m.setCell(0, 99);
    steps(m, 4);
    CHECK_EQ(m.cell(2), 99u);
    CHECK_EQ(m.cell(0), 99u);
    CHECK(!m.memLock());
}

TEST(store_keeps_lock_across_move)
{
    // RTL: STORE clears dirty but not MemLock, and > flushes (and unlocks)
    // only a dirty counter. So the lock survives and the counter stands
    // for the new cell. TRS REQ-ML-005 expects the lock to be released.
    Machine m = bfMachine("+++++P>");
    steps(m, 7);
    CHECK_EQ(m.cell(0), 5u);
    CHECK(m.memLock());
    CHECK(!m.dirty());
    CHECK_EQ(m.ap(), 1u);
    CHECK_EQ(m.cellValue(), 5u);   // cell 1 in memory is still 0
    CHECK_EQ(m.cell(1), 0u);
}

TEST(clrml_flushes_and_unlocks)
{
    // REQ-ML-008 / REQ-ISA-BF-006
    Machine m = bfMachine("++M");
    steps(m, 3);
    CHECK(!m.memLock());
    CHECK_EQ(m.cell(0), 2u);
    CHECK_EQ(m.memWrites(), 1u);

    Machine m2 = bfMachine("+PM");      // lock without dirty: no write
    steps(m2, 3);
    CHECK(!m2.memLock());
    CHECK_EQ(m2.memWrites(), 1u);       // only the STORE
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
    CHECK(m.dirty());
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

//======================================================================
// Loops (REQ-ISA-BF-001, REQ-ISA-DEBUG-001..003, REQ-CNT-007)
//======================================================================

TEST(lbeg_skip_stops_on_matching_bracket)
{
    // [ with zero: the scan stops ON ], which is then decoded
    Machine m = bfMachine("[+[+]+]+");
    steps(m, 1);
    CHECK_EQ(m.ip(), 0u);
    steps(m, 1);
    CHECK_EQ(m.ip(), 6u);
    CHECK_EQ(m.insn(), BF_LEND);
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

TEST(debug_loops_test_ap)
{
    // { skips when AP == 0; } repeats while AP != 0
    Machine m;
    m.loadCode(assemble("D{\a}B>D{B<D}H"));
    m.softReset();
    m.run();
    CHECK(m.runUntilHalt(100) == Status::Halted);
    CHECK_EQ(m.bells(), 0u);            // body of the first loop skipped
    CHECK_EQ(m.ap(), 0u);               // second loop ran AP down to 0
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

TEST(loop_overflow_halts)
{
    // REQ-CNT-007: the 100th level overflows; caught before the step, the
    // scan is aborted and the machine halts
    Config cfg;
    cfg.bellOnError = true;
    std::string src = std::string(100, '[') + std::string(100, ']') + "H";
    Machine m = bfMachine(src, cfg);
    CHECK(m.step() == Status::Ok);
    CHECK(m.step() == Status::Halted);
    CHECK(m.loopOverflow());
    CHECK_EQ(m.loopCount(), 99u);       // the increment is not issued
    CHECK_EQ(m.bells(), 1u);
    CHECK_EQ(m.ip(), 100u);             // stopped at 99, halt step +1
    CHECK_EQ(m.iret(), 1u);
}

TEST(loop_overflow_sticks_at_top)
{
    // After an overflow the counter stays at 99: the next scan overflows at
    // once on its own bracket, without moving IP
    Config cfg;
    cfg.bellOnError = true;
    std::string src = std::string(100, '[') + std::string(100, ']') + "H";
    Machine m = bfMachine(src, cfg);
    m.runUntilHalt();
    CHECK(m.loopOverflow());
    CHECK_EQ(m.loopCount(), 99u);
    CHECK_EQ(m.ip(), 100u);
    m.loadCode(assemble("[]"), m.ip());  // a fresh, balanced scan with loop = 99
    m.run();
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK_EQ(m.bells(), 2u);             // the second overflow fired
    CHECK_EQ(m.loopCount(), 99u);
    CHECK_EQ(m.ip(), 101u);              // stood on its own [, halt step +1
}

TEST(clrl_clears_loop_and_error)
{
    // REQ-ISA-DEBUG-006
    std::string src = std::string(100, '[') + std::string(100, ']');
    Machine m = bfMachine(src);
    m.runUntilHalt();
    CHECK(m.loopOverflow());
    m.loadCode(assemble("DL"), m.ip());
    m.run();
    steps(m, 2);
    CHECK(!m.loopOverflow());
    CHECK_EQ(m.loopCount(), 0u);
}

TEST(unmatched_bracket_overflows)
{
    // A lone [ is counted again on every turn through program memory, so
    // the scan ends with the REQ-CNT-007 overflow, not a hang
    Machine m = bfMachine("[+H");
    CHECK(m.step() == Status::Ok);
    CHECK(m.step() == Status::Halted);
    CHECK(m.loopOverflow());
    CHECK_EQ(m.ip(), 1u);               // stopped on its own [, halt step +1
}

TEST(backward_scan_wraps_through_bootloader)
{
    // An unmatched ] scans back through 0 into the bootloader bank; a
    // { there counts as an opening bracket (same opcode 0x6)
    Machine m = bfMachine("+]H");
    m.setBootRom(assemble("{"));
    steps(m, 3);
    CHECK_EQ(m.ip(), BOOT_BASE);
    CHECK_EQ(m.insn(), DBG_LABEG);
}

//======================================================================
// Debug ISA (REQ-ISA-DEBUG-*, REQ-VER-015)
//======================================================================

TEST(bell)
{
    Machine m;
    int hooks = 0;
    m.onBell = [&hooks]() { ++hooks; };
    m.loadCode(assemble("D\a\aH"));
    m.softReset();
    m.run();
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK_EQ(m.bells(), 2u);
    CHECK_EQ(hooks, 2);
}

TEST(bell_on_halt)
{
    Config cfg;
    cfg.bellOnHalt = true;
    Machine m = bfMachine("H", cfg);
    m.step();
    CHECK_EQ(m.bells(), 1u);
}

TEST(clri_restarts_at_zero)
{
    // REQ-ISA-DEBUG-004. The next instruction executed is at address 0,
    // read without an increment.
    Machine m = bfMachine("+DNI");
    steps(m, 4);
    CHECK_EQ(m.ip(), 0u);
    steps(m, 1);
    CHECK_EQ(m.ip(), 0u);
    CHECK_EQ(m.insn(), BF_INC);
}

TEST(clri_keeps_memlock)
{
    // RTL: CLRI only resets the IP counter. TRS REQ-ML-006 says it also
    // clears MemLock.
    Machine m = bfMachine("+DI");
    steps(m, 3);
    CHECK(m.memLock());
    CHECK(m.dirty());
}

TEST(clra)
{
    // REQ-ISA-DEBUG-005: dirty counter is flushed to the old cell first
    Machine m = bfMachine(">>++DA");
    steps(m, 6);
    CHECK_EQ(m.ap(), 0u);
    CHECK_EQ(m.cell(2), 2u);
    CHECK(!m.memLock());
}

TEST(clra_after_store_keeps_lock)
{
    // RTL: same rule as > : only a dirty counter unlocks
    Machine m = bfMachine(">+PDA");
    steps(m, 5);
    CHECK_EQ(m.ap(), 0u);
    CHECK(m.memLock());
}

TEST(clrd_debug)
{
    Machine m = bfMachine("D0");
    m.setCell(0, 9);
    steps(m, 2);
    CHECK_EQ(m.cellValue(), 0u);
    CHECK(m.memLock());
}

TEST(hrst_opcode)
{
    // REQ-ISA-DEBUG-008, REQ-RST-006: with RunOnHardRst the bootloader runs
    Config cfg;
    cfg.runOnHardRst = true;
    Machine m = bfMachine("+DR", cfg);
    m.setBootRom(assemble("\aH"));
    steps(m, 3);
    CHECK_EQ(m.ip(), BOOT_BASE);
    CHECK_EQ(m.insnMode(), ISA_DEBUG);
    CHECK(!m.halted());
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK_EQ(m.bells(), 1u);
    CHECK_EQ(m.ip(), BOOT_BASE + 2);
}

TEST(srst_opcode)
{
    // REQ-ISA-DEBUG-009, REQ-RST-007: without RunOnSoftRst the machine waits
    Machine m = bfMachine(">+Dr");
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK_EQ(m.ip(), 0u);
    CHECK_EQ(m.ap(), 0u);
    CHECK_EQ(m.insnMode(), ISA_BF);
    CHECK_EQ(m.iret(), 4u);
}

//======================================================================
// SOT/EOT program loading (REQ-LOAD-*, REQ-VER-018)
//======================================================================

TEST(sot_loads_after_itself)
{
    // REQ-LOAD-001/002/006/007/008: a program executes SOT and loads at
    // the address after it, EOT is not
    // stored, SoftRstOnEOT gives Soft Reset
    Machine m = bfMachine("DS");
    m.pushInsns({BF_INC, BF_INC, BF_COUT, DBG_EOT});
    steps(m, 2);
    CHECK(m.loading());
    CHECK_EQ(m.insnMode(), ISA_DEBUG);  // OPEN-003: SOT keeps the ISA
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK(!m.loading());
    CHECK_EQ(m.code(2), BF_INC);
    CHECK_EQ(m.code(3), BF_INC);
    CHECK_EQ(m.code(4), BF_COUT);
    CHECK_EQ(m.code(5), OP_NOP);        // EOT not written
    CHECK_EQ(m.ip(), 0u);
    CHECK_EQ(m.insnMode(), ISA_BF);
    CHECK_EQ(m.iret(), 6u);             // 2 + 3 opcodes + EOT
}

TEST(sot_at_top_wraps_to_zero)
{
    // REQ-LOAD-005: SOT at the top address, the load wraps to 0.
    // In Debug ISA 0x4 is EOT, so a load in Debug ISA can't carry '>'.
    Machine m;
    std::vector<uint8_t> rom(BOOT_SIZE, OP_NOP);
    rom[99] = DBG_SOT;                  // SOT at 99999
    m.setBootRom(rom);
    m.hardReset();
    m.run();
    m.pushInsns({BF_INC, BF_DEC, DBG_EOT});
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK_EQ(m.code(0), BF_INC);
    CHECK_EQ(m.code(1), BF_DEC);
    CHECK_EQ(m.ip(), 0u);
}

TEST(load_into_bootloader_is_ignored)
{
    // REQ-MEM-IP-007: the ROM bank is write-protected
    std::vector<uint8_t> rom = assemble("\a");
    Machine n;
    n.setBootRom(rom);
    n.hardReset();                      // IP = 99900
    n.startLoading();                   // loads at the current IP
    n.pushInsns({BF_INC, DBG_EOT});
    CHECK(n.runUntilHalt() == Status::Halted);
    CHECK_EQ(n.code(BOOT_BASE), DBG_BELL);   // unchanged
    CHECK_EQ(n.code(BOOT_BASE + 1), OP_NOP);
}

TEST(panel_loading_starts_at_current_ip)
{
    Machine m;
    m.softReset();
    m.startLoading();
    m.pushInsns({BF_INC, BF_INC});
    steps(m, 2);
    CHECK_EQ(m.code(0), BF_INC);
    CHECK_EQ(m.code(1), BF_INC);
    CHECK_EQ(m.ip(), 1u);
    CHECK(m.step() == Status::WaitInput);
    CHECK_EQ(m.ip(), 2u);               // IP moved before waiting (S_INSN_IN)
    m.pushInsn(DBG_EOT);
    // mode after softReset is BF: 0x4 is '>' there, not EOT
    CHECK(m.step() == Status::Ok);
    CHECK(m.loading());
    CHECK_EQ(m.code(2), BF_AINC);
}

TEST(eot_without_soft_reset_halts)
{
    // REQ-LOAD-009
    Config cfg;
    cfg.softRstOnEot = false;
    Machine m = bfMachine("DS", cfg);
    m.pushInsns({BF_INC, DBG_EOT});
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK(!m.loading());
    CHECK_EQ(m.insnMode(), ISA_DEBUG);
    CHECK_EQ(m.ip(), 4u);               // EOT at 3, halt step +1
}

TEST(isa1_during_load_hides_eot)
{
    // RTL: ISA0/ISA1 act while loading and are stored; after ISA1 the
    // code 0x4 is '>' and is stored instead of ending the load. ISA0
    // brings EOT back.
    Machine m = bfMachine("DS");
    m.pushInsns({OP_ISA1, BF_AINC, OP_ISA0, DBG_EOT});
    CHECK(m.runUntilHalt() == Status::Halted);
    CHECK_EQ(m.code(2), OP_ISA1);
    CHECK_EQ(m.code(3), BF_AINC);
    CHECK_EQ(m.code(4), OP_ISA0);
    CHECK_EQ(m.code(5), OP_NOP);
    CHECK_EQ(m.ip(), 0u);               // soft reset after EOT
}

//======================================================================
// Bootloader and full programs (REQ-BOOT-*, REQ-VER-019)
//======================================================================

TEST(bootloader_clears_data_ram)
{
    // REQ-BOOT-003/005: Debug loops walk AP through all of memory
    Config cfg;
    cfg.runOnHardRst = true;
    Machine m(cfg);
    m.setBootRom(assemble(BOOTLOADER));
    for (uint32_t a = 0; a <= AP_TOP_BF; a += 997)
        m.setCell(a, static_cast<uint8_t>(a | 1));
    m.setCell(AP_TOP_BF, 1);
    m.hardReset();
    CHECK(m.runUntilHalt() == Status::Halted);
    bool allZero = true;
    for (uint32_t a = 0; a <= AP_TOP_BF; ++a)
        allZero = allZero && (m.cell(a) == 0);
    CHECK(allZero);
    CHECK_EQ(m.ap(), 0u);
    CHECK(!m.memLock());
    CHECK_EQ(m.insnMode(), ISA_BF);
    CHECK_EQ(m.ip(), BOOT_BASE + 14);   // H at 99913, halt step +1
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
