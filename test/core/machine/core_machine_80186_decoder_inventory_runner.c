#include "type.h"

#include "core/machine/cpu_instructions.h"

/* A decoder-boundary inventory is intentionally separate from the timing
 * manifest: this runner reports what the current lexical decoder accepts,
 * while the S6 verifier decides whether every such candidate has a manual and
 * ledger disposition.  It is not a retirement or timing conformance test. */
C_INT main(C_VOID)
{
    const C_CHAR *const path = PROJECT_TEST_80186_DECODER_PATH;
    type_bool opcode_seen[0x100] = { TYPE_FALSE };
    type_bool modrm_seen[0x100][0x100] = { { TYPE_FALSE } };
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
                    CORE_MACHINE_CPU_PROFILE_80186, TYPE_FALSE, &lexeme) ||
                !lexeme.available || lexeme.byte_count == 0u) continue;
            ++accepted_pairs;
            opcode_seen[opcode] = TYPE_TRUE;
            modrm_seen[opcode][modrm] = TYPE_TRUE;
        }
    }
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        if (opcode_seen[opcode]) ++accepted_opcodes;
    }
    if (accepted_pairs != 61530u || accepted_opcodes != 247u) return 1;

    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL || STD_FPRINTF(file,
            "{\n  \"schema\": \"nxvm.80186-decoder-inventory.v1\",\n"
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
    if (STD_FPRINTF(file,
            "\n  },\n  \"semantic_only_prefixes\": [\"F0\"]\n}\n") < 0 ||
        STD_FCLOSE(file) != 0) return 1;
    STD_PRINTF("M5:T435:S6:I186-DECODER-LEXEME:%u:%u\n", accepted_pairs,
        accepted_opcodes);
    return 0;
}
