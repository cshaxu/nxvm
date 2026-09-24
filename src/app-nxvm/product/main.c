/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */
#include "lib/types/types_interface.h"
#include <stdio.h>


#include "app-nxvm/product/version.h"

#include "banner.h"
#include "app-nxvm/product/composition.h"
#include "app-nxvm/product/command.h"
#include "app-nxvm/product/startup.h"

lib_i32 main(void)
{
    vm_app *session = LIB_NULL;
    vm_app_console_context *console_context = LIB_NULL;
    lib_u8 ini_path[1024];
    lib_status status;
    lib_status destroy_status;

    PRODUCT_PRINT_BANNER();
    if (vm_app_ini_executable_path(ini_path, sizeof(ini_path)) != LIB_STATUS_OK) {
        printf("Unable to determine NXVM.ini path.\n");
        return 1;
    }
    if (vm_app_create(&session) != LIB_STATUS_OK ||
        vm_app_console_context_create(&console_context) != LIB_STATUS_OK) {
        (void)vm_app_destroy(session);
        return 1;
    }
    status = vm_app_console_main(console_context, session, ini_path);
    vm_app_console_context_destroy(console_context);
    destroy_status = vm_app_destroy(session);
    return status == LIB_STATUS_OK && destroy_status == LIB_STATUS_OK ? 0 : 1;
}
