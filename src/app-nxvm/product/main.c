/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */

#include "type.h"

#include "app-nxvm/product/version.h"

#include "banner.h"
#include "app-nxvm/product/composition.h"
#include "app-nxvm/product/command.h"
#include "app-nxvm/product/startup.h"

C_INT main(C_VOID)
{
    vm_app *session = STD_NULL;
    vm_app_console_context *console_context = STD_NULL;
    C_CHAR ini_path[1024];
    type_status status;
    type_status destroy_status;

    PRODUCT_PRINT_BANNER();
    if (vm_app_ini_executable_path(ini_path, sizeof(ini_path)) != TYPE_STATUS_OK) {
        STD_PRINTF("Unable to determine NXVM.ini path.\n");
        return 1;
    }
    if (vm_app_create(&session) != TYPE_STATUS_OK ||
        vm_app_console_context_create(&console_context) != TYPE_STATUS_OK) {
        (C_VOID)vm_app_destroy(session);
        return 1;
    }
    status = vm_app_console_main(console_context, session, ini_path);
    vm_app_console_context_destroy(console_context);
    destroy_status = vm_app_destroy(session);
    return status == TYPE_STATUS_OK && destroy_status == TYPE_STATUS_OK ? 0 : 1;
}
