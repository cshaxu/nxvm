/* Copyright 2012-2014 Neko. */

#ifndef VM_PRODUCT_CONSOLE_H
#define VM_PRODUCT_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vm/product/console_target.h"

typedef struct nxvm_product_console_context {
    size_t argument_count;
    char **arguments;
    int exit_requested;
    char command_buffer[0x100];
    const nxvm_product_console_target *target;
} nxvm_product_console_context;

void nxvm_product_console_context_initialize(
    nxvm_product_console_context *context);

/* Entry point of NXVM console */
void vm_product_console_main(nxvm_product_console_context *context,
                 const nxvm_product_console_target *target);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
