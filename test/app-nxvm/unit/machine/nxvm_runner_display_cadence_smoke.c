#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "core/machine/control.h"
#include "core/machine/display.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "x86/debug/protocol_interface.h"
#include "../../../core/machine/support/emulator_machine_fixture.h"
#include "../../support/rom/session_assets.h"

static void vm_runner_completed_state(void *context,
    emulator_machine_state state, lib_u32 run_generation)
{
    if (state != EMULATOR_MACHINE_RUNNING)
        vm_test_emulator_machine_state_changed(context, state, run_generation);
}

static lib_bool vm_runner_debug_execute(vm_machine *machine,
    const emulator_machine_debug_lease *lease, const x86_debug_request *request,
    x86_debug_response *response)
{
    lib_size bytes = 0u;

    return emulator_machine_debug_execute_with_lease(machine->executor, lease,
        request, sizeof(*request), response, sizeof(*response), &bytes) ==
        LIB_STATUS_OK && bytes == sizeof(*response);
}

static lib_bool vm_runner_step_and_pause(vm_machine *machine,
    const vm_test_emulator_machine_state_waiter *waiter)
{
    emulator_machine_debug_lease lease;
    x86_debug_response response;

    if (emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK ||
        !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_SET_EXECUTION_PLAN,
            .execution_kind = X86_DEBUG_EXECUTION_TRACE, .instruction_count = 4u
        }, &response) || base_sync_event_reset(waiter->event) != LIB_STATUS_OK)
        return LIB_FALSE;
    /* Expire only host presentation eligibility while the executor is paused;
     * the real runner must still decide whether any guest content changed. */
    machine->last_display_publish_milliseconds = 0u;
    if (vm_machine_resume(machine) != LIB_STATUS_OK ||
        base_sync_event_wait(waiter->event, 2000u) != BASE_SYNC_WAIT_SIGNALED ||
        emulator_machine_state_get(machine->executor) != EMULATOR_MACHINE_PAUSED ||
        emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK)
        return LIB_FALSE;
    return vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
        .operation = X86_DEBUG_GET_EXECUTION_RESULT
    }, &response) && response.enabled && response.value == 4u;
}

