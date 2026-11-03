/* Copyright (c) 2016-2026, Artem Kashkanov
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#include "dpcrun.h"

#include <cstring>
#include <stdexcept>

namespace dpc {

//======================================================================
// Loader
//======================================================================

// Same table as rtl/run/generate_rom.py
int symbolToOpcode(char c)
{
    switch (c) {
    case 'N':  return OP_NOP;
    case 'H':  return OP_HALT;
    case '+':  return OP_INC;
    case '-':  return OP_DEC;
    case '>':  return OP_AINC;
    case '<':  return OP_ADEC;
    case '[':  return OP_LBEG;
    case ']':  return OP_LEND;
    case '.':  return OP_COUT;
    case ',':  return OP_CIN;
    case '0':  return OP_CLRD;
    case 'S':  return OP_LD;      // start of loading
    case 'E':  return OP_LD;      // end of loading in the load stream
    case 'A':  return OP_CLRA;
    case 'P':  return OP_MEMLOCK;
    case '{':  return OP_LABEG;
    case '}':  return OP_LAEND;
    default:   return -1;
    }
}

std::vector<uint8_t> assemble(const std::string& source)
{
    std::vector<uint8_t> code;
    code.reserve(source.size());
    for (char c : source) {
        int op = symbolToOpcode(c);
        if (op >= 0)
            code.push_back(static_cast<uint8_t>(op));
    }
    return code;
}

char opcodeToSymbol(uint8_t op)
{
    static const char sym[] = "NH+-><[].,0SAP{}";
    return sym[op & 0xF];
}

const char* mnemonic(uint8_t op)
{
    static const char* const name[16] = {
        "NOP", "HALT", "INC", "DEC", "AINC", "ADEC", "LBEG", "LEND",
        "COUT", "CIN", "CLRD", "LD", "CLRA", "MEMLOCK", "LABEG", "LAEND"
    };
    return name[op & 0xF];
}

const char* statusName(Status s)
{
    switch (s) {
    case Status::Ok:        return "Ok";
    case Status::Halted:    return "Halted";
    case Status::WaitInput: return "WaitInput";
    }
    return "?";
}

//======================================================================
// Machine
//======================================================================

static inline uint32_t wrapInc(uint32_t v, uint32_t size) { return (v + 1) % size; }
static inline uint32_t wrapDec(uint32_t v, uint32_t size) { return (v + size - 1) % size; }

Machine::Machine(const Config& cfg) : m_cfg(cfg)
{
    if (m_cfg.apTop == 0 || m_cfg.apTop >= IP_SIZE)
        throw std::invalid_argument("dpc::Config::apTop must be 1..99999");
    powerOn();
}

void Machine::powerOn()
{
    m_codeRam.assign(IP_SIZE, OP_NOP);
    m_bootRom.assign(BOOT_SIZE, OP_NOP);
    m_dataMem.assign(m_cfg.apTop + 1, 0);
    m_memReg    = 0;

    m_ip = m_ap = m_loop = 0;
    m_data      = 0;
    m_lock = m_memHere = false;
    m_ipCounted = false;
    m_insn      = OP_NOP;
    m_overflow  = false;

    m_loading   = false;
    m_halted    = true;
    m_phase     = PH_FETCH;
    m_rstType   = RST_NONE;

    m_iret = m_bells = m_memReads = m_memWrites = 0;
    m_rx.clear();
    m_insnIn.clear();
    m_tx.clear();
}

//----------------------------------------------------------------------
// Resets
//
// soft_rst/hard_rst are physical lines: every counter goes to its reset
// position, and IpLine/ApLine/MachineCtrl drop their state. Memories keep
// their contents (ferrite core). A locked data counter is NOT written back:
// its value is lost. IRET only resets with rst_n.
//----------------------------------------------------------------------
void Machine::resetCounters(ResetType type)
{
    m_ip   = (type == RST_HARD) ? BOOT_BASE : 0;   // HARD_RST_D_CNT = 4 -> 99990
    m_ap   = 0;
    m_loop = 0;
    m_data = 0;

    m_lock = m_memHere = false;
    m_ipCounted = false;
    m_overflow  = false;
    m_loading   = false;
    m_phase     = PH_FETCH;

    m_rstType = type;

    bool runOnRst = (type == RST_HARD) ? m_cfg.runOnHardRst : m_cfg.runOnSoftRst;
    m_halted = !runOnRst;
}

void Machine::hardReset() { resetCounters(RST_HARD); }
void Machine::softReset() { resetCounters(RST_SOFT); }

void Machine::run()
{
    m_halted  = false;
    m_rstType = RST_NONE;
}

void Machine::startLoading()
{
    // key_insn_loading_start in S_HALT: loading starts and the machine runs
    m_loading = true;
    run();
}

//----------------------------------------------------------------------
// Halt
//
// IpLine sees halt_rq = IsHalted. If IP already points at the retired
// instruction it steps once more, so that after Run the next instruction
// is read without another increment. The step ignores a pending loop scan.
//----------------------------------------------------------------------
void Machine::enterHalt()
{
    m_halted = true;
    m_phase  = PH_FETCH;
    if (m_ipCounted) {
        m_ip        = wrapInc(m_ip, IP_SIZE);
        m_ipCounted = false;
    }
}

void Machine::bell()
{
    ++m_bells;
    if (onBell)
        onBell();
}

void Machine::cout(uint8_t c)
{
    m_tx.push_back(static_cast<char>(c));
    if (onCout)
        onCout(c);
}

//----------------------------------------------------------------------
// Memories
//----------------------------------------------------------------------
uint8_t Machine::readCode(uint32_t addr) const
{
    return (addr >= BOOT_BASE) ? m_bootRom[addr - BOOT_BASE] : m_codeRam[addr];
}

void Machine::writeCode(uint32_t addr, uint8_t op)
{
    // IpMemory: writes into the bootloader bank are dropped
    if (addr < BOOT_BASE)
        m_codeRam[addr] = op & 0xF;
}

uint8_t Machine::code(uint32_t addr) const
{
    return readCode(addr % IP_SIZE);
}

void Machine::loadCode(const std::vector<uint8_t>& code, uint32_t base)
{
    for (size_t i = 0; i < code.size(); ++i)
        writeCode(static_cast<uint32_t>((base + i) % IP_SIZE), code[i]);
}

void Machine::setBootRom(const std::vector<uint8_t>& rom)
{
    m_bootRom.assign(BOOT_SIZE, OP_NOP);
    for (size_t i = 0; i < rom.size() && i < BOOT_SIZE; ++i)
        m_bootRom[i] = rom[i] & 0xF;
}

void Machine::setCell(uint32_t addr, uint8_t value)
{
    m_dataMem.at(addr) = value;
    if (addr == m_ap && m_memHere)
        m_memReg = value;
}

void Machine::pushInsns(const std::vector<uint8_t>& ops)
{
    for (uint8_t op : ops)
        pushInsn(op);
}

//----------------------------------------------------------------------
// ApLine
//----------------------------------------------------------------------
void Machine::memRead()
{
    ++m_memReads;
    m_memReg  = m_dataMem[m_ap];
    m_memHere = true;
}

// Write-through: the output register takes the written value.
void Machine::flush()
{
    ++m_memWrites;
    m_dataMem[m_ap] = m_data;
    m_memReg  = m_data;
    m_memHere = true;
}

// OP_AP_STEP (> <) and OP_AP_ZERO (CLRA).
// MemLock is the only flag: a locked counter owns the cell and is flushed
// before the address moves, which also releases the lock (REQ-ML-005/007).
// After MemLock this writes the counter into the cell MemLock took over.
void Machine::apMove(bool zero, bool dec)
{
    if (m_lock) {
        flush();
        m_lock = false;
    }
    if (zero)
        m_ap = 0;
    else if (dec)
        m_ap = wrapDec(m_ap, m_cfg.apTop + 1);   // 0 - 1 -> TOP
    else
        m_ap = wrapInc(m_ap, m_cfg.apTop + 1);   // TOP + 1 -> 0
    m_memHere = false;   // lazy read: the new cell is not read now
}

// OP_DATA_STEP (+ -): MemLock is taken before the first step.
void Machine::dataStep(bool dec)
{
    if (!m_lock) {
        if (!m_memHere)
            memRead();
        m_data = m_memReg;
    }
    m_data  = dec ? static_cast<uint8_t>(wrapDec(m_data, DATA_TOP + 1))
                  : static_cast<uint8_t>(wrapInc(m_data, DATA_TOP + 1));
    m_lock  = true;
}

//----------------------------------------------------------------------
// IpLine
//----------------------------------------------------------------------

// MachineCtrl: loop_val_zero = op_q[3] ? ap_zero : data_zero
bool Machine::loopValZero() const
{
    if (m_insn & 0x8)
        return m_ap == 0;                       // { }
    return (m_lock ? m_data : m_memReg) == 0;   // [ ]
}

// OP_NEXT
Status Machine::fetch()
{
    if (!m_ipCounted) {
        // First fetch after reset or halt: read at the current IP
        m_ipCounted = true;
        if (m_loading)
            m_phase = PH_INSN;
        else
            m_insn = readCode(m_ip);
        return Status::Ok;
    }

    if (m_loading) {
        m_ip    = wrapInc(m_ip, IP_SIZE);
        m_phase = PH_INSN;
        return Status::Ok;
    }

    bool isOpen  = isLoopOpen(m_insn);
    bool isClose = isLoopClose(m_insn);
    bool zero    = loopValZero();
    if (isOpen && zero)
        return scan(false);
    if (isClose && !zero)
        return scan(true);

    m_ip   = wrapInc(m_ip, IP_SIZE);
    m_insn = readCode(m_ip);
    return Status::Ok;
}

// Loop scan. The counter takes +1 for the starting bracket, +1 for every
// bracket of the same direction, -1 for every matching one; [ ] and { } are
// counted together (IpLine sees only x11x). The scan stops ON
// the matching bracket, which is then decoded as an ordinary instruction.
// Incrementing past 99 is the hardware error of REQ-CNT-007: the scan is
// aborted and the machine halts. As in the RTL (IpLine, at_top of the loop
// counter) the overflow is caught BEFORE the step: the increment is not
// issued and the counter stays at 99 until a reset. A scan that
// starts with the counter already at 99 overflows at once.
//
// The scan always ends. Program memory is a ring: if the brackets in it are
// balanced or short of own ones, the count reaches zero within one turn;
// otherwise every turn adds the starting bracket again and the count climbs
// to the overflow (about 10^7 reads for a lone bracket).
Status Machine::scan(bool backward)
{

    const uint32_t LOOP_TOP = LOOP_SIZE - 1;

    if (m_loop != LOOP_TOP) {
        ++m_loop;
        for (;;) {
            m_ip   = backward ? wrapDec(m_ip, IP_SIZE) : wrapInc(m_ip, IP_SIZE);
            m_insn = readCode(m_ip);
            if (backward ? isLoopClose(m_insn) : isLoopOpen(m_insn)) {
                if (m_loop == LOOP_TOP)
                    break;               // overflow, stand on this bracket
                ++m_loop;
            }
            else if (isLoopOpen(m_insn) || isLoopClose(m_insn)) {
                --m_loop;
                if (m_loop == 0)
                    return Status::Ok;   // matching bracket, stand on it
            }
        }
    }

    // MachineCtrl halts on the rising edge of loop_overflow (REQ-CTLV2-005)
    m_overflow = true;
    if (m_cfg.bellOnError)
        bell();
    enterHalt();
    return Status::Halted;
}

//----------------------------------------------------------------------
// MachineCtrl
//----------------------------------------------------------------------

// S_INSN_IN: accept one opcode from InsnIn and write it at IP. LD (EOT) is
// the only code that is not written.
bool Machine::takeInsn()
{
    if (m_insnIn.empty())
        return false;
    uint8_t op = m_insnIn.front();
    m_insnIn.pop_front();
    m_insn = op;
    if (op != OP_LD)
        writeCode(m_ip, op);
    return true;
}

// Load mode: only LD (EOT) acts; everything else is just stored.
void Machine::decodeLoading()
{
    if (m_insn == OP_LD) {
        m_loading = false;
        if (m_cfg.softRstOnEot)
            resetCounters(RST_SOFT);
        else
            enterHalt();
    }
}

void Machine::decode()
{
    ++m_iret;

    if (m_loading) {
        decodeLoading();
        return;
    }

    switch (m_insn) {
    case OP_HALT:
        if (m_cfg.bellOnHalt)
            bell();
        enterHalt();
        break;

    // Brackets: IpLine decides at the next fetch. Before [ ] MachineCtrl
    // first makes the zero flag valid (AP_TEST): one read if the cell value
    // is neither in the counter nor in the memory register. { } test AP.
    case OP_LBEG: case OP_LEND:
        if (!m_lock && !m_memHere)
            memRead();
        break;
    case OP_LABEG: case OP_LAEND:
        break;

    case OP_CLRD:                               // [-]  (REQ-ML-009)
        m_data  = 0;
        m_lock  = true;
        break;

    case OP_LD: m_loading = true; break;        // SOT
    case OP_CLRA: apMove(true, false); break;

    case OP_MEMLOCK:                            // flag without a load
        m_lock = true;
        break;

    case OP_INC: case OP_DEC:   dataStep(m_insn & 1); break;      // + -
    case OP_AINC: case OP_ADEC: apMove(false, m_insn & 1); break; // > <

    case OP_COUT:
        // MachineCtrl issues AP_COUT, then tx_vld. Output is always the
        // data counter: without MemLock ApLine loads the cell into it
        // first (reading it if the memory register is elsewhere). MemLock
        // is unchanged (OPEN-017).
        if (!m_lock) {
            if (!m_memHere)
                memRead();
            m_data = m_memReg;
        }
        cout(m_data);
        break;

    case OP_CIN:
        if (m_cfg.bellOnCin)
            bell();
        m_phase = PH_CIN;
        break;

    default:                                    // NOP
        break;
    }
}

// S_CIN_WAIT -> AP_CIN -> optional echo
Status Machine::finishCin()
{
    int c = -1;
    if (onCin)
        c = onCin();
    else if (!m_rx.empty()) {
        c = m_rx.front();
        m_rx.pop_front();
    }
    if (c < 0)
        return Status::WaitInput;

    m_phase = PH_FETCH;
    m_data  = static_cast<uint8_t>(c % (DATA_TOP + 1));
    m_lock  = true;
    if (m_cfg.echoMode)
        cout(m_data);
    return Status::Ok;
}

Status Machine::step()
{
    if (m_halted)
        return Status::Halted;

    switch (m_phase) {
    case PH_CIN:
        return finishCin();
    case PH_INSN:
        break;
    case PH_FETCH: {
        Status s = fetch();
        if (s != Status::Ok)
            return s;
        break;
    }
    }

    if (m_phase == PH_INSN) {
        if (!takeInsn())
            return Status::WaitInput;
        m_phase = PH_FETCH;
    }

    decode();

    if (m_phase == PH_CIN)
        return finishCin();
    return m_halted ? Status::Halted : Status::Ok;
}

Status Machine::runUntilHalt(uint64_t maxSteps)
{
    for (uint64_t i = 0; i < maxSteps; ++i) {
        Status s = step();
        if (s != Status::Ok)
            return s;
    }
    return Status::Ok;
}

} // namespace dpc

//======================================================================
// Standalone runner
//======================================================================
#ifdef EXEC

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <getopt.h>

static void usage()
{
    std::cout <<
        "dpcrun -f <file> [options]\n"
        "  Runs DekatronPC source text: the loader turns it into opcodes,\n"
        "  puts them at address 0 followed by HALT, then Soft Reset + Run.\n"
        "  -b <file>  bootloader source for the ROM at 99990\n"
        "  -B         start with Hard Reset (bootloader) instead of Soft Reset\n"
        "  -a <top>   AP limit, default 29999 (OPEN-001)\n"
        "  -n <max>   stop after <max> instructions\n"
        "  -s         trace every instruction to stderr\n"
        "  -h         this help\n";
}

static bool readFile(const char* path, std::string& text)
{
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open())
        return false;
    text.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}

int main(int argc, char** argv)
{
    const char* filePath = nullptr;
    const char* bootPath = nullptr;
    bool        trace    = false;
    bool        fromBoot = false;
    uint64_t    maxSteps = UINT64_MAX;
    dpc::Config cfg;

    int c;
    while ((c = getopt(argc, argv, "f:b:Ba:n:sh")) != -1) {
        switch (c) {
        case 'f': filePath = optarg; break;
        case 'b': bootPath = optarg; break;
        case 'B': fromBoot = true; break;
        case 'a': cfg.apTop = static_cast<uint32_t>(std::stoul(optarg)); break;
        case 'n': maxSteps = std::stoull(optarg); break;
        case 's': trace = true; break;
        case 'h': usage(); return 0;
        default:  usage(); return -1;
        }
    }

    std::string source;
    if (!filePath || !readFile(filePath, source)) {
        std::cerr << "Input file error, exiting" << std::endl;
        return -1;
    }
    std::vector<uint8_t> code = dpc::assemble(source);
    if (code.empty()) {
        std::cerr << "Input file " << filePath << " has no instructions, exiting" << std::endl;
        return -1;
    }
    code.push_back(dpc::OP_HALT);

    dpc::Machine m(cfg);
    m.loadCode(code);
    if (bootPath) {
        std::string boot;
        if (!readFile(bootPath, boot)) {
            std::cerr << "Bootloader file error, exiting" << std::endl;
            return -1;
        }
        m.setBootRom(dpc::assemble(boot));
    }
    m.onCout = [](uint8_t ch) { std::cout << static_cast<char>(ch) << std::flush; };
    m.onCin  = []() -> int { int ch = std::cin.get(); return ch == EOF ? 0 : ch; };

    if (fromBoot) m.hardReset(); else m.softReset();
    m.run();

    dpc::Status s = dpc::Status::Ok;
    for (uint64_t i = 0; i < maxSteps; ++i) {
        s = m.step();
        if (trace) {
            fprintf(stderr, "IRET:%llu IP:%05u %-5s LOOP:%02u AP:%05u DATA:%3u ML:%d %s\n",
                    static_cast<unsigned long long>(m.iret()), m.ip(),
                    dpc::mnemonic(m.insn()), m.loopCount(), m.ap(),
                    m.txData(), m.memLock(), dpc::statusName(s));
        }
        if (s != dpc::Status::Ok)
            break;
    }

    std::cout << std::endl << "IRET:" << m.iret()
              << " MEM_RD:" << m.memReads() << " MEM_WR:" << m.memWrites()
              << " STATUS:" << dpc::statusName(s) << std::endl;
    if (m.loopOverflow()) {
        std::cerr << "Loop counter overflow" << std::endl;
        return 2;
    }
    return 0;
}
#endif
