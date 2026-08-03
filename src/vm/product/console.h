/* Copyright 2012-2014 Neko. */

#ifndef VM_PRODUCT_CONSOLE_H
#define VM_PRODUCT_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "vm/product/console_target.h"

typedef struct nxvm_product_console_context {
    size_t argument_count;
    C_CHAR **arguments;
    C_INT exit_requested;
    C_CHAR command_buffer[0x100];
    const nxvm_product_console_target *target;
} nxvm_product_console_context;

C_VOID nxvm_product_console_context_initialize(
    nxvm_product_console_context *context);

/* Entry point of NXVM console */
C_VOID vm_product_console_main(nxvm_product_console_context *context,
                 const nxvm_product_console_target *target);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
