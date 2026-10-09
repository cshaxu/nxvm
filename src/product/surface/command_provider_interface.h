#ifndef PRODUCT_SURFACE_COMMAND_PROVIDER_H
#define PRODUCT_SURFACE_COMMAND_PROVIDER_H

#include "product/surface/command_interface.h"

lib_status product_surface_command_provider_initialize(product_surface_command_context *command,
    emulator_machine *machine, emulator_session_display display,
    const product_surface_command_extensions *extensions,
    emulator_session_command_provider *out_provider);

#endif
