#ifndef X86_PRODUCT_COMMAND_PROVIDER_H
#define X86_PRODUCT_COMMAND_PROVIDER_H

#include "x86/product/command_interface.h"

/* This builds the one PC Session-provider adapter. The adapter delegates
 * fixed monitor syntax to Emulator Product; it is not a second parser. */
lib_status x86_product_command_provider_initialize(x86_product_command_context *command,
    emulator_machine *machine, emulator_session_display display,
    const x86_product_command_extensions *extensions,
    emulator_session_command_provider *out_provider);

#endif
