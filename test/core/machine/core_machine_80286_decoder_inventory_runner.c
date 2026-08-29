#include "type.h"

#include "core/machine/cpu_instructions.h"

/* S7 records the actual lexical decoder universe before comparing it with the
 * 80286 manual ledger.  It is an audit producer, not a timing test. */
C_INT main(C_VOID)
{
    const C_CHAR *const path = PROJECT_TEST_80286_DECODER_PATH;
    type_bool opcode_seen[0x100] = { TYPE_FALSE };
    type_bool modrm_seen[0x100][0x100] = { { TYPE_FALSE } };
    type_bool escaped_modrm_seen[5][0x100] = { { TYPE_FALSE } };
    const type_unsigned_8 escaped_opcodes[5] = { 0x00u, 0x01u, 0x02u,
        0x03u, 0x06u };
    type_unsigned_16 opcode;
    type_unsigned_16 modrm;
    type_unsigned_32 accepted_pairs = 0u;
    type_unsigned_32 accepted_opcodes = 0u;
    STD_FILE *file;

    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            const type_unsigned_8 bytes[15] = {
                (type_unsigned_8)opcode, (type_unsigned_8)modrm
            };
            core_machine_cpu_instruction_lexeme lexeme;

            if (!core_machine_cpu_instruction_lexeme_scan(bytes, sizeof(bytes),
                    CORE_MACHINE_CPU_PROFILE_80286, TYPE_FALSE, &lexeme) ||
                !lexeme.available || lexeme.byte_count == 0u) continue;
            ++accepted_pairs;
            opcode_seen[opcode] = TYPE_TRUE;
            modrm_seen[opcode][modrm] = TYPE_TRUE;
        }
    }
    for (opcode = 0u; opcode < 5u; ++opcode) {
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            const type_unsigned_8 bytes[15] = { 0x0fu,
                escaped_opcodes[opcode], (type_unsigned_8)modrm };
            core_machine_cpu_instruction_lexeme lexeme;

            if (core_machine_cpu_instruction_lexeme_scan(bytes,
                    sizeof(bytes), CORE_MACHINE_CPU_PROFILE_80286,
                    TYPE_FALSE, &lexeme) && lexeme.available &&
                lexeme.byte_count != 0u) {
                escaped_modrm_seen[opcode][modrm] = TYPE_TRUE;
            }
        }
    }
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        if (opcode_seen[opcode]) ++accepted_opcodes;
    }
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL || STD_FPRINTF(file,
            "{\n  \"schema\": \"nxvm.80286-decoder-inventory.v1\",\n"
            "  \"lexeme_opcode_modrm_candidates\": %u,\n"
            "  \"lexeme_primary_opcode_count\": %u,\n"
            "  \"lexeme_primary_opcodes\": [",
            accepted_pairs, accepted_opcodes) < 0) {
        if (file != STD_NULL) STD_FCLOSE(file);
        return 1;
    }
    accepted_opcodes = 0u;
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        if (!opcode_seen[opcode]) continue;
        if ((accepted_opcodes != 0u && STD_FPRINTF(file, ",") < 0) ||
            STD_FPRINTF(file, "\"%02X\"", opcode) < 0) {
            STD_FCLOSE(file);
            return 1;
        }
        ++accepted_opcodes;
    }
    if (STD_FPRINTF(file, "],\n  \"accepted_modrm_masks\": {") < 0) {
        STD_FCLOSE(file);
        return 1;
    }
    accepted_opcodes = 0u;
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        type_unsigned_16 byte;

        if (!opcode_seen[opcode]) continue;
        if ((accepted_opcodes != 0u && STD_FPRINTF(file, ",") < 0) ||
            STD_FPRINTF(file, "\n    \"%02X\":\"", opcode) < 0) {
            STD_FCLOSE(file);
            return 1;
        }
        for (byte = 0u; byte < 32u; ++byte) {
            type_unsigned_8 bits = 0u;
            type_unsigned_8 bit;

            for (bit = 0u; bit < 8u; ++bit) {
                if (modrm_seen[opcode][byte * 8u + bit]) bits |= 1u << bit;
            }
            if (STD_FPRINTF(file, "%02X", bits) < 0) {
                STD_FCLOSE(file);
                return 1;
            }
        }
        if (STD_FPRINTF(file, "\"") < 0) {
            STD_FCLOSE(file);
            return 1;
        }
        ++accepted_opcodes;
    }
    if (STD_FPRINTF(file, "\n  },\n  \"accepted_0f_modrm_masks\": {") < 0) {
        STD_FCLOSE(file);
        return 1;
    }
    for (opcode = 0u; opcode < 5u; ++opcode) {
        type_unsigned_16 byte;

        if ((opcode != 0u && STD_FPRINTF(file, ",") < 0) ||
            STD_FPRINTF(file, "\n    \"%02X\":\"", escaped_opcodes[opcode]) < 0) {
            STD_FCLOSE(file);
            return 1;
        }
        for (byte = 0u; byte < 32u; ++byte) {
            type_unsigned_8 bits = 0u;
            type_unsigned_8 bit;

            for (bit = 0u; bit < 8u; ++bit) {
                if (escaped_modrm_seen[opcode][byte * 8u + bit]) {
                    bits |= 1u << bit;
                }
            }
            if (STD_FPRINTF(file, "%02X", bits) < 0) {
                STD_FCLOSE(file);
                return 1;
            }
        }
        if (STD_FPRINTF(file, "\"") < 0) {
            STD_FCLOSE(file);
            return 1;
        }
    }
    if (STD_FPRINTF(file,
            "\n  },\n  \"semantic_only_prefixes\": [\"F0\"]\n}\n") < 0 ||
        STD_FCLOSE(file) != 0) return 1;
    STD_PRINTF("M5:T435:S7:I286-DECODER-LEXEME:%u:%u\n", accepted_pairs,
        accepted_opcodes);
    return 0;
}
