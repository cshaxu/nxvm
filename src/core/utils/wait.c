#include "type.h"

#include "core/utils/wait.h"
#include "core/utils/wait_provider.h"

C_VOID core_utils_wait_scope_initialize(core_utils_wait_scope *scope,
    core_utils_wait_provider provider, C_VOID *context)
{
    if (scope == STD_NULL) return;
    scope->provider = provider;
    scope->context = context;
}

C_VOID core_utils_wait_milliseconds(const core_utils_wait_scope *scope,
    uint32_t milliseconds)
{
    if (scope == STD_NULL || scope->provider == STD_NULL) return;
    scope->provider(scope->context, milliseconds);
}
