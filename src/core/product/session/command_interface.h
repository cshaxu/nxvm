#ifndef CORE_PRODUCT_SESSION_COMMAND_INTERFACE_H
#define CORE_PRODUCT_SESSION_COMMAND_INTERFACE_H

#include "type.h"

#include "core/product/session/session_interface.h"

typedef struct core_product_session_output_provider {
    C_VOID (*write_line)(C_VOID *context, const C_CHAR *line);
    C_VOID *context;
} core_product_session_output_provider;

C_INT core_product_session_command_execute(core_product_session_manager *manager,
    C_INT argument_count, const C_CHAR *const *arguments,
    const core_product_session_output_provider *output);

#endif
