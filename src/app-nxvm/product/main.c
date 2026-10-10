/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */
#include "core/product/startup_interface.h"
#include "app-nxvm/product/binding_interface.h"

#define APP_PRODUCT_NAME "Neko's x86 Virtual Machine"

lib_i32 main(void)
{
    return vm_app_run(APP_PRODUCT_NAME, &vm_app_machine);
}
