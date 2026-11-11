/* Copyright (c) 2016-2026, Artem Kashkanov
All rights reserved. See dpcrun.cpp for the full license text. */

// DekatronPC golden model (REQ-GM-001/003/004).
//
// An instruction-level model of the DekatronPC machine: the 4-bit ISA16
// (doc/isa16.md of the dekatronpc repository), MemLock with the lazy data
// read, LD (SOT/EOT) program loading, hard and soft resets, the loop-nesting
// counter with its overflow error and the write-protected bootloader ROM.
//
// The model mirrors the RTL (rtl/DekatronPC/MachineCtrl.sv, IpLine.sv,
// ApLine.sv) as it is, including the places where the RTL and the TRS still
// disagree. Those places are marked "RTL:" in dpcrun.cpp and listed in
// doc/dpcrun_golden_model.md of the dekatronpc repository.
//
// Program memory holds 4-bit opcodes, never ASCII (REQ-IPV2-005). Converting
// source text to opcodes is the loader's job: see assemble().

#ifndef DPCRUN_H
#define DPCRUN_H

#include <stdint.h>
#include <deque>
#include <functional>
#include <string>
#include <vector>

namespace dpc {

//----------------------------------------------------------------------
// Architecture sizes (rtl/parameters.sv)
//----------------------------------------------------------------------
const uint32_t IP_SIZE    = 10000;    // 4 dekatrons: 0..9999
const uint32_t BOOT_BASE  = 9990;     // bootloader ROM, one decade
const uint32_t BOOT_SIZE  = 10;
const uint32_t LOOP_SIZE  = 100;      // 2 dekatrons: 0..99 (LOOP_DEKATRON_NUM)
const uint32_t DATA_TOP   = 255;      // 3 dekatrons, TOP_LIMIT_MODE
const uint32_t AP_SIZE    = 1000;     // 3 dekatrons: 0..999, wraps 0 <-> 999
const uint32_t AP_TOP     = AP_SIZE - 1;

//----------------------------------------------------------------------
// Opcodes: one 4-bit instruction set (ISA16), no modes. 0x0..0xA are the
// Brainfuck codes. Brackets are x11x: bit 0 open/close, bit 3 what is
// tested (0 the cell, 1 AP); a scan counts both kinds together.
//----------------------------------------------------------------------
enum Opcode : uint8_t {
    OP_NOP   = 0x0,
    OP_HALT  = 0x1,
    OP_INC   = 0x2,    // +
    OP_DEC   = 0x3,    // -
    OP_AINC  = 0x4,    // >
    OP_ADEC  = 0x5,    // <
    OP_LBEG  = 0x6,    // [  skip if data == 0
    OP_LEND  = 0x7,    // ]  repeat if data != 0
    OP_COUT  = 0x8,    // .
    OP_CIN   = 0x9,    // ,
    OP_CLRD  = 0xA,    // [-] sets MemLock
    OP_LD    = 0xB,    // SOT; EOT in the load stream (not stored)
    OP_CLRA  = 0xC,
    OP_MEMLOCK = 0xD,  // sets the MemLock flag without a load: cell := data counter
    OP_LABEG = 0xE,    // {  skip if AP == 0
    OP_LAEND = 0xF     // }  repeat if AP != 0
};

inline bool isLoopOpen(uint8_t op)  { return (op & 0x7) == 0x6; }
inline bool isLoopClose(uint8_t op) { return (op & 0x7) == 0x7; }

//----------------------------------------------------------------------
// Loader: text <-> opcodes. Same symbol table as rtl/run/generate_rom.py.
//----------------------------------------------------------------------

// Opcode for a program symbol, or -1 when the character is not one
// (comments, whitespace).
int symbolToOpcode(char c);

// Converts source text to opcodes, skipping everything that isn't a symbol.
std::vector<uint8_t> assemble(const std::string& source);

// Printable symbol and mnemonic of an opcode.
char        opcodeToSymbol(uint8_t op);
const char* mnemonic(uint8_t op);

//----------------------------------------------------------------------
// Panel switches and parameters
//----------------------------------------------------------------------
struct Config {
    uint32_t apTop        = AP_TOP;      // a smaller ring is a model option only
    bool     runOnHardRst = false;       // OPEN-011: Emulator hardcodes these
    bool     runOnSoftRst = false;
    bool     softRstOnEot = true;
    bool     echoMode     = true;
    bool     bellOnCin    = false;
    bool     bellOnHalt   = false;
    bool     bellOnError  = false;
};

//----------------------------------------------------------------------
// Result of one step
//----------------------------------------------------------------------
enum class Status {
    Ok,          // instruction retired, machine keeps running
    Halted,      // machine is (or just became) halted, or stopped by a loop overflow
    WaitInput    // CIN or load mode has no input; retry after input
};

const char* statusName(Status s);

//----------------------------------------------------------------------
// The machine
//----------------------------------------------------------------------
class Machine {
public:
    explicit Machine(const Config& cfg = Config());

