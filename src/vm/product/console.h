/* Copyright 2012-2014 Neko. */

#ifndef VM_PRODUCT_CONSOLE_H
#define VM_PRODUCT_CONSOLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "type.h"

#include "vm/product/console_machine_provider.h"
#include "core/product/session/session_interface.h"

    typedef struct vm_product_console_context
    {
        STD_SIZE_T argument_count;
        C_CHAR **arguments;
        C_INT exit_requested;
        C_CHAR command_buffer[0x100];
        const vm_product_console_machine_provider *machine_provider;
        core_product_session_manager *session_manager;
    } vm_product_console_context;

    C_VOID vm_product_console_context_initialize(
        vm_product_console_context *context);

    /* Entry point of product console */
    C_VOID vm_product_console_main(vm_product_console_context *context,
                                   const vm_product_console_machine_provider *machine_provider,
                                   core_product_session_manager *session_manager);

#ifdef __cplusplus
} /*_EOCD_*/
#endif

#endif
