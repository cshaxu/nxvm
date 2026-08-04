#include "type.h"



#include "core/machine/cpu.h"
#include "core/machine/debug_interface.h"

#include "core/machine/memory.h"

#include "vm/composition/session/control.h"

C_INT main(C_VOID)
{
    vm_session session = {0};
    const C_UCHAR invalid_instruction[] = { 0x0fu, 0x0bu };
    C_INT failed = 0;

    vm_session_storage_initialize(&session);
    vm_session_control_initialize(session.control, &session);
    vm_session_control_reset(session.control);
    if (core_machine_memory_write_real_to(
            core_machine_debug_memory_borrow(session.core_machine), 0xf000u,
            0xfff0u, invalid_instruction, sizeof(invalid_instruction)) !=
        NTVDM64_STATUS_OK) failed = 1;
    vm_session_control_start(session.control);

    failed |= vm_session_control_is_running(session.control) != NTVDM64_TYPE_FALSE;
    failed |= core_machine_cpu_execution_consume_stop_request(
        core_machine_debug_cpu_execution_borrow(session.core_machine)) !=
        NTVDM64_TYPE_FALSE;
    vm_session_control_finalize(session.control, &session);
    vm_session_storage_finalize(&session);

    if (failed) return 1;
    puts("M5:T14:S3:CPU-STOP:OK");
    return 0;
}
