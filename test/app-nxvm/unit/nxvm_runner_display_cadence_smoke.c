#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/machine/control.h"
#include "x86-nxvm/machine/display.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-debug/product/protocol_interface.h"
#include "../../x86-nxvm/machine/support/emulator_machine_fixture.h"
#include "../support/rom/session_assets.h"

static void vm_runner_completed_state(void *context,
    emulator_machine_state state, lib_u32 run_generation)
{
    if (state != EMULATOR_MACHINE_RUNNING)
        vm_test_emulator_machine_state_changed(context, state, run_generation);
}

/* Every caller has confirmed PAUSED. Check the original guest index as well
 * as its resolved publication colour; the fixture programs a nonstandard palette. */
static lib_bool vm_runner_pixel_equals(const x86_nxvm_machine *machine,
    const kvm_window_frame_storage *frame, lib_size pixel, lib_u8 colour)
{
    return machine->latest_snapshot.pixels[pixel] == colour &&
        frame->data.graphics->pixels[pixel] == machine->latest_snapshot.palette_rgb[colour];
}

static lib_bool vm_runner_debug_execute(x86_nxvm_machine *machine,
    const emulator_machine_debug_lease *lease, const x86_debug_request *request,
    x86_debug_response *response)
{
    lib_size bytes = 0u;

    return emulator_machine_debug_execute_with_lease(machine->executor, lease,
        request, sizeof(*request), response, sizeof(*response), &bytes) ==
        LIB_STATUS_OK && bytes == sizeof(*response);
}

typedef struct vm_runner_port_write {
    lib_u16 port;
    lib_u8 value;
} vm_runner_port_write;

static lib_bool vm_runner_program_ports(x86_nxvm_machine *machine,
    const emulator_machine_debug_lease *lease,
    const vm_runner_port_write *ports, lib_size count)
{
    x86_debug_response response;

    for (lib_size index = 0u; index < count; ++index)
        if (!vm_runner_debug_execute(machine, lease, &(x86_debug_request) {
                .operation = X86_DEBUG_WRITE_PORT, .port = ports[index].port,
                .bytes = 1u, .address = ports[index].value
            }, &response)) return LIB_FALSE;
    return LIB_TRUE;
}

static lib_bool vm_runner_step_and_pause(x86_nxvm_machine *machine,
    const vm_test_emulator_machine_state_waiter *waiter, lib_u32 instructions)
{
    emulator_machine_debug_lease lease;
    x86_debug_response response;

    if (emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK ||
        !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_SET_EXECUTION_PLAN,
            .execution_kind = X86_DEBUG_EXECUTION_TRACE, .instruction_count = instructions
        }, &response) || base_sync_event_reset(waiter->event) != LIB_STATUS_OK)
        return LIB_FALSE;
    /* Expire only host presentation eligibility while the executor is paused;
     * the real runner must still decide whether any guest content changed. */
    machine->last_display_publish_milliseconds = 0u;
    if (x86_nxvm_machine_resume(machine) != LIB_STATUS_OK ||
        base_sync_event_wait(waiter->event, 2000u) != BASE_SYNC_WAIT_SIGNALED ||
        emulator_machine_state_get(machine->executor) != EMULATOR_MACHINE_PAUSED ||
        emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK)
        return LIB_FALSE;
    return vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
        .operation = X86_DEBUG_GET_EXECUTION_RESULT
    }, &response) && response.enabled && response.value == instructions;
}

