#ifndef NXVM_BUILD_SUFFIX
#define NXVM_BUILD_SUFFIX "m1t2s1"
#endif

/*
 * Author:     Xu Ha
 * Email:      cshaxu@gmail.com
 * Repository: https://github.com/cshaxu/nxvm
 * Start:      01/25/2012
 */

#include "type.h"



#include "core/product/utils.h"
#include "core/product/session/session_interface.h"
#include "core/product/session/session_provider.h"

#include "version.h"

#include "vm/composition/session/provider.h"

#include "vm/product/console.h"

C_INT main(C_INT argc, C_CHAR **argv) {
    C_CHAR banner[160];
    core_product_session_provider session_provider;
    core_product_session_manager *session_manager = STD_NULL;
    vm_product_console_machine_provider machine_provider;
    nxvm_product_console_context console_context;

    (C_VOID)argc;
    (C_VOID)argv;
    if (!ntvdm64_version_format_nxvm_banner(banner, sizeof(banner),
        NXVM_BUILD_SUFFIX)) {
        return 1;
    }
    STD_PRINTF("%s\n", banner);
    STD_PRINTF("Built on %s at %s.\n", ntvdm64_version_build_date(),
        ntvdm64_version_build_time());
    vm_session_provider_initialize(&session_provider);
    if (core_product_session_manager_create(&session_provider,
            &session_manager) != NTVDM64_STATUS_OK) {
        core_product_session_manager_destroy(session_manager);
        return 1;
    }
    vm_session_machine_provider_initialize(&machine_provider, session_manager);
    vm_product_console_main(&console_context, &machine_provider, session_manager);
    core_product_session_manager_destroy(session_manager);
    return 0;
}
