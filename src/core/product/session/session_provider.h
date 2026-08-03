#ifndef NTVDM64_CORE_PRODUCT_SESSION_PROVIDER_H
#define NTVDM64_CORE_PRODUCT_SESSION_PROVIDER_H

#include "type.h"

#include "core/product/session/session_interface.h"

typedef struct core_product_session_provider {
    ntvdm64_status (*open)(C_VOID *context, core_product_session_id id,
        C_VOID **out_session);
    ntvdm64_status (*describe)(C_VOID *context, const C_VOID *session,
        core_product_session_snapshot *out_snapshot);
    ntvdm64_status (*close)(C_VOID *context, C_VOID *session);
    C_VOID *context;
} core_product_session_provider;

#endif
