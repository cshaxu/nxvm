#include "type.h"

#include <windows.h>

#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "tests/support/vm_session_fixture.h"
#include "vm/machine/fdd.h"

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
    vm_session *session = STD_NULL;
    HANDLE thread = STD_NULL;
    DWORD elapsed;
    uint32_t bda_ticks = 0u;
    uint32_t int1a_ticks;
    uint64_t paused_elapsed_ticks;
    uint64_t observed_paused_elapsed_ticks;
    uint64_t stepped_elapsed_ticks;
    uint32_t rollover_seed = VM_TIMER_DAILY_LIMIT - 1u;
    uint8_t rollover_byte = 0u;
    core_machine_cpu_execution_context *execution;
    core_machine_run_budget budget = { 512u, 0u };
    core_machine_run_result result;
    t_cpu *cpu;
    C_INT stage = 0;
    static const uint8_t int1a_program[] = { 0xb4u, 0x00u, 0xcdu, 0x1au, 0xf4u };
    static const uint8_t rollover_program[] = {
        0xcdu, 0x08u, 0xb4u, 0x00u, 0xcdu, 0x1au, 0xf4u
    };

    stage = 1;
    if (argc != 2 || vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_machine_fdd_insert_for(vm_session_fixture_fdd(session), argv[1]) != 0) goto fail;
    stage = 2;
    thread = CreateThread(STD_NULL, 0u, vm_timer_run, session, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    stage = 3;
    for (elapsed = 0u; elapsed < 3000u; elapsed += 10u) {
        Sleep(10u);
        if (core_machine_debug_read_memory(vm_session_fixture_machine(session),
                VM_TIMER_BDA_TICKS, &bda_ticks, sizeof(bda_ticks)) ==
                TYPE_STATUS_OK && bda_ticks != 0u) break;
    }
    if (bda_ticks == 0u) goto fail;
    stage = 4;
    vm_session_control_request_pause(vm_session_fixture_control(session), VM_SESSION_PAUSE_EXPLICIT);
    if (!vm_session_control_wait_for_pause(vm_session_fixture_control(session), 2000u)) goto fail;
    stage = 5;
    if (core_machine_get_elapsed_ticks(vm_session_fixture_machine(session),
            &paused_elapsed_ticks) != TYPE_STATUS_OK) goto fail;
    /* This is a host-side watchdog observation, never a guest clock source. */
    Sleep(25u);
    if (core_machine_get_elapsed_ticks(vm_session_fixture_machine(session),
            &observed_paused_elapsed_ticks) != TYPE_STATUS_OK ||
        observed_paused_elapsed_ticks != paused_elapsed_ticks) goto fail;
    if (!vm_session_control_step(vm_session_fixture_control(session)) ||
        !vm_session_control_wait_for_pause(vm_session_fixture_control(session), 2000u) ||
        vm_session_control_get_pause_reason(vm_session_fixture_control(session)) !=
            VM_SESSION_PAUSE_STEP ||
        core_machine_get_elapsed_ticks(vm_session_fixture_machine(session),
            &stepped_elapsed_ticks) != TYPE_STATUS_OK ||
        stepped_elapsed_ticks != paused_elapsed_ticks + 1u) goto fail;
    stage = 6;
    cpu = core_machine_debug_cpu_borrow(vm_session_fixture_machine(session));
    execution = core_machine_debug_cpu_execution_borrow(vm_session_fixture_machine(session));
    if (cpu == STD_NULL || execution == STD_NULL ||
        core_machine_cpu_execution_load_segment(execution, &cpu->data.cs, 0u) ||
        core_machine_cpu_execution_load_segment(execution, &cpu->data.ds, 0u)) {
        goto fail;
    }
    stage = 7;
    cpu->data.eip = 0x1000u;
    cpu->data.flagHalt = TYPE_FALSE;
    if (core_machine_memory_write(vm_session_fixture_machine(session), 0x1000u, int1a_program,
            sizeof(int1a_program)) != TYPE_STATUS_OK ||
        core_machine_run(vm_session_fixture_machine(session), budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) goto fail;
    stage = 8;
    int1a_ticks = ((uint32_t)cpu->data.cx << 16) | cpu->data.dx;
    if (int1a_ticks != bda_ticks || cpu->data.al != 0u) goto fail;
    stage = 9;
    cpu->data.eip = 0x1100u;
    cpu->data.flagHalt = TYPE_FALSE;
    if (core_machine_memory_write(vm_session_fixture_machine(session), VM_TIMER_BDA_TICKS,
            &rollover_seed, sizeof(rollover_seed)) != TYPE_STATUS_OK ||
        core_machine_memory_write(vm_session_fixture_machine(session), VM_TIMER_BDA_ROLLOVER,
            &rollover_byte, sizeof(rollover_byte)) != TYPE_STATUS_OK ||
        core_machine_memory_write(vm_session_fixture_machine(session), 0x1100u,
            rollover_program, sizeof(rollover_program)) != TYPE_STATUS_OK ||
        core_machine_run(vm_session_fixture_machine(session), budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) goto fail;
    stage = 10;
    int1a_ticks = ((uint32_t)cpu->data.cx << 16) | cpu->data.dx;
    if (int1a_ticks != 0u || cpu->data.al != 1u ||
        core_machine_debug_read_memory(vm_session_fixture_machine(session),
            VM_TIMER_BDA_ROLLOVER, &rollover_byte, sizeof(rollover_byte)) !=
            TYPE_STATUS_OK || rollover_byte != 0u) goto fail;
    vm_session_stop(session);
    if (WaitForSingleObject(thread, 2000u) != WAIT_OBJECT_0) goto fail;
    CloseHandle(thread);
    vm_session_destroy(session);
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
    vm_session_destroy(session);
    return 1;
}