static lib_bool vm_runner_blink_cycle(x86_nxvm_machine *machine,
    const vm_test_emulator_machine_state_waiter *waiter,
    kvm_window_frame_storage *frame, lib_u32 run_generation, lib_bool character)
{
    static const vm_runner_port_write ports[] = {
        {0x03c4u, 3u}, {0x03c5u, 0u}, {0x03c4u, 2u}, {0x03c5u, 1u},
        {0x03d4u, 0u}, {0x03d5u, 6u}, {0x03d4u, 1u}, {0x03d5u, 0u},
        {0x03d4u, 6u}, {0x03d5u, 6u}, {0x03d4u, 7u}, {0x03d5u, 0x10u},
        {0x03d4u, 9u}, {0x03d5u, 1u}, {0x03d4u, 0x11u}, {0x03d5u, 0u},
        {0x03d4u, 0x12u}, {0x03d5u, 1u}, {0x03d4u, 0x13u}, {0x03d5u, 1u},
        {0x03d4u, 0x14u}, {0x03d5u, 31u}, {0x03d4u, 0x15u}, {0x03d5u, 3u},
        {0x03d4u, 0x0bu}, {0x03d5u, 1u}, {0x03d4u, 0x0eu}, {0x03d5u, 0u},
        {0x03d4u, 0x0fu}, {0x03d5u, 0u}
    };
    emulator_machine_debug_lease lease;
    x86_debug_response response;
    lib_bool hidden = LIB_FALSE;
    lib_bool visible_again = LIB_FALSE;
    lib_u32 sequence;

    if (emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK ||
        !vm_runner_program_ports(machine, &lease, ports, sizeof(ports) / sizeof(ports[0])) ||
        !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_WRITE_LINEAR, .address = 0xa0000u,
            .bytes = 1u, .data = {'A'}
        }, &response) || !vm_runner_debug_execute(machine, &lease,
            &(x86_debug_request) {.operation = X86_DEBUG_READ_PORT,
                .port = 0x03dau, .bytes = 1u}, &response) ||
        !vm_runner_program_ports(machine, &lease, (const vm_runner_port_write[]) {
            {0x03d4u, 0x0au}, {0x03d5u, character ? 31u : 0u},
            {0x03c0u, 0x30u}, {0x03c0u, character ? 8u : 0u},
            {0x03c4u, 2u}, {0x03c5u, 2u}
        }, 6u) || !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_WRITE_LINEAR, .address = 0xa0000u,
            .bytes = 1u, .data = {character ? 0x92u : 2u}
        }, &response) || !vm_runner_step_and_pause(machine, waiter, 4u) ||
        !emulator_machine_copy_published_frame(machine->executor, frame, run_generation))
        return LIB_FALSE;
    if (character ? !frame->frame.graphics || frame->data.graphics->width != 8u ||
            frame->data.graphics->height != 2u || !vm_runner_pixel_equals(machine, frame, 0, 2u) :
        frame->frame.graphics || !frame->data.text->base.cursor_phase)
        return LIB_FALSE;
    sequence = frame->frame.sequence;
    /* Only real execution may produce the next named blink transition. */
    for (lib_u16 segment = 0u; segment < 128u && !visible_again; ++segment) {
        lib_bool visible;

        if (!emulator_machine_copy_published_frame(machine->executor, frame, run_generation) ||
            frame->frame.sequence != sequence || !vm_runner_step_and_pause(machine, waiter, 16u) ||
            !emulator_machine_copy_published_frame(machine->executor, frame, run_generation))
            return LIB_FALSE;
        if (character ? !frame->frame.graphics || frame->data.graphics->width != 8u ||
                frame->data.graphics->height != 2u || !vm_runner_pixel_equals(machine, frame, 1, 1u) :
            frame->frame.graphics || frame->data.text->base.text_columns != 1u ||
                frame->data.text->base.text_rows != 1u) return LIB_FALSE;
        visible = character ? vm_runner_pixel_equals(machine, frame, 0, 2u) :
            frame->data.text->base.cursor_phase;
        if (character && !visible && !vm_runner_pixel_equals(machine, frame, 0, 1u)) return LIB_FALSE;
        if (!visible || hidden) {
            if ((!hidden || visible) && frame->frame.sequence == sequence) return LIB_FALSE;
            if (!visible) hidden = LIB_TRUE;
            else visible_again = LIB_TRUE;
        }
        sequence = frame->frame.sequence;
    }
    if (!hidden || !visible_again)
        lib_c_fprintf(lib_c_stderr, "RUNNER:BLINK-CYCLE:FAIL character=%u hidden=%u\n",
            character, hidden);
    return hidden && visible_again;
}

