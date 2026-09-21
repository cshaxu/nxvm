/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */

#include "type.h"

#include "app/version.h"

#include "banner.h"
#include "app/composition.h"
#include "app/command.h"

static const C_CHAR *vm_main_ini_path(C_INT argc, C_CHAR **argv,
    C_CHAR *path, STD_SIZE_T capacity)
{
    C_CHAR *cursor;
    STD_SIZE_T length;

    if (path == STD_NULL || capacity < sizeof("NXVM.ini") || argc <= 0 || argv == STD_NULL ||
        argv[0] == STD_NULL) return "NXVM.ini";
    length = STD_STRLEN(argv[0]);
    if (length + sizeof("NXVM.ini") >= capacity) return "NXVM.ini";
    STD_MEMCPY(path, argv[0], length + 1u);
    cursor = path + length;
    while (cursor != path && cursor[-1] != '/' && cursor[-1] != '\\') --cursor;
    if (cursor == path) return "NXVM.ini";
    STD_MEMCPY(cursor, "NXVM.ini", sizeof("NXVM.ini"));
    return path;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    vm_app *session = STD_NULL;
    vm_app_console_context *console_context = STD_NULL;
    C_CHAR ini_path[1024];

    PRODUCT_PRINT_BANNER();
    if (vm_app_create(&session) != TYPE_STATUS_OK ||
        vm_app_console_context_create(&console_context) != TYPE_STATUS_OK) {
        vm_app_destroy(session);
        return 1;
    }
    vm_app_console_main(console_context, session,
        vm_main_ini_path(argc, argv, ini_path, sizeof(ini_path)));
    vm_app_console_context_destroy(console_context);
    vm_app_destroy(session);
    return 0;
}
