#include "type.h"



#include "core/machine/cpu.h"
#include "core/machine/debug_interface.h"

#include "core/machine/memory.h"

#include "vm/composition/session/control.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    const C_UCHAR invalid_instruction[] = { 0x0fu, 0x0bu };
    C_INT failed = 0;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    vm_session_control_reset(&session->control);
    if (core_machine_memory_write_real_to(
            core_machine_debug_memory_borrow(session->core_machine), 0xf000u,
            0xfff0u, invalid_instruction, sizeof(invalid_instruction)) !=
        TYPE_STATUS_OK) failed = 1;
    vm_session_control_start(&session->control);

    failed |= vm_session_control_is_running(&session->control) != TYPE_FALSE;
    failed |= core_machine_cpu_execution_consume_stop_request(
        core_machine_debug_cpu_execution_borrow(session->core_machine)) !=
        TYPE_FALSE;
    vm_session_destroy(session);

    if (failed) return 1;
    puts("M5:T14:S3:CPU-STOP:OK");
    return 0;
}
