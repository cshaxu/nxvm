/* Copyright 2012-2014 Neko. */

#ifndef CORE_PRODUCT_DEBUG_H
#define CORE_PRODUCT_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "core/product/debug/debug_target.h"
#include "core/product/wait_provider.h"

typedef struct core_product_debug_input_provider {
    C_VOID (*flush_console_input)(C_VOID *context);
    C_VOID *context;
} core_product_debug_input_provider;

typedef struct core_product_debug_context {
    const core_product_debug_target *target;
    const core_product_debug_input_provider *input_provider;
    const core_product_wait_scope *wait_scope;
    STD_SIZE_T error_position;
    STD_SIZE_T argument_count;
    C_CHAR **arguments;
    C_INT exit_requested;
    C_CHAR command_buffer[0x100];
    C_CHAR command_copy[0x100];
    C_CHAR file_name[0x100];
    uint16_t dump_segment;
    uint16_t dump_offset;
    uint16_t assemble_segment;
    uint16_t assemble_offset;
    uint16_t unassemble_segment;
    uint16_t unassemble_offset;
    uint16_t parsed_segment;
    uint16_t parsed_offset;
} core_product_debug_context;

C_VOID core_product_debug_context_initialize(core_product_debug_context *context);
C_VOID core_product_debug_main(core_product_debug_context *context,
               const core_product_debug_target *target,
               const core_product_debug_input_provider *input_provider);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
