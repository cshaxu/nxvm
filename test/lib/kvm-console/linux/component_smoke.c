#include "lib/types/test.h"
#include "lib/kvm-console/console.c"
#include "lib/kvm-console/linux/component.c"

static lib_bool input(void *context, const kvm_input_event *event)
{ (void)context; (void)event; lib_test_assert(0); return LIB_FALSE; }
static void failure(void *context, lib_u64 source, lib_status status)
{ (void)context; (void)source; (void)status; lib_test_assert(0); }

int main(void)
{
    kvm_console_options options = { .input_sink = input, .failure_sink = failure };
    for (lib_u32 i = 0u; i < 2u; ++i) {
        kvm_console *console = (void *)&console;
        lib_test_assert(kvm_console_create(&console, &options) == LIB_STATUS_UNSUPPORTED);
        lib_test_assert(console == LIB_NULL);
        lib_test_assert(kvm_console_destroy(console) == LIB_STATUS_OK);
    }
    return 0;
}
