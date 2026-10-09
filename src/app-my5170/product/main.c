#include "core/product/startup_interface.h"
#include "core/product/version.h"
#include "app-my5170/product/binding_interface.h"

#define APP_PRODUCT_NAME "My5170"
#define APP_PRODUCT_BANNER APP_PRODUCT_NAME " [" CORE_PRODUCT_VERSION "]\n" \
    CORE_PRODUCT_COPYRIGHT "\n\n"

lib_i32 main(void)
{
    vm_app_print_banner(APP_PRODUCT_BANNER);
    return vm_app_run(APP_PRODUCT_NAME, &vm_app_machine);
}
