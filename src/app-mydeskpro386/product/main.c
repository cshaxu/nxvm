#include "ibmpc/product/entry_interface.h"
#include "ibmpc/product/composition.h"
#include "ibmpc/nxvm/factory_interface.h"
#include "ibmpc/nxvm/startup_interface.h"
#include "ibmpc/product/version_interface.h"
#include "app-mydeskpro386/product/binding_interface.h"

lib_i32 main(void)
{
    vm_app_definition definition = {
        .name = PRODUCT_NAME, .version = PRODUCT_BUILD_VERSION,
        .copyright = PRODUCT_COPYRIGHT, .build_time = __DATE__ " " __TIME__,
        .configuration_file = "NXVM.ini", .load_request = vm_app_ini_load_request
    };

    vm_app_configure_factory(&vm_app_machine, &definition.factory);
    definition.configure_extensions = vm_app_configure_standard_extensions;
    return vm_app_run(&definition);
}
