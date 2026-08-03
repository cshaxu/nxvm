/* Copyright 2012-2014 Neko. */

#ifndef NTVDM64_CORE_PRODUCT_DEBUG_H
#define NTVDM64_CORE_PRODUCT_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/product/debug/debug_target.h"

typedef struct core_product_debug_context {
    size_t error_position;
    size_t argument_count;
    char **arguments;
    int exit_requested;
    char command_buffer[0x100];
    char command_copy[0x100];
    char file_name[0x100];
    uint16_t dump_segment;
    uint16_t dump_offset;
    uint16_t assemble_segment;
    uint16_t assemble_offset;
    uint16_t unassemble_segment;
    uint16_t unassemble_offset;
    uint16_t parsed_segment;
    uint16_t parsed_offset;
} core_product_debug_context;

void core_product_debug_context_initialize(core_product_debug_context *context);
void debugMain(core_product_debug_context *context,
               const core_product_debug_target *target);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
