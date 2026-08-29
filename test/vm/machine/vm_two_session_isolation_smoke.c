#include "type.h"
#include "vm/composition/session/session_private.h"




#include "../../core/support/core_machine_cpu_fixture.h"

#include "vm/composition/session/session_interface.h"

C_INT main(C_VOID)
{
    vm_session *first;
    vm_session *second;
    C_INT failed = 0;

    first = ((vm_session *)STD_CALLOC(1u, sizeof(vm_session)));
    second = ((vm_session *)STD_CALLOC(1u, sizeof(vm_session)));
    if (first == STD_NULL || second == STD_NULL) {
        STD_FREE(second);
        STD_FREE(first);
        return 1;
    }

    vm_session_storage_initialize(first);
    vm_session_storage_initialize(second);

    vm_machine_debug_initialize(&first->debug);
    vm_machine_debug_initialize(&second->debug);
    vm_machine_debug_set_trace(&first->debug, 2u);

    failed |= first->fdc_dma_request.core_token == second->fdc_dma_request.core_token;
    failed |= !test_core_machine_fixture_sessions_are_isolated(
        first->core_machine, second->core_machine);
    failed |= !first->debug.data.flagTrace || second->debug.data.flagTrace;

    vm_session_storage_finalize(second);
    vm_session_storage_finalize(first);
    STD_FREE(second);
    STD_FREE(first);

    if (failed) return 1;
    puts("M5:T73:S1:TWO-SESSION-ISOLATION:OK");
    return 0;
}