static lib_bool vm_runner_bound_delivery(vm_machine *machine,
    vm_test_emulator_machine_state_waiter *waiter)
{
    static const struct { lib_u16 port; lib_u8 value; } graphics[] = {
        {0x03c4u, 4u}, {0x03c5u, 6u},
        {0x03c4u, 2u}, {0x03c5u, 0x0fu},
        {0x03ceu, 5u}, {0x03cfu, 0u},
        {0x03ceu, 6u}, {0x03cfu, 0x05u}, {0x03c2u, 0x03u},
        {0x03c0u, 0x30u}, {0x03c0u, 0x01u},
        {0x03d4u, 1u}, {0x03d5u, 0x27u},
        {0x03d4u, 7u}, {0x03d5u, 0x10u},
        {0x03d4u, 0x12u}, {0x03d5u, 0xc7u},
        {0x03d4u, 0x13u}, {0x03d5u, 0x14u},
        {0x03d4u, 0x09u}, {0x03d5u, 0u},
        {0x03d4u, 0x17u}, {0x03d5u, 0xe3u},
        {0x03d4u, 0x18u}, {0x03d5u, 0xffu}
    };
    emulator_machine_debug_lease lease;
    x86_debug_response response;
    static emulator_machine_frame published;
    lib_u32 sequence;
    lib_u32 run_generation;

    emulator_machine_set_state_sink(machine->executor,
        vm_runner_completed_state, waiter);
    if (base_sync_event_reset(waiter->event) != LIB_STATUS_OK ||
        !emulator_machine_reset(machine->executor) ||
        base_sync_event_wait(waiter->event, 2000u) != BASE_SYNC_WAIT_SIGNALED ||
        emulator_machine_state_get(machine->executor) != EMULATOR_MACHINE_PAUSED)
        return LIB_FALSE;
    run_generation = emulator_machine_run_generation(machine->executor);
    if (!emulator_machine_copy_published_frame(machine->executor, &published,
            run_generation) || published.window.graphics ||
        !published.window.valid ||
        emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK ||
        !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_READ_PORT, .port = 0x03dau, .bytes = 1u
        }, &response)) return LIB_FALSE;
    sequence = published.sequence;
    for (lib_size index = 0u; index < sizeof(graphics) / sizeof(graphics[0]); ++index) {
        if (!vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
                .operation = X86_DEBUG_WRITE_PORT, .port = graphics[index].port,
                .bytes = 1u, .address = graphics[index].value
            }, &response)) return LIB_FALSE;
    }
    if (!vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_WRITE_LINEAR, .address = 0x500u,
            .bytes = 2u, .data = {0xebu, 0xfeu}
        }, &response) || !vm_runner_debug_execute(machine, &lease,
            &(x86_debug_request) {.operation = X86_DEBUG_WRITE_REGISTER,
                .register_id = X86_DEBUG_CS, .address = 0u}, &response) ||
        !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_WRITE_REGISTER,
            .register_id = X86_DEBUG_EIP, .address = 0x500u
        }, &response) || !vm_runner_debug_execute(machine, &lease,
            &(x86_debug_request) {.operation = X86_DEBUG_WRITE_LINEAR,
                .address = 0xa0000u, .bytes = 1u, .data = {0x80u}}, &response) ||
        !vm_runner_step_and_pause(machine, waiter) ||
        !emulator_machine_copy_published_frame(machine->executor, &published,
            run_generation) || !published.window.graphics ||
        published.window.image.width != 320u || published.window.image.height != 200u ||
        published.window.image.pixels[0] != 15u || published.sequence != sequence + 1u)
        return LIB_FALSE;
    sequence = published.sequence;
    if (!vm_runner_step_and_pause(machine, waiter) ||
        emulator_machine_published_frame_sequence(machine->executor) != sequence ||
        !emulator_machine_copy_published_frame(machine->executor, &published,
            run_generation) || !published.window.graphics || published.sequence != sequence)
        return LIB_FALSE;
    if (base_sync_event_reset(waiter->event) != LIB_STATUS_OK ||
        !emulator_machine_reset(machine->executor) ||
        base_sync_event_wait(waiter->event, 2000u) != BASE_SYNC_WAIT_SIGNALED ||
        emulator_machine_state_get(machine->executor) != EMULATOR_MACHINE_PAUSED ||
        emulator_machine_copy_published_frame(machine->executor, &published,
            run_generation) ||
        !emulator_machine_copy_published_frame(machine->executor, &published,
            emulator_machine_run_generation(machine->executor))) return LIB_FALSE;
    return published.window.valid && !published.window.graphics &&
        published.window.text.base.text_columns == 80u &&
        published.window.text.base.text_rows == 25u && published.sequence == sequence + 1u;
}

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    vm_test_emulator_machine_state_waiter waiter = {0};
    static emulator_machine_frame frame;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        vm_machine_set_speed(session, VM_MACHINE_SPEED_TURBO) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    if (vm_machine_publish_display(session, LIB_TRUE) != LIB_STATUS_OK ||
        vm_machine_copy_emulator_frame(session, &frame) != LIB_STATUS_OK ||
        frame.window.text.base.text_columns != 80u ||
        frame.window.text.base.text_rows != 25u ||
        vm_test_emulator_machine_bind(session) != LIB_STATUS_OK ||
        vm_test_emulator_machine_state_waiter_initialize(session, &waiter) !=
            LIB_STATUS_OK ||
        vm_machine_resume(session) != LIB_STATUS_OK ||
        !vm_test_emulator_machine_wait_state(session, &waiter,
            EMULATOR_MACHINE_RUNNING, 2000u) ||
        vm_machine_request_pause(session) != LIB_STATUS_OK ||
        !vm_test_emulator_machine_wait_state(session, &waiter,
            EMULATOR_MACHINE_PAUSED, 2000u) ||
        vm_machine_resume(session) != LIB_STATUS_OK ||
        !vm_test_emulator_machine_wait_state(session, &waiter,
            EMULATOR_MACHINE_RUNNING, 2000u) ||
        vm_machine_request_pause(session) != LIB_STATUS_OK ||
        !vm_test_emulator_machine_wait_state(session, &waiter,
            EMULATOR_MACHINE_PAUSED, 2000u)) {
        failed = 1;
        goto done;
    }
    failed |= !vm_runner_bound_delivery(session, &waiter);
done:
    vm_machine_stop(session);
    failed |= !vm_test_emulator_machine_wait_state(session, &waiter,
        EMULATOR_MACHINE_STOPPED, 2000u);
    vm_test_emulator_machine_unbind(session);
    vm_test_emulator_machine_state_waiter_finalize(&waiter);
    vm_machine_destroy(session);
    if (failed) return 1;
    puts("RUNNER-CADENCE:OK");
    puts("COMPOSITION-RUNNER-CADENCE:OK");
    return 0;
}
