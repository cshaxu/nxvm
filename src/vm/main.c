/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */

#include "type.h"

#include "vm/product/version.h"

#include "banner.h"
#include "vm/app/app.h"
#include "vm/product/console.h"

static const C_CHAR *vm_main_profile_directory(C_INT argc, C_CHAR **argv,
    C_CHAR *directory, STD_SIZE_T capacity)
{
    C_CHAR *cursor;
    STD_SIZE_T length;

    if (directory == STD_NULL || capacity < 2u || argc <= 0 || argv == STD_NULL ||
        argv[0] == STD_NULL) return ".";
    length = STD_STRLEN(argv[0]);
    if (length >= capacity) return ".";
    STD_MEMCPY(directory, argv[0], length + 1u);
    cursor = directory + length;
    while (cursor != directory && cursor[-1] != '/' && cursor[-1] != '\\') --cursor;
    if (cursor == directory) return ".";
    cursor[-1] = '\0';
    return directory;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    vm_app *session = STD_NULL;
    vm_product_console_context *console_context = STD_NULL;
    C_CHAR profile_directory[1024];

    PRODUCT_PRINT_BANNER();
    if (vm_app_create(&session) != TYPE_STATUS_OK ||
        vm_product_console_context_create(&console_context) != TYPE_STATUS_OK) {
        vm_app_destroy(session);
        return 1;
    }
    vm_product_console_main(console_context, session,
        vm_main_profile_directory(argc, argv, profile_directory,
            sizeof(profile_directory)));
    vm_product_console_context_destroy(console_context);
    vm_app_destroy(session);
    return 0;
}
