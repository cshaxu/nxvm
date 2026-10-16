/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */
#include "x86-nxvm/product/startup_interface.h"
#include "app-nxvm/product_binding_interface.h"

#define APP_PRODUCT_NAME "Neko's x86 Virtual Machine"

lib_i32 main(void)
{
    return x86_nxvm_product_run(APP_PRODUCT_NAME, APP_PRODUCT_INI_NAME, &vm_app_machine);
}
