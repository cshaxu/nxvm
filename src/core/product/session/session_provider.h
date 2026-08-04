#ifndef CORE_PRODUCT_SESSION_PROVIDER_H
#define CORE_PRODUCT_SESSION_PROVIDER_H

#include "type.h"

#include "core/product/session/session_interface.h"

typedef struct core_product_session_provider {
    type_status (*open)(C_VOID *context, core_product_session_id id,
        const core_product_session_open_options *options, C_VOID **out_session);
    type_status (*describe)(C_VOID *context, const C_VOID *session,
        core_product_session_snapshot *out_snapshot);
    type_status (*close)(C_VOID *context, C_VOID *session);
    C_VOID *context;
} core_product_session_provider;

#endif
