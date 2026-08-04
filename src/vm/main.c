/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */

#include "type.h"

#include "vm/product/version.h"

#include "core/product/banner.h"
#include "core/product/session/session_interface.h"
#include "core/product/session/session_provider.h"

#include "vm/composition/session/provider.h"
#include "vm/product/console.h"

C_INT main(C_INT argc, C_CHAR **argv)
{
    core_product_session_provider session_provider;
    core_product_session_manager *session_manager = STD_NULL;
    vm_product_console_machine_provider machine_provider;
    vm_product_console_context console_context;

    (C_VOID) argc;
    (C_VOID) argv;
    CORE_PRODUCT_PRINT_BANNER();
    vm_session_provider_initialize(&session_provider);
    if (core_product_session_manager_create(&session_provider,
                                            &session_manager) != TYPE_STATUS_OK)
    {
        core_product_session_manager_destroy(session_manager);
        return 1;
    }
    vm_session_machine_provider_initialize(&machine_provider, session_manager);
    vm_product_console_main(&console_context, &machine_provider, session_manager);
    core_product_session_manager_destroy(session_manager);
    return 0;
}
