#ifndef X86_XASM32_INTERFACE_H
#define X86_XASM32_INTERFACE_H

#include "lib/types/types_interface.h"

#define X86_XASM32_MAX_STATEMENT_BYTES 255u
#define X86_XASM32_MAX_CODE_BYTES 15u

lib_status x86_xasm32_assemble(const char *statement, lib_size statement_bytes,
    lib_u8 *code, lib_size code_capacity, lib_size *out_code_bytes, lib_bool flag32);
lib_status x86_xasm32_assemble_paragraph(const char *statement,
    lib_size statement_bytes, lib_u8 *code, lib_size code_capacity,
    lib_size *out_code_bytes, lib_bool flag32);
lib_status x86_xasm32_disassemble(const lib_u8 *code, lib_size code_bytes,
    char *statement, lib_size statement_capacity, lib_size *out_statement_bytes,
    lib_size *out_code_bytes, lib_bool flag32);

#endif
