/* Copyright 2012-2014 Neko. */

#ifndef VM_APP_COMMAND_H
#define VM_APP_COMMAND_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "type.h"

#include "app/composition.h"
#include "app/ini_interface.h"

    typedef struct vm_app_console_context vm_app_console_context;

    type_status vm_app_console_context_create(
        vm_app_console_context **out_context);
    C_VOID vm_app_console_context_destroy(
        vm_app_console_context *context);

    /* Entry point of product console */
C_VOID vm_app_console_main(vm_app_console_context *context,
    vm_app *session,
    const C_CHAR *ini_path);

#ifdef __cplusplus
} /*_EOCD_*/
#endif

#endif