    // Power-on: counters at zero, memories cleared, halted.
    // rst_n in the RTL resets logic only; the model has no separate
    // "counter" state to keep, so this is the full initial state.
    void powerOn();

    //--- Panel -----------------------------------------------------------
    void hardReset();          // HardRstKey
    void softReset();          // SoftRstKey or SoftRstOnEOT
    void run();                // Run key: leave halt
    void startLoading();       // InsnLoadingStart: load from the current IP

    //--- Execution -------------------------------------------------------
    // Executes one instruction (one pass through S_DECODE in MachineCtrl).
    Status step();

    // Runs until the machine halts, waits for input or maxSteps
    // instructions retire (then returns Ok).
    Status runUntilHalt(uint64_t maxSteps = UINT64_MAX);

    //--- Memory initialisation (no machine cycles) -----------------------
    // Writes opcodes into program RAM from `base`, like the IPMEMFILE hex.
    // Addresses in the bootloader range are dropped: it is a ROM.
    void loadCode(const std::vector<uint8_t>& code, uint32_t base = 0);
    void setBootRom(const std::vector<uint8_t>& rom);
    void setCell(uint32_t addr, uint8_t value);

    //--- Terminal and loader input ---------------------------------------
    void pushInput(uint8_t c)      { m_rx.push_back(c); }
    void pushInsn(uint8_t op)      { m_insnIn.push_back(op & 0xF); }
    void pushInsns(const std::vector<uint8_t>& ops);

    // Optional hooks. Without onCin the model takes input from the
    // pushInput() queue; without onCout output only goes to output().
    // onCin returns -1 when there is no character yet.
    std::function<int()>        onCin;
    std::function<void(uint8_t)> onCout;
    std::function<void()>        onBell;

    const std::string& output() const { return m_tx; }
    void clearOutput()                { m_tx.clear(); }

    //--- Observable state ------------------------------------------------
    uint32_t ip() const          { return m_ip; }
    uint32_t ap() const          { return m_ap; }
    uint32_t loopCount() const   { return m_loop; }
    uint8_t  dataCounter() const { return m_data; }
    // What DekatronPC drives on tx_data_bcd: always the data counter.
    // COUT loads the cell into it first when MemLock is off (OPEN-017);
    // between instructions it may hold a value of another cell.
    uint8_t  txData() const      { return m_data; }
    // Architectural value of the current cell.
    uint8_t  cellValue() const   { return m_lock ? m_data : m_dataMem[m_ap]; }
    bool     memLock() const     { return m_lock; }
    bool     loading() const     { return m_loading; }
    bool     halted() const      { return m_halted; }
    bool     loopOverflow() const { return m_overflow; }
    uint8_t  insn() const        { return m_insn; }   // last fetched opcode
    uint64_t iret() const        { return m_iret; }
    uint64_t bells() const       { return m_bells; }
    uint64_t memReads() const    { return m_memReads; }
    uint64_t memWrites() const   { return m_memWrites; }

    uint8_t  code(uint32_t addr) const;
    uint8_t  cell(uint32_t addr) const  { return m_dataMem.at(addr); }

    const Config& config() const { return m_cfg; }

private:
    enum ResetType { RST_NONE, RST_HARD, RST_SOFT };
    // Where step() resumes: a fresh fetch, or a wait that the RTL makes
    // after IP has already moved (S_INSN_IN) or after decode (S_CIN_WAIT).
    enum Phase { PH_FETCH, PH_INSN, PH_CIN };

    void     resetCounters(ResetType type);
    void     enterHalt();
    void     bell();
    void     cout(uint8_t c);
    bool     loopValZero() const;
    Status   fetch();
    Status   scan(bool backward);
    bool     takeInsn();
    void     decode();
    void     decodeLoading();
    Status   finishCin();

    // ApLine operations
    void     memRead();
    void     flush();
    void     apMove(bool zero, bool dec);
    void     dataStep(bool dec);

    uint8_t  readCode(uint32_t addr) const;
    void     writeCode(uint32_t addr, uint8_t op);

    Config   m_cfg;

    // Memories
    std::vector<uint8_t> m_codeRam;
    std::vector<uint8_t> m_bootRom;
    std::vector<uint8_t> m_dataMem;
    uint8_t  m_memReg;     // data RAM output register (last touched cell)

    // Counters
    uint32_t m_ip;
    uint32_t m_ap;
    uint32_t m_loop;
    uint8_t  m_data;

    // ApLine
    bool     m_lock;
    bool     m_memHere;

    // IpLine
    bool     m_ipCounted;  // IP already points at the retired instruction
    uint8_t  m_insn;
    bool     m_overflow;

    // MachineCtrl
    bool     m_loading;
    bool     m_halted;
    Phase    m_phase;
    ResetType m_rstType;

    // Statistics
    uint64_t m_iret;
    uint64_t m_bells;
    uint64_t m_memReads;
    uint64_t m_memWrites;

    // Terminal
    std::deque<uint8_t> m_rx;
    std::deque<uint8_t> m_insnIn;
    std::string m_tx;
};

} // namespace dpc

#endif
