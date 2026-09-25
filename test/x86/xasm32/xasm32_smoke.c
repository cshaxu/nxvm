#include "lib/types/test.h"
#include "x86/xasm32/xasm32_interface.h"

/* Golden digest captured before the dispatch-table relocation. Hash bytes and
 * lengths, never pointer values or host-width representations. */
static lib_u32 dispatch_digest(void)
{
    static const lib_u8 addressing[] = {0x00, 0x04, 0x05, 0x44, 0x85, 0xc0, 0xdb, 0xff};
    static const lib_u8 prefixes[] = {0, 0x66, 0x67};
    lib_u32 hash = 2166136261u;
    lib_u8 code[16];
    char statement[X86_XASM32_MAX_STATEMENT_BYTES + 1u];
    for (lib_u32 opcode = 0; opcode < 256; ++opcode) {
        for (lib_u32 map = 0; map < 2; ++map) {
            for (lib_u32 prefix = 0; prefix < sizeof(prefixes); ++prefix) {
                for (lib_u32 addr = 0; addr < sizeof(addressing); ++addr) {
                    for (lib_i32 mode = 0; mode < 2; ++mode) {
                        lib_size length = 0, text_length = 0, at = 0;
                        lib_status status;
                        lib_memory_set(code, 0xa5, sizeof(code));
                        if (prefix) code[at++] = prefixes[prefix];
                        if (map) code[at++] = 0x0f;
                        code[at++] = (lib_u8)opcode;
                        code[at] = addressing[addr];
                        status = x86_xasm32_disassemble(code, sizeof(code),
                            statement, sizeof(statement), &text_length, &length, mode);
                        if (status != LIB_STATUS_OK && status != LIB_STATUS_UNSUPPORTED)
                            return 0;
                        hash = (hash ^ (status == LIB_STATUS_OK)) * 16777619u;
                        hash = (hash ^ (lib_u32)length) * 16777619u;
                        hash = (hash ^ (lib_u32)text_length) * 16777619u;
                        if (status == LIB_STATUS_OK) {
                            for (lib_size i = 0; i < text_length; ++i)
                                hash = (hash ^ (lib_u8)statement[i]) * 16777619u;
                        }
                    }
                }
            }
        }
    }
    return hash;
}

int main(void)
{
    lib_u8 code[16] = {0};
    char statement[X86_XASM32_MAX_STATEMENT_BYTES + 1u] = {0};
    lib_size length;
    lib_size statement_length;
    lib_size paragraph_length;

    if (dispatch_digest() != 0x70f241beu) return 19;

    if (x86_xasm32_assemble("nop", 3u, code, sizeof(code), &length,
            LIB_TRUE) != LIB_STATUS_OK || length != 1u || code[0] != 0x90u) return 11;
    if (x86_xasm32_disassemble(code, sizeof(code), statement,
            sizeof(statement), &statement_length, &length, LIB_TRUE) != LIB_STATUS_OK ||
        statement_length != lib_text_length(statement) || statement_length <= 1u ||
        length != 1u) return 12;

    if (x86_xasm32_assemble_paragraph("nop\nnop", 7u, code,
            sizeof(code), &paragraph_length, LIB_TRUE) != LIB_STATUS_OK ||
        paragraph_length != 2u || code[0] != 0x90u || code[1] != 0x90u) return 13;

    if (x86_xasm32_assemble("mov dr0,eax", 11u, code, sizeof(code), &length,
            LIB_TRUE) != LIB_STATUS_OK) return 14;
    if (length != 4u || code[0] != 0x67u) return 141;
    if (code[1] != 0x0fu) return (lib_i32)code[1];
    if (code[2] != 0x23u) return (lib_i32)code[2];
    if (code[3] != 0xc0u) return (lib_i32)code[3];
    if (x86_xasm32_assemble("mov eax,dr0", 11u, code, sizeof(code), &length,
            LIB_TRUE) != LIB_STATUS_OK || length != 4u || code[0] != 0x67u ||
        code[1] != 0x0fu || code[2] != 0x21u || code[3] != 0xc0u) return 15;
    if (x86_xasm32_assemble("mov tr6,eax", 11u, code, sizeof(code), &length,
            LIB_TRUE) != LIB_STATUS_OK || length != 4u || code[0] != 0x67u ||
        code[1] != 0x0fu || code[2] != 0x26u || code[3] != 0xf0u) return 16;
    if (x86_xasm32_assemble("mov eax,tr6", 11u, code, sizeof(code), &length,
            LIB_TRUE) != LIB_STATUS_OK || length != 4u || code[0] != 0x67u ||
        code[1] != 0x0fu || code[2] != 0x24u || code[3] != 0xf0u) return 17;

    /* Cover every ModRM/SIB path; non-SIB addressing must not use SIB state. */
    code[0] = 0x8bu; /* MOV r32,r/m32 */
    for (lib_u32 modrm = 0; modrm < 256; ++modrm) {
        lib_u32 mod = modrm >> 6, rm = modrm & 7;
        lib_bool has_sib = mod != 3 && rm == 4;
        code[1] = (lib_u8)modrm;
        for (lib_u32 sib = 0; sib < (has_sib ? 256u : 1u); ++sib) {
            lib_size expected = 2u + (has_sib ? 1u : 0u);
            lib_memory_set(code + 2, 0, sizeof(code) - 2);
            code[2] = (lib_u8)sib;
            if (mod == 1) ++expected;
            if (mod == 2 || (mod == 0 && (rm == 5 ||
                    (has_sib && (sib & 7) == 5)))) expected += 4;
            if (x86_xasm32_disassemble(code, sizeof(code), statement,
                    sizeof(statement), &statement_length, &length, LIB_TRUE) !=
                    LIB_STATUS_OK || length != expected) return 18;
        }
    }
    return 0;
}
