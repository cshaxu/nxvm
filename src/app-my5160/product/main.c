#include "core/product/startup_interface.h"
#include "core/product/version_interface.h"
#include "app-my5160/product/binding_interface.h"

lib_i32 main(void)
{
    return vm_app_run(NXVM_PRODUCT_NAME, &vm_app_machine);
}
