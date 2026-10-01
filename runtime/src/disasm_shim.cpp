#include "psx_disasm.h"
#include "mips_decoder.h"

#include <cstdio>
#include <cstring>

/* C-linkage bridge so the C debug server can use the C++ MIPS decoder that the
 * recompiler already ships (recompiler/src/mips_decoder.cpp). */
extern "C" int psx_disasm_one(uint32_t word, uint32_t addr, char* out, int cap) {
    if (!out || cap <= 0) return 0;
    using PSXRecomp::DecodedInstruction;
    using PSXRecomp::InstrFormat;
    using PSXRecomp::MipsDecoder;

    DecodedInstruction d = MipsDecoder::decode(word, addr);
    const char* m = d.mnemonic ? d.mnemonic : "???";
    auto rn = [](uint8_t r) { return MipsDecoder::register_name(r); };

    int n;
    if (std::strcmp(m, "JR") == 0) {
        n = std::snprintf(out, (size_t)cap, "%-8s %s", m, rn(d.rs));
    } else if (std::strcmp(m, "JALR") == 0) {
        n = std::snprintf(out, (size_t)cap, "%-8s %s, %s", m, rn(d.rd), rn(d.rs));
    } else if (std::strcmp(m, "SLL") == 0 || std::strcmp(m, "SRL") == 0 ||
               std::strcmp(m, "SRA") == 0) {
        n = std::snprintf(out, (size_t)cap, "%-8s %s, %s, %d", m, rn(d.rd),
                          rn(d.rt), d.shamt);
    } else if (std::strcmp(m, "MFHI") == 0 || std::strcmp(m, "MFLO") == 0) {
        n = std::snprintf(out, (size_t)cap, "%-8s %s", m, rn(d.rd));
    } else if (std::strcmp(m, "MTHI") == 0 || std::strcmp(m, "MTLO") == 0) {
        n = std::snprintf(out, (size_t)cap, "%-8s %s", m, rn(d.rs));
    } else if (std::strcmp(m, "SYSCALL") == 0 || std::strcmp(m, "BREAK") == 0) {
        n = std::snprintf(out, (size_t)cap, "%s", m);
    } else if (d.format == InstrFormat::J) {
        n = std::snprintf(out, (size_t)cap, "%-8s 0x%08X", m, d.jump_target);
    } else if (d.is_branch) {
        n = std::snprintf(out, (size_t)cap, "%-8s %s, %s, 0x%08X", m, rn(d.rs),
                          rn(d.rt), d.branch_target);
    } else if (d.opcode == 0x0F) { /* LUI */
        n = std::snprintf(out, (size_t)cap, "%-8s %s, 0x%04X", m, rn(d.rt),
                          (unsigned)d.uimm16);
    } else if (d.is_load || d.is_store) {
        n = std::snprintf(out, (size_t)cap, "%-8s %s, %d(%s)", m, rn(d.rt),
                          (int)d.imm16, rn(d.rs));
    } else if (d.format == InstrFormat::R) {
        n = std::snprintf(out, (size_t)cap, "%-8s %s, %s, %s", m, rn(d.rd),
                          rn(d.rs), rn(d.rt));
    } else {
        n = std::snprintf(out, (size_t)cap, "%-8s %s, %s, %d", m, rn(d.rt),
                          rn(d.rs), (int)d.imm16);
    }
    if (n < 0) { out[0] = '\0'; return 0; }
    return (n < cap) ? n : (cap - 1);
}
