#include "common/xasm32/xasm32_interface.h"

#include "common/xasm32/aasm32.h"
#include "common/xasm32/dasm32.h"

#include <stdlib.h>
#include <string.h>

static lib_status common_xasm32_validate_statement(const char *statement,
    lib_size statement_bytes, int paragraph)
{
    lib_size index;
    lib_size line_bytes = 0u;

    if (statement == LIB_NULL || statement_bytes == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < statement_bytes; ++index) {
        if (statement[index] == '\0') return LIB_STATUS_INVALID_ARGUMENT;
        if (statement[index] == '\n' && paragraph) {
            if (line_bytes > COMMON_XASM32_MAX_STATEMENT_BYTES) return LIB_STATUS_INVALID_ARGUMENT;
            line_bytes = 0u;
        } else if (++line_bytes > COMMON_XASM32_MAX_STATEMENT_BYTES) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    return LIB_STATUS_OK;
}

lib_status common_xasm32_assemble(const char *statement, lib_size statement_bytes,
    lib_u8 *code, lib_size code_capacity, lib_size *out_code_bytes, int flag32)
{
    char local_statement[COMMON_XASM32_MAX_STATEMENT_BYTES + 1u];
    lib_u8 local_code[COMMON_XASM32_MAX_CODE_BYTES];
    lib_u8 code_bytes;
    lib_status status;

    if (code == LIB_NULL || out_code_bytes == LIB_NULL || code_capacity == 0u ||
        (flag32 != LIB_FALSE && flag32 != LIB_TRUE)) return LIB_STATUS_INVALID_ARGUMENT;
    status = common_xasm32_validate_statement(statement, statement_bytes, LIB_FALSE);
    if (status != LIB_STATUS_OK) return status;
    memcpy(local_statement, statement, statement_bytes);
    local_statement[statement_bytes] = '\0';
    code_bytes = aasm32(local_statement, local_code, flag32);
    if (code_bytes == 0u) return LIB_STATUS_UNSUPPORTED;
    if (code_bytes > code_capacity) return LIB_STATUS_LIMIT_EXCEEDED;
    memcpy(code, local_code, code_bytes);
    *out_code_bytes = code_bytes;
    return LIB_STATUS_OK;
}

lib_status common_xasm32_assemble_paragraph(const char *statement,
    lib_size statement_bytes, lib_u8 *code, lib_size code_capacity,
    lib_size *out_code_bytes, int flag32)
{
    char *local_statement;
    lib_status status;

    if (code == LIB_NULL || out_code_bytes == LIB_NULL || code_capacity == 0u ||
        statement_bytes == (lib_size)-1 ||
        (flag32 != LIB_FALSE && flag32 != LIB_TRUE)) return LIB_STATUS_INVALID_ARGUMENT;
    status = common_xasm32_validate_statement(statement, statement_bytes, LIB_TRUE);
    if (status != LIB_STATUS_OK) return status;
    local_statement = malloc(statement_bytes + 1u);
    if (local_statement == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    memcpy(local_statement, statement, statement_bytes);
    local_statement[statement_bytes] = '\0';
    status = aasm32x(local_statement, code_capacity, code, out_code_bytes, flag32);
    free(local_statement);
    return status;
}

lib_status common_xasm32_disassemble(const lib_u8 *code, lib_size code_bytes,
    char *statement, lib_size statement_capacity, lib_size *out_statement_bytes,
    int flag32)
{
    char local_statement[COMMON_XASM32_MAX_STATEMENT_BYTES + 1u];
    lib_u8 decoded_bytes;
    lib_size statement_bytes;

    if (code == LIB_NULL || statement == LIB_NULL || out_statement_bytes == LIB_NULL ||
        code_bytes < COMMON_XASM32_MAX_CODE_BYTES || statement_capacity == 0u ||
        (flag32 != LIB_FALSE && flag32 != LIB_TRUE)) return LIB_STATUS_INVALID_ARGUMENT;
    decoded_bytes = dasm32(local_statement, (lib_u8 *)code, flag32);
    statement_bytes = strlen(local_statement);
    if (decoded_bytes == 0u) return LIB_STATUS_UNSUPPORTED;
    if (statement_bytes >= statement_capacity) return LIB_STATUS_LIMIT_EXCEEDED;
    memcpy(statement, local_statement, statement_bytes + 1u);
    *out_statement_bytes = statement_bytes;
    return LIB_STATUS_OK;
}
