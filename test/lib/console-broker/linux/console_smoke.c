#include "lib/types/test.h"
#include "lib/console-broker/linux/console.c"
int main(void)
{
    console_broker_backend *backend = (void *)1;
    lib_test_assert(console_broker_backend_create(&backend) == LIB_STATUS_UNSUPPORTED && !backend);
    lib_test_assert(console_broker_backend_create(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(console_broker_backend_request_cooked_line(LIB_NULL) == LIB_STATUS_UNSUPPORTED);
    lib_bool completed = LIB_TRUE;
    lib_test_assert(console_broker_backend_cancel_cooked_line(backend, &completed) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(!completed);
    lib_test_assert(console_broker_backend_prepare(backend, LIB_NULL, 0) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(console_broker_backend_activate(backend, LIB_NULL, 0, 1u, LIB_FALSE) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(console_broker_backend_deactivate(backend, &completed) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(console_broker_backend_write_bound(backend, LIB_NULL, 1u, "test", 4u) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(console_broker_backend_write_text_frame_bound(backend, LIB_NULL, 1u, LIB_NULL) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(console_broker_backend_destroy(backend) == LIB_STATUS_OK);
    return 0;
}
