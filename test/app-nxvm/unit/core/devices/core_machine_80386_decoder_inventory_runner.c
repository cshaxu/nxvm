#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/cpu_instructions.h"

/* S8 records the 80386DX lexical decoder universe.  It is audit evidence,
 * not a timing or semantic conformance test. */
lib_i32 main(void)
{
    const char *const path = PROJECT_TEST_80386_DECODER_PATH;
    lib_u8 primary_seen[0x100] = { LIB_FALSE };
    lib_u8 primary_masks[0x100][0x100] = { { LIB_FALSE } };
    lib_u8 escaped_masks[0x100][0x100] = { { LIB_FALSE } };
    lib_u16 opcode;
    lib_u16 modrm;
    lib_u32 pairs = 0u;
    lib_u32 primary_count = 0u;
    lib_u32 escaped_count = 0u;
    FILE *file;

    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            const lib_u8 bytes[15] = {
                (lib_u8)opcode, (lib_u8)modrm
            };
            core_machine_cpu_instruction_lexeme lexeme;

            if (!core_machine_cpu_instruction_lexeme_scan(bytes, sizeof(bytes),
                    CORE_MACHINE_CPU_PROFILE_80386, LIB_FALSE, &lexeme) ||
                !lexeme.available || lexeme.byte_count == 0u) continue;
            ++pairs;
            primary_seen[opcode] = LIB_TRUE;
            primary_masks[opcode][modrm] = LIB_TRUE;
        }
    }
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            const lib_u8 bytes[15] = { 0x0fu,
                (lib_u8)opcode, (lib_u8)modrm };
            core_machine_cpu_instruction_lexeme lexeme;

            if (core_machine_cpu_instruction_lexeme_scan(bytes, sizeof(bytes),
                    CORE_MACHINE_CPU_PROFILE_80386, LIB_FALSE, &lexeme) &&
                lexeme.available && lexeme.byte_count != 0u) {
                escaped_masks[opcode][modrm] = LIB_TRUE;
            }
        }
        if (primary_seen[opcode]) ++primary_count;
    }
    if (pairs != 63021u || primary_count != 253u) return 1;
    file = fopen(path, "wb");
    if (file == LIB_NULL || fprintf(file,
            "{\n  \"schema\": \"nxvm.80386-decoder-inventory.v1\",\n"
            "  \"lexeme_opcode_modrm_candidates\": %u,\n"
            "  \"lexeme_primary_opcode_count\": %u,\n"
            "  \"accepted_modrm_masks\": {", pairs, primary_count) < 0) {
        if (file != LIB_NULL) fclose(file);
        return 1;
    }
    for (opcode = 0u, primary_count = 0u; opcode <= 0xffu; ++opcode) {
        lib_u16 byte;
        if (!primary_seen[opcode]) continue;
        if ((primary_count != 0u && fprintf(file, ",") < 0) ||
            fprintf(file, "\n    \"%02X\":\"", opcode) < 0) return 1;
        for (byte = 0u; byte < 32u; ++byte) {
            lib_u8 bits = 0u;
            lib_u8 bit;
            for (bit = 0u; bit < 8u; ++bit) {
                if (primary_masks[opcode][byte * 8u + bit]) bits |= 1u << bit;
            }
            if (fprintf(file, "%02X", bits) < 0) return 1;
        }
        if (fprintf(file, "\"") < 0) return 1;
        ++primary_count;
    }
    if (fprintf(file, "\n  },\n  \"accepted_0f_modrm_masks\": {") < 0) return 1;
    for (opcode = 0u, escaped_count = 0u; opcode <= 0xffu; ++opcode) {
        lib_u16 byte;
        lib_u8 any = LIB_FALSE;
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            if (escaped_masks[opcode][modrm]) any = LIB_TRUE;
        }
        if (!any) continue;
        if ((escaped_count != 0u && fprintf(file, ",") < 0) ||
            fprintf(file, "\n    \"%02X\":\"", opcode) < 0) return 1;
        for (byte = 0u; byte < 32u; ++byte) {
            lib_u8 bits = 0u;
            lib_u8 bit;
            for (bit = 0u; bit < 8u; ++bit) {
                if (escaped_masks[opcode][byte * 8u + bit]) bits |= 1u << bit;
            }
            if (fprintf(file, "%02X", bits) < 0) return 1;
        }
        if (fprintf(file, "\"") < 0) return 1;
        ++escaped_count;
    }
    if (fprintf(file, "\n  }\n}\n") < 0 || fclose(file) != 0) return 1;
    printf("M5:T435:S8:I386-DECODER-LEXEME:%u:%u\n", pairs, primary_count);
    return 0;
}
