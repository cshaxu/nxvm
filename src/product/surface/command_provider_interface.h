#ifndef PRODUCT_SURFACE_COMMAND_PROVIDER_H
#define PRODUCT_SURFACE_COMMAND_PROVIDER_H

#include "product/surface/command_interface.h"

/* This builds the one PC Session-provider adapter. The adapter delegates
 * fixed monitor syntax to Emulator Product; it is not a second parser. */
lib_status product_surface_command_provider_initialize(product_surface_command_context *command,
    emulator_machine *machine, emulator_session_display display,
    const product_surface_command_extensions *extensions,
    emulator_session_command_provider *out_provider);

#endif
