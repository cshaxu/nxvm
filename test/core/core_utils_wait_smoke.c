#include "type.h"




#include "core/utils/wait.h"
#include "core/utils/wait_provider.h"

static type_unsigned_32 observed_milliseconds;

static C_VOID core_utils_wait_smoke_provider(C_VOID *context, type_unsigned_32 milliseconds)
{
    type_unsigned_32 *calls = context;

    *calls += 1u;
    observed_milliseconds = milliseconds;
}

C_INT main(C_VOID)
{
    type_unsigned_32 calls = 0u;
    type_unsigned_32 nested_calls = 0u;
    core_utils_wait_scope scope;
    core_utils_wait_scope nested_scope;

    core_utils_wait_scope_initialize(&scope, core_utils_wait_smoke_provider,
        &calls);
    core_utils_wait_milliseconds(&scope, 17u);
    if (calls != 1u || observed_milliseconds != 17u) {
        return 1;
    }
    core_utils_wait_scope_initialize(&nested_scope,
        core_utils_wait_smoke_provider, &nested_calls);
    core_utils_wait_milliseconds(&nested_scope, 23u);
    if (calls != 1u || nested_calls != 1u || observed_milliseconds != 23u) {
        return 1;
    }
    core_utils_wait_milliseconds(&scope, 31u);
    if (calls != 2u || nested_calls != 1u || observed_milliseconds != 31u) {
        return 1;
    }
    core_utils_wait_milliseconds(STD_NULL, 1u);
    if (calls != 2u) {
        return 1;
    }
    puts("M5:T234:S2:CORE-UTILS-WAIT:OK");
    return 0;
}
