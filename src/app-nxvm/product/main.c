/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */
#include "core/product/startup_interface.h"
#include "core/product/version_interface.h"
#include "app-nxvm/product/binding_interface.h"

lib_i32 main(void)
{
    return vm_app_run(NXVM_PRODUCT_NAME, &vm_app_machine);
}
