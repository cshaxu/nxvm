#include "lib/types/test.h"
#include "lib/kvm-window/window.c"
#include "lib/kvm-window/linux/component.c"

static lib_bool input(void *context, const kvm_input_event *event)
{ (void)context; (void)event; lib_test_assert(0); return LIB_FALSE; }
static void failure(void *context, lib_u64 source, lib_status status)
{ (void)context; (void)source; (void)status; lib_test_assert(0); }

int main(void)
{
    kvm_window_options options = { .initial_title = "fixture",
        .component = { .input_sink = input, .failure_sink = failure } };
    for (lib_u32 i = 0u; i < 2u; ++i) {
        kvm_window *window = (void *)&window;
        lib_test_assert(kvm_window_create(&window, &options) == LIB_STATUS_UNSUPPORTED);
        lib_test_assert(window == LIB_NULL);
        lib_test_assert(kvm_window_destroy(window) == LIB_STATUS_OK);
    }
    return 0;
}
