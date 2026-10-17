#include "lib/types/test.h"
#include "lib/kvm-window/window_interface.h"
int main(void)
{
    kvm_window *window = (kvm_window *)1;
    lib_test_assert(kvm_window_create(&window, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT &&
        window == LIB_NULL);
    return 0;
}