static lib_bool vm_runner_bound_delivery(x86_nxvm_machine *machine,
    vm_test_emulator_machine_state_waiter *waiter)
{
    static const vm_runner_port_write graphics[] = {
        {0x03c4u, 0u}, {0x03c5u, 1u}, {0x03c4u, 1u}, {0x03c5u, 1u},
        {0x03c4u, 0u}, {0x03c5u, 3u},
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
    static const vm_runner_port_write text[] = {
        {0x03d8u, 0x0du}, {0x03c2u, 3u},
        {0x03c4u, 4u}, {0x03c5u, 7u},
        {0x03c4u, 3u}, {0x03c5u, 4u},
        {0x03c4u, 2u}, {0x03c5u, 4u},
        {0x03ceu, 5u}, {0x03cfu, 0u},
        {0x03ceu, 6u}, {0x03cfu, 4u},
        {0x03c0u, 0x30u}, {0x03c0u, 0u},
        {0x03d4u, 1u}, {0x03d5u, 79u},
        {0x03d4u, 7u}, {0x03d5u, 0x12u},
        {0x03d4u, 9u}, {0x03d5u, 7u},
        {0x03d4u, 0x12u}, {0x03d5u, 0x8fu},
        {0x03d4u, 0x13u}, {0x03d5u, 40u},
        {0x03d4u, 0x17u}, {0x03d5u, 0xe3u},
        {0x03d4u, 0x18u}, {0x03d5u, 0xffu},
        {0x03d4u, 0x0au}, {0x03d5u, 6u},
        {0x03d4u, 0x0bu}, {0x03d5u, 7u},
        {0x03d4u, 0x0eu}, {0x03d5u, 0x0fu},
        {0x03d4u, 0x0fu}, {0x03d5u, 0x52u}
    };
    emulator_machine_debug_lease lease;
    x86_debug_response response;
    static kvm_window_frame_storage published;
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
            run_generation) || published.frame.graphics ||
        published.frame.text == LIB_NULL ||
        emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK ||
        !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_READ_PORT, .port = 0x03dau, .bytes = 1u
        }, &response)) return LIB_FALSE;
    sequence = published.frame.sequence;
    if (!vm_runner_program_ports(machine, &lease, graphics,
            sizeof(graphics) / sizeof(graphics[0]))) return LIB_FALSE;
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
!vm_runner_step_and_pause(machine, waiter, 4u) ||
        !emulator_machine_copy_published_frame(machine->executor, &published,
            run_generation) || !published.frame.graphics ||
        published.data.graphics->width != 320u || published.data.graphics->height != 200u ||
        !vm_runner_pixel_equals(machine, &published, 0, 15u) || published.frame.sequence != sequence + 1u)
        return LIB_FALSE;
    sequence = published.frame.sequence;
    if (!vm_runner_step_and_pause(machine, waiter, 4u) ||
        emulator_machine_published_frame_sequence(machine->executor) != sequence ||
        !emulator_machine_copy_published_frame(machine->executor, &published,
            run_generation) || !published.frame.graphics || published.frame.sequence != sequence)
        return LIB_FALSE;
    if (emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK ||
        !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_READ_PORT, .port = 0x03dau, .bytes = 1u
        }, &response) || !vm_runner_program_ports(machine, &lease, text,
            sizeof(text) / sizeof(text[0])) ||
        !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_WRITE_LINEAR, .address = 0xa0000u + 65u * 32u,
            .bytes = 1u, .data = {0x80u}
        }, &response) || !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_WRITE_LINEAR, .address = 0xa0000u + 8192u + 65u * 32u,
            .bytes = 1u, .data = {0x40u}
        }, &response)) return LIB_FALSE;
    for (lib_u8 plane = 0u; plane < 2u; ++plane) {
        const lib_u8 first = plane == 0u ? 'A' : 0x17u;
        const lib_u8 last = plane == 0u ? 'A' : 0x2fu;

        if (!vm_runner_program_ports(machine, &lease, (const vm_runner_port_write[]) {
                {0x03c4u, 2u}, {0x03c5u, (lib_u8)(1u << plane)} }, 2u) ||
            !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
                .operation = X86_DEBUG_WRITE_LINEAR, .address = 0xa0000u,
                .bytes = 1u, .data = {first}
            }, &response) || !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
                .operation = X86_DEBUG_WRITE_LINEAR, .address = 0xa0000u + 3920u,
                .bytes = 1u, .data = {last}
            }, &response)) return LIB_FALSE;
    }
    if (!emulator_machine_copy_published_frame(machine->executor, &published,
            run_generation) || !published.frame.graphics || published.frame.sequence != sequence ||
        !vm_runner_step_and_pause(machine, waiter, 4u) ||
        !emulator_machine_copy_published_frame(machine->executor, &published, run_generation) ||
        published.frame.graphics || published.frame.sequence != sequence + 1u ||
        published.data.text->base.text_columns != 80u || published.data.text->base.text_rows != 50u ||
        published.data.text->base.cells[0].glyph_index != 'A' ||
        published.data.text->base.cells[0].glyph_bank != 0u ||
        published.data.text->base.cells[3920u].glyph_index != 'A' ||
        published.data.text->base.cells[3920u].glyph_bank != 1u ||
        published.data.text->font[65u * 32u] != 0x80u ||
        published.data.text->secondary_font[65u * 32u] != 0x40u ||
        !published.data.text->base.cursor_phase ||
        published.data.text->base.cursor_column != 2 || published.data.text->base.cursor_row != 49)
        return LIB_FALSE;
    sequence = published.frame.sequence;
    if (emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK ||
        !vm_runner_program_ports(machine, &lease, (const vm_runner_port_write[]) {
            {0x03c4u, 2u}, {0x03c5u, 4u} }, 2u) ||
        !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_WRITE_LINEAR, .address = 0xa0000u + 8192u + 65u * 32u,
            .bytes = 1u, .data = {0x20u}
        }, &response) || !vm_runner_step_and_pause(machine, waiter, 4u) ||
        !emulator_machine_copy_published_frame(machine->executor, &published, run_generation) ||
        published.frame.sequence != sequence + 1u ||
        published.data.text->secondary_font[65u * 32u] != 0x20u) return LIB_FALSE;
    sequence = published.frame.sequence;
    if (!vm_runner_step_and_pause(machine, waiter, 4u) ||
        emulator_machine_published_frame_sequence(machine->executor) != sequence)
        return LIB_FALSE;
    if (emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK ||
        !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_READ_PORT, .port = 0x03dau, .bytes = 1u
        }, &response) || !vm_runner_program_ports(machine, &lease,
            (const vm_runner_port_write[]) {
                {0x03c4u, 0u}, {0x03c5u, 1u}, {0x03c4u, 1u}, {0x03c5u, 0u},
                {0x03c4u, 0u}, {0x03c5u, 3u}, {0x03c0u, 0x33u}, {0x03c0u, 8u}
            }, 8u) || !vm_runner_step_and_pause(machine, waiter, 4u) ||
        !emulator_machine_copy_published_frame(machine->executor, &published, run_generation) ||
        !published.frame.graphics || published.data.graphics->width != 720u ||
        published.data.graphics->height != 400u || !vm_runner_pixel_equals(machine, &published, 0, 7u) ||
        !vm_runner_pixel_equals(machine, &published, 1, 1u) ||
        !vm_runner_pixel_equals(machine, &published, 392u * 720u + 2u, 7u) ||
        published.frame.sequence != sequence + 1u) {
        lib_c_fprintf(lib_c_stderr, "RUNNER:PIXEL-TEXT:graphics=%u width=%u height=%u sequence=%u expected=%u first=%u second=%u last=%u\n",
            published.frame.graphics != LIB_NULL, published.data.graphics->width, published.data.graphics->height,
            published.frame.sequence, sequence + 1u, published.data.graphics->pixels[0],
            published.data.graphics->pixels[1], published.data.graphics->pixels[392u * 720u + 2u]);
        return LIB_FALSE;
    }
    sequence = published.frame.sequence;
    if (!vm_runner_step_and_pause(machine, waiter, 4u) ||
        emulator_machine_published_frame_sequence(machine->executor) != sequence)
        return LIB_FALSE;
    if (emulator_machine_debug_acquire(machine->executor, &lease) != LIB_STATUS_OK ||
        !vm_runner_debug_execute(machine, &lease, &(x86_debug_request) {
            .operation = X86_DEBUG_READ_PORT, .port = 0x03dau, .bytes = 1u
        }, &response) || !vm_runner_program_ports(machine, &lease,
            (const vm_runner_port_write[]) {
                {0x03c4u, 0u}, {0x03c5u, 1u}, {0x03c4u, 1u}, {0x03c5u, 1u},
                {0x03c4u, 0u}, {0x03c5u, 3u}, {0x03c0u, 0x33u}, {0x03c0u, 0u},
                {0x03d4u, 0x14u}, {0x03d5u, 1u}, {0x03c4u, 2u}, {0x03c5u, 2u}
            }, 12u) || !vm_runner_debug_execute(machine, &lease,
            &(x86_debug_request) {.operation = X86_DEBUG_WRITE_LINEAR,
                .address = 0xa0000u, .bytes = 1u, .data = {1u}}, &response) ||
        !vm_runner_step_and_pause(machine, waiter, 4u) ||
        !emulator_machine_copy_published_frame(machine->executor, &published, run_generation) ||
        !published.frame.graphics || published.data.graphics->width != 640u ||
        published.data.graphics->height != 400u || !vm_runner_pixel_equals(machine, &published, 0, 1u) ||
        !vm_runner_pixel_equals(machine, &published, 640u, 0u) || published.frame.sequence != sequence + 1u)
        return LIB_FALSE;
    for (lib_u16 x = 0u; x < 8u; ++x)
        if (!vm_runner_pixel_equals(machine, &published, 1280u + x, 1u)) return LIB_FALSE;
    sequence = published.frame.sequence;
    if (!vm_runner_blink_cycle(machine, waiter, &published, run_generation, LIB_FALSE) ||
        !vm_runner_blink_cycle(machine, waiter, &published, run_generation, LIB_TRUE))
        return LIB_FALSE;
    sequence = published.frame.sequence;
    if (base_sync_event_reset(waiter->event) != LIB_STATUS_OK ||
        !emulator_machine_reset(machine->executor) ||
        base_sync_event_wait(waiter->event, 2000u) != BASE_SYNC_WAIT_SIGNALED ||
        emulator_machine_state_get(machine->executor) != EMULATOR_MACHINE_PAUSED ||
        emulator_machine_copy_published_frame(machine->executor, &published,
            run_generation) ||
        !emulator_machine_copy_published_frame(machine->executor, &published,
            emulator_machine_run_generation(machine->executor))) return LIB_FALSE;
    return published.frame.text != LIB_NULL && !published.frame.graphics &&
        published.data.text->base.text_columns == 1u &&
        published.data.text->base.text_rows == 1u &&
        published.data.text->font[65u * 32u] == 0u &&
        published.data.text->secondary_font[65u * 32u] == 0u &&
        published.frame.sequence == sequence + 1u;
}

