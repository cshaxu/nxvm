/* Copyright 2012-2014 Neko. */

#ifndef VM_PRODUCT_CONSOLE_H
#define VM_PRODUCT_CONSOLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "type.h"

#include "vm/product/console_machine_provider.h"
#include "vm/product/session_catalog.h"
#include "core/product/session/session_interface.h"

    typedef struct vm_product_console_context vm_product_console_context;

    type_status vm_product_console_context_create(
        vm_product_console_context **out_context);
    C_VOID vm_product_console_context_destroy(
        vm_product_console_context *context);

    /* Entry point of product console */
    C_VOID vm_product_console_main(vm_product_console_context *context,
                                   const vm_session_machine_provider *machine_provider,
                                   core_product_session_manager *session_manager,
                                   const C_CHAR *profile_directory);

#ifdef __cplusplus
} /*_EOCD_*/
#endif

#endif
