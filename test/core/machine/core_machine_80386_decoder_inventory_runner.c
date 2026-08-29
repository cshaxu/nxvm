#include "type.h"

#include "core/machine/cpu_instructions.h"

/* S8 records the 80386DX lexical decoder universe.  It is audit evidence,
 * not a timing or semantic conformance test. */
C_INT main(C_VOID)
{
    const C_CHAR *const path =
        "docs/etc/cpu-timing/t435-s8-80386-decoder-inventory.json";
    type_bool primary_seen[0x100] = { TYPE_FALSE };
    type_bool primary_masks[0x100][0x100] = { { TYPE_FALSE } };
    type_bool escaped_masks[0x100][0x100] = { { TYPE_FALSE } };
    type_unsigned_16 opcode;
    type_unsigned_16 modrm;
    type_unsigned_32 pairs = 0u;
    type_unsigned_32 primary_count = 0u;
    type_unsigned_32 escaped_count = 0u;
    STD_FILE *file;

    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            const type_unsigned_8 bytes[15] = {
                (type_unsigned_8)opcode, (type_unsigned_8)modrm
            };
            core_machine_cpu_instruction_lexeme lexeme;

            if (!core_machine_cpu_instruction_lexeme_scan(bytes, sizeof(bytes),
                    CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, &lexeme) ||
                !lexeme.available || lexeme.byte_count == 0u) continue;
            ++pairs;
            primary_seen[opcode] = TYPE_TRUE;
            primary_masks[opcode][modrm] = TYPE_TRUE;
        }
    }
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            const type_unsigned_8 bytes[15] = { 0x0fu,
                (type_unsigned_8)opcode, (type_unsigned_8)modrm };
            core_machine_cpu_instruction_lexeme lexeme;

            if (core_machine_cpu_instruction_lexeme_scan(bytes, sizeof(bytes),
                    CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE, &lexeme) &&
                lexeme.available && lexeme.byte_count != 0u) {
                escaped_masks[opcode][modrm] = TYPE_TRUE;
            }
        }
        if (primary_seen[opcode]) ++primary_count;
    }
    if (pairs != 63021u || primary_count != 253u) return 1;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL || STD_FPRINTF(file,
            "{\n  \"schema\": \"nxvm.80386-decoder-inventory.v1\",\n"
            "  \"lexeme_opcode_modrm_candidates\": %u,\n"
            "  \"lexeme_primary_opcode_count\": %u,\n"
            "  \"accepted_modrm_masks\": {", pairs, primary_count) < 0) {
        if (file != STD_NULL) STD_FCLOSE(file);
        return 1;
    }
    for (opcode = 0u, primary_count = 0u; opcode <= 0xffu; ++opcode) {
        type_unsigned_16 byte;
        if (!primary_seen[opcode]) continue;
        if ((primary_count != 0u && STD_FPRINTF(file, ",") < 0) ||
            STD_FPRINTF(file, "\n    \"%02X\":\"", opcode) < 0) return 1;
        for (byte = 0u; byte < 32u; ++byte) {
            type_unsigned_8 bits = 0u;
            type_unsigned_8 bit;
            for (bit = 0u; bit < 8u; ++bit) {
                if (primary_masks[opcode][byte * 8u + bit]) bits |= 1u << bit;
            }
            if (STD_FPRINTF(file, "%02X", bits) < 0) return 1;
        }
        if (STD_FPRINTF(file, "\"") < 0) return 1;
        ++primary_count;
    }
    if (STD_FPRINTF(file, "\n  },\n  \"accepted_0f_modrm_masks\": {") < 0) return 1;
    for (opcode = 0u, escaped_count = 0u; opcode <= 0xffu; ++opcode) {
        type_unsigned_16 byte;
        type_bool any = TYPE_FALSE;
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            if (escaped_masks[opcode][modrm]) any = TYPE_TRUE;
        }
        if (!any) continue;
        if ((escaped_count != 0u && STD_FPRINTF(file, ",") < 0) ||
            STD_FPRINTF(file, "\n    \"%02X\":\"", opcode) < 0) return 1;
        for (byte = 0u; byte < 32u; ++byte) {
            type_unsigned_8 bits = 0u;
            type_unsigned_8 bit;
            for (bit = 0u; bit < 8u; ++bit) {
                if (escaped_masks[opcode][byte * 8u + bit]) bits |= 1u << bit;
            }
            if (STD_FPRINTF(file, "%02X", bits) < 0) return 1;
        }
        if (STD_FPRINTF(file, "\"") < 0) return 1;
        ++escaped_count;
    }
    if (STD_FPRINTF(file, "\n  }\n}\n") < 0 || STD_FCLOSE(file) != 0) return 1;
    STD_PRINTF("M5:T435:S8:I386-DECODER-LEXEME:%u:%u\n", pairs, primary_count);
    return 0;
}
