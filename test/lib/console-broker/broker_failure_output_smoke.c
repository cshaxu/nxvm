#include "lib/types/test.h"
#include "lib/console-broker/console_interface.h"
int main(void)
{
    console_broker *broker = (console_broker *)1;

    lib_test_assert(console_broker_create(&broker, LIB_NULL, CONSOLE_BROKER_RAW_EVENTS) ==
        LIB_STATUS_INVALID_ARGUMENT && broker == LIB_NULL);
    return 0;
}
