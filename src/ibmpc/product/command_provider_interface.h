#ifndef APP_COMMAND_PROVIDER_H
#define APP_COMMAND_PROVIDER_H

#include "ibmpc/product/command_interface.h"

lib_status app_command_provider_initialize(app_command_context *command,
    common_machine *machine, common_session_display display,
    const app_command_extensions *extensions,
    common_session_command_provider *out_provider);

#endif
