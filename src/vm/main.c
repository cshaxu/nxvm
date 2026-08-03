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

#include "version.h"

#include "vm/composition/console_target.h"

#include "vm/composition/session.h"

#include "vm/product/console.h"

C_INT main(C_INT argc, C_CHAR **argv) {
    C_CHAR banner[160];
    vm_session *session;

    (C_VOID)argc;
    (C_VOID)argv;
    if (!ntvdm64_version_format_nxvm_banner(banner, sizeof(banner),
        NXVM_BUILD_SUFFIX)) {
        return 1;
    }
    STD_PRINTF("%s\n", banner);
    STD_PRINTF("Built on %s at %s.\n", ntvdm64_version_build_date(),
        ntvdm64_version_build_time());
    if (vm_session_create(STD_NULL, &session) != NTVDM64_STATUS_OK) return 1;
    vm_session_console_target_initialize(session->console_target, session);
    vm_product_console_main(session->console_context, session->console_target);
    vm_session_destroy(session);
    return 0;
}
