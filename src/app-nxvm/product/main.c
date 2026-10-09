/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */
#include "core/product/startup_interface.h"
#include "core/product/version.h"
#include "app-nxvm/product/binding_interface.h"

#define APP_PRODUCT_NAME "Neko's x86 Virtual Machine"
#define APP_PRODUCT_BANNER APP_PRODUCT_NAME " [" CORE_PRODUCT_VERSION "]\n" \
    CORE_PRODUCT_COPYRIGHT "\n\n"

lib_i32 main(void)
{
    vm_app_print_banner(APP_PRODUCT_BANNER);
    return vm_app_run(APP_PRODUCT_NAME, &vm_app_machine);
}
