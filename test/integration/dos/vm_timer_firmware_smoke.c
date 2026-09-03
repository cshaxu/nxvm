#include "type.h"

#include <windows.h>

#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "../../core/support/core_machine_cpu_fixture.h"
#include "test/integration/support/session_yaml.h"

#define VM_TIMER_BDA_TICKS 0x046cu
#define VM_TIMER_BDA_ROLLOVER 0x0470u
#define VM_TIMER_DAILY_LIMIT 0x001800b0u

static DWORD WINAPI vm_timer_run(C_VOID *opaque)
{
    vm_session_start((vm_session *)opaque);
    return 0u;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    integration_yaml_session yaml_session;
    vm_session *session = STD_NULL;
    HANDLE thread = STD_NULL;
    DWORD elapsed;
    type_unsigned_32 bda_ticks = 0u;
    type_unsigned_32 int1a_ticks;
    type_unsigned_64 paused_elapsed_ticks;
    type_unsigned_64 observed_paused_elapsed_ticks;
    type_unsigned_64 stepped_elapsed_ticks;
    type_unsigned_32 rollover_seed = VM_TIMER_DAILY_LIMIT - 1u;
    type_unsigned_8 rollover_byte = 0u;
    core_machine_run_budget budget = { 512u, 0u };
    core_machine_run_result result = {0};
    t_cpu cpu;
    C_INT stage = 0;
    static const type_unsigned_8 int1a_program[] = { 0xb4u, 0x00u, 0xcdu, 0x1au, 0xf4u };
    static const type_unsigned_8 rollover_program[] = {
        0xcdu, 0x08u, 0xb4u, 0x00u, 0xcdu, 0x1au, 0xf4u
    };

    stage = 1;
    if (argc != 3) goto fail;
    if (integration_yaml_session_open(argv[1], argv[2], &yaml_session) != TYPE_STATUS_OK) {
        return 77;
    }
    session = yaml_session.session;
    stage = 2;
    thread = CreateThread(STD_NULL, 0u, vm_timer_run, session, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    stage = 3;
    /* Host time is only a bounded startup watchdog; guest time remains core-owned. */
    for (elapsed = 0u; elapsed < 10000u; elapsed += 10u) {
        Sleep(10u);
        if (core_machine_debug_read_memory(session->core_machine,
                VM_TIMER_BDA_TICKS, &bda_ticks, sizeof(bda_ticks)) ==
                TYPE_STATUS_OK && bda_ticks != 0u) break;
    }
    if (bda_ticks == 0u) goto fail;
    stage = 4;
    vm_session_control_request_pause(&session->control, VM_SESSION_PAUSE_EXPLICIT);
    if (!vm_session_control_wait_for_pause(&session->control, 2000u)) goto fail;
    stage = 5;
    if (core_machine_get_elapsed_ticks(session->core_machine,
            &paused_elapsed_ticks) != TYPE_STATUS_OK) goto fail;
    /* This is a host-side watchdog observation, never a guest clock source. */
    Sleep(25u);
    if (core_machine_get_elapsed_ticks(session->core_machine,
            &observed_paused_elapsed_ticks) != TYPE_STATUS_OK ||
        observed_paused_elapsed_ticks != paused_elapsed_ticks) goto fail;
    if (!vm_session_control_step(&session->control) ||
        !vm_session_control_wait_for_pause(&session->control, 2000u) ||
        vm_session_control_get_pause_reason(&session->control) !=
            VM_SESSION_PAUSE_STEP ||
        core_machine_get_elapsed_ticks(session->core_machine,
            &stepped_elapsed_ticks) != TYPE_STATUS_OK ||
        stepped_elapsed_ticks <= paused_elapsed_ticks) goto fail;
    stage = 6;
    if (!test_core_machine_fixture_prepare_real_mode_execution(
            session->core_machine, 0x1000u)) {
        goto fail;
    }
    stage = 7;
    if (core_machine_memory_write(session->core_machine, 0x1000u, int1a_program,
            sizeof(int1a_program)) != TYPE_STATUS_OK ||
        core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) goto fail;
    stage = 8;
    cpu = test_core_machine_fixture_capture_cpu_after_run(session->core_machine);
    int1a_ticks = ((type_unsigned_32)cpu.data.cx << 16) | cpu.data.dx;
    if (int1a_ticks != bda_ticks || cpu.data.al != 0u) goto fail;
    stage = 9;
    if (!test_core_machine_fixture_prepare_real_mode_execution(
            session->core_machine, 0x1100u)) goto fail;
    if (core_machine_memory_write(session->core_machine, VM_TIMER_BDA_TICKS,
            &rollover_seed, sizeof(rollover_seed)) != TYPE_STATUS_OK ||
        core_machine_memory_write(session->core_machine, VM_TIMER_BDA_ROLLOVER,
            &rollover_byte, sizeof(rollover_byte)) != TYPE_STATUS_OK ||
        core_machine_memory_write(session->core_machine, 0x1100u,
            rollover_program, sizeof(rollover_program)) != TYPE_STATUS_OK ||
        core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) goto fail;
    stage = 10;
    cpu = test_core_machine_fixture_capture_cpu_after_run(session->core_machine);
    int1a_ticks = ((type_unsigned_32)cpu.data.cx << 16) | cpu.data.dx;
    if (int1a_ticks != 0u || cpu.data.al != 1u ||
        core_machine_debug_read_memory(session->core_machine,
            VM_TIMER_BDA_ROLLOVER, &rollover_byte, sizeof(rollover_byte)) !=
            TYPE_STATUS_OK || rollover_byte != 0u) goto fail;
    vm_session_stop(session);
    if (WaitForSingleObject(thread, 2000u) != WAIT_OBJECT_0) goto fail;
    CloseHandle(thread);
    integration_yaml_session_close(&yaml_session);
    STD_PRINTF("M5:T225:S4:IRQ0-BDA-INT1A-ROLLOVER:DOS:OK\n");
    return 0;

fail:
    STD_FPRINTF(STD_STDERR, "M5:T225:S3:TIMER:FAIL:%d:%d:%u\n", stage,
        (C_INT)result.reason, bda_ticks);
    if (session != STD_NULL) vm_session_stop(session);
    if (thread != STD_NULL) {
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
    }
    integration_yaml_session_close(&yaml_session);
    return 1;
}
