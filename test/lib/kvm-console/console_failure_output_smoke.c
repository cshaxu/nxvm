#include "lib/types/test.h"
#include "lib/kvm-console/console_interface.h"
int main(void)
{
    kvm_console *console = (kvm_console *)1;

    lib_test_assert(kvm_console_create(&console, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT &&
        console == LIB_NULL);
    return 0;
}
