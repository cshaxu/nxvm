#include "lib/types/types_interface.h"
#include "app-nxvm/product/config.h"

lib_i32 main(void)
{
    vm_machine_config config;
    vm_machine_config cleared = {0};

    lib_memory_set(&config, 0xff, sizeof(config));
    if (vm_app_configure_machine(LIB_NULL, &config) != LIB_STATUS_INVALID_ARGUMENT ||
        lib_memory_compare(&config, &cleared, sizeof(config))) return 1;
    return 0;
}
