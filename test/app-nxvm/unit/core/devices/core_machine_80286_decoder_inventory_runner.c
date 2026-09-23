#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/devices/cpu_instructions.h"

/* S7 records the actual lexical decoder universe before comparing it with the
 * 80286 manual ledger.  It is an audit producer, not a timing test. */
C_INT main(C_VOID)
{
    const C_CHAR *const path = PROJECT_TEST_80286_DECODER_PATH;
    type_bool opcode_seen[0x100] = { LIB_FALSE };
    type_bool modrm_seen[0x100][0x100] = { { LIB_FALSE } };
    type_bool escaped_modrm_seen[5][0x100] = { { LIB_FALSE } };
    const lib_u8 escaped_opcodes[5] = { 0x00u, 0x01u, 0x02u,
        0x03u, 0x06u };
    lib_u16 opcode;
    lib_u16 modrm;
    lib_u32 accepted_pairs = 0u;
    lib_u32 accepted_opcodes = 0u;
    FILE *file;

    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            const lib_u8 bytes[15] = {
                (lib_u8)opcode, (lib_u8)modrm
            };
            core_machine_cpu_instruction_lexeme lexeme;

            if (!core_machine_cpu_instruction_lexeme_scan(bytes, sizeof(bytes),
                    CORE_MACHINE_CPU_PROFILE_80286, LIB_FALSE, &lexeme) ||
                !lexeme.available || lexeme.byte_count == 0u) continue;
            ++accepted_pairs;
            opcode_seen[opcode] = LIB_TRUE;
            modrm_seen[opcode][modrm] = LIB_TRUE;
        }
    }
    for (opcode = 0u; opcode < 5u; ++opcode) {
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            const lib_u8 bytes[15] = { 0x0fu,
                escaped_opcodes[opcode], (lib_u8)modrm };
            core_machine_cpu_instruction_lexeme lexeme;

            if (core_machine_cpu_instruction_lexeme_scan(bytes,
                    sizeof(bytes), CORE_MACHINE_CPU_PROFILE_80286,
                    LIB_FALSE, &lexeme) && lexeme.available &&
                lexeme.byte_count != 0u) {
                escaped_modrm_seen[opcode][modrm] = LIB_TRUE;
            }
        }
    }
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        if (opcode_seen[opcode]) ++accepted_opcodes;
    }
    file = fopen(path, "wb");
    if (file == LIB_NULL || fprintf(file,
            "{\n  \"schema\": \"nxvm.80286-decoder-inventory.v1\",\n"
            "  \"lexeme_opcode_modrm_candidates\": %u,\n"
            "  \"lexeme_primary_opcode_count\": %u,\n"
            "  \"lexeme_primary_opcodes\": [",
            accepted_pairs, accepted_opcodes) < 0) {
        if (file != LIB_NULL) fclose(file);
        return 1;
    }
    accepted_opcodes = 0u;
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        if (!opcode_seen[opcode]) continue;
        if ((accepted_opcodes != 0u && fprintf(file, ",") < 0) ||
            fprintf(file, "\"%02X\"", opcode) < 0) {
            fclose(file);
            return 1;
        }
        ++accepted_opcodes;
    }
    if (fprintf(file, "],\n  \"accepted_modrm_masks\": {") < 0) {
        fclose(file);
        return 1;
    }
    accepted_opcodes = 0u;
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        lib_u16 byte;

        if (!opcode_seen[opcode]) continue;
        if ((accepted_opcodes != 0u && fprintf(file, ",") < 0) ||
            fprintf(file, "\n    \"%02X\":\"", opcode) < 0) {
            fclose(file);
            return 1;
        }
        for (byte = 0u; byte < 32u; ++byte) {
            lib_u8 bits = 0u;
            lib_u8 bit;

            for (bit = 0u; bit < 8u; ++bit) {
                if (modrm_seen[opcode][byte * 8u + bit]) bits |= 1u << bit;
            }
            if (fprintf(file, "%02X", bits) < 0) {
                fclose(file);
                return 1;
            }
        }
        if (fprintf(file, "\"") < 0) {
            fclose(file);
            return 1;
        }
        ++accepted_opcodes;
    }
    if (fprintf(file, "\n  },\n  \"accepted_0f_modrm_masks\": {") < 0) {
        fclose(file);
        return 1;
    }
    for (opcode = 0u; opcode < 5u; ++opcode) {
        lib_u16 byte;

        if ((opcode != 0u && fprintf(file, ",") < 0) ||
            fprintf(file, "\n    \"%02X\":\"", escaped_opcodes[opcode]) < 0) {
            fclose(file);
            return 1;
        }
        for (byte = 0u; byte < 32u; ++byte) {
            lib_u8 bits = 0u;
            lib_u8 bit;

            for (bit = 0u; bit < 8u; ++bit) {
                if (escaped_modrm_seen[opcode][byte * 8u + bit]) {
                    bits |= 1u << bit;
                }
            }
            if (fprintf(file, "%02X", bits) < 0) {
                fclose(file);
                return 1;
            }
        }
        if (fprintf(file, "\"") < 0) {
            fclose(file);
            return 1;
        }
    }
    if (fprintf(file,
            "\n  },\n  \"semantic_only_prefixes\": [\"F0\"]\n}\n") < 0 ||
        fclose(file) != 0) return 1;
    STD_PRINTF("M5:T435:S7:I286-DECODER-LEXEME:%u:%u\n", accepted_pairs,
        accepted_opcodes);
    return 0;
}