lib_i32 main(void)
{
    x86_nxvm_machine *session = LIB_NULL;
    vm_test_emulator_machine_state_waiter waiter = {0};
    static kvm_window_frame_storage frame;
    kvm_console_text_frame console = {0};
    lib_bool produced;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        x86_nxvm_machine_set_speed(session, X86_NXVM_MACHINE_SPEED_TURBO) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    if (x86_nxvm_machine_publish_display(session, LIB_TRUE) != LIB_STATUS_OK ||
        x86_nxvm_machine_copy_emulator_frame(session, &frame, &console, &produced) != LIB_STATUS_OK ||
        !produced ||
        frame.data.text->base.text_columns != 1u ||
        frame.data.text->base.text_rows != 1u ||
        vm_test_emulator_machine_bind(session) != LIB_STATUS_OK ||
        vm_test_emulator_machine_state_waiter_initialize(session, &waiter) !=
            LIB_STATUS_OK ||
        x86_nxvm_machine_resume(session) != LIB_STATUS_OK ||
        !vm_test_emulator_machine_wait_state(session, &waiter,
            EMULATOR_MACHINE_RUNNING, 2000u) ||
        x86_nxvm_machine_request_pause(session) != LIB_STATUS_OK ||
        !vm_test_emulator_machine_wait_state(session, &waiter,
            EMULATOR_MACHINE_PAUSED, 2000u) ||
        x86_nxvm_machine_resume(session) != LIB_STATUS_OK ||
        !vm_test_emulator_machine_wait_state(session, &waiter,
            EMULATOR_MACHINE_RUNNING, 2000u) ||
        x86_nxvm_machine_request_pause(session) != LIB_STATUS_OK ||
        !vm_test_emulator_machine_wait_state(session, &waiter,
            EMULATOR_MACHINE_PAUSED, 2000u)) {
        failed = 1;
        goto done;
    }
    failed |= !vm_runner_bound_delivery(session, &waiter);
done:
    x86_nxvm_machine_stop(session);
    failed |= !vm_test_emulator_machine_wait_state(session, &waiter,
        EMULATOR_MACHINE_STOPPED, 2000u);
    vm_test_emulator_machine_unbind(session);
    vm_test_emulator_machine_state_waiter_finalize(&waiter);
    x86_nxvm_machine_destroy(session);
    if (failed) return 1;
    lib_c_printf("RUNNER-CADENCE:OK\nCOMPOSITION-RUNNER-CADENCE:OK\n");
    return 0;
}
