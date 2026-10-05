/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */
#include "ibmpc/product/entry_interface.h"
#include "ibmpc/product/version_interface.h"
#include "ibmpc/product/factory_interface.h"
#include "app-nxvm/product/profile_binding.h"

lib_i32 main(void)
{
    vm_app_definition definition = {
        .name = PRODUCT_NAME, .version = PRODUCT_BUILD_VERSION,
        .copyright = PRODUCT_COPYRIGHT, .build_time = __DATE__ " " __TIME__
    };

    vm_app_configure_factory(&vm_app_machine, &definition.factory);
    return vm_app_run(&definition);
}
