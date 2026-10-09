/* Copyright 2012-2026 Neko. */
#include "core/chips/xtkeyboard/xtkeyboard_interface.h"

#define X86_XT_KEYBOARD_FIFO_CAPACITY 16u

struct x86_xt_keyboard {
    x86_xt_keyboard_timing timing;
    x86_xt_keyboard_receive receive;
    void *context;
    lib_u64 serial_remaining_ticks;
    lib_u64 bat_remaining_ticks;
    lib_u64 clock_low_ticks;
    lib_u8 fifo[X86_XT_KEYBOARD_FIFO_CAPACITY];
    lib_u8 fifo_head, fifo_count, serial_byte, serial_bits_remaining;
    lib_bool clock_held, clear_asserted, serial_active, serial_response;
    lib_bool bat_active, bat_result_pending;
};

static void x86_xt_keyboard_finish_serial(x86_xt_keyboard *keyboard)
{
    if (keyboard == LIB_NULL || !keyboard->serial_active) return;
    keyboard->serial_remaining_ticks = 0u;
    if (keyboard->receive(keyboard->context,
            keyboard->serial_byte) != LIB_STATUS_OK) return;
    if (!keyboard->serial_response) {
        keyboard->fifo_head = (lib_u8)((keyboard->fifo_head + 1u) %
            X86_XT_KEYBOARD_FIFO_CAPACITY);
        --keyboard->fifo_count;
    }
    keyboard->serial_active = LIB_FALSE;
    keyboard->serial_response = LIB_FALSE;
    keyboard->serial_remaining_ticks = 0u;
}

static void x86_xt_keyboard_start_serial(x86_xt_keyboard *keyboard)
{
    if (keyboard == LIB_NULL || keyboard->bat_active || keyboard->clock_held ||
        keyboard->clear_asserted || keyboard->timing.first_edge_ticks == 0u) return;
    if (keyboard->serial_active) {
        /* A completed frame waits for receiver release, not more bit clocks. */
        if (keyboard->serial_bits_remaining == 0u) {
            x86_xt_keyboard_finish_serial(keyboard);
        }
        return;
    }
    if (!keyboard->bat_result_pending && keyboard->fifo_count == 0u) return;
    keyboard->serial_response = keyboard->bat_result_pending;
    keyboard->serial_byte = keyboard->bat_result_pending ? 0xaau : keyboard->fifo[keyboard->fifo_head];
    keyboard->bat_result_pending = LIB_FALSE;
    keyboard->serial_bits_remaining = 9u;
    keyboard->serial_remaining_ticks = keyboard->timing.first_edge_ticks;
    keyboard->serial_active = LIB_TRUE;
}

lib_status x86_xt_keyboard_create(const x86_xt_keyboard_timing *timing,
    x86_xt_keyboard_receive receive, void *context, x86_xt_keyboard **out_keyboard)
{
    x86_xt_keyboard *keyboard;
    lib_bool disabled;

    if (out_keyboard == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_keyboard = LIB_NULL;
    if (timing == LIB_NULL || receive == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    disabled = timing->reset_ticks == 0u && timing->bat_ticks == 0u &&
        timing->first_edge_ticks == 0u && timing->clock_ticks == 0u;
    if (!disabled && (timing->reset_ticks == 0u || timing->bat_ticks == 0u ||
        timing->first_edge_ticks == 0u || timing->clock_ticks == 0u)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    keyboard = lib_allocate_zero(1u, sizeof(*keyboard));
    if (keyboard == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    keyboard->timing = *timing;
    keyboard->receive = receive;
    keyboard->context = context;
    x86_xt_keyboard_reset(keyboard);
    *out_keyboard = keyboard;
    return LIB_STATUS_OK;
}

void x86_xt_keyboard_reset(x86_xt_keyboard *keyboard)
{
    if (keyboard == LIB_NULL) return;
    keyboard->serial_remaining_ticks = 0u;
    keyboard->bat_remaining_ticks = 0u;
    keyboard->clock_low_ticks = 0u;
    keyboard->fifo_head = 0u;
    keyboard->fifo_count = 0u;
    keyboard->serial_byte = 0u;
    keyboard->serial_bits_remaining = 0u;
    keyboard->clock_held = LIB_TRUE;
    keyboard->clear_asserted = LIB_FALSE;
    keyboard->serial_active = LIB_FALSE;
    keyboard->serial_response = LIB_FALSE;
    keyboard->bat_active = LIB_FALSE;
    keyboard->bat_result_pending = LIB_FALSE;
}

void x86_xt_keyboard_destroy(x86_xt_keyboard *keyboard)
{
    lib_release(keyboard);
}

void x86_xt_keyboard_set_lines(x86_xt_keyboard *keyboard,
    lib_bool clock_held, lib_bool clear_asserted)
{
    if (keyboard == LIB_NULL) return;
    if (keyboard->clock_held && !clock_held) {
        if (keyboard->timing.reset_ticks != 0u &&
            keyboard->clock_low_ticks >= keyboard->timing.reset_ticks) {
            keyboard->bat_active = LIB_TRUE;
            keyboard->bat_remaining_ticks = keyboard->timing.bat_ticks;
            keyboard->serial_active = LIB_FALSE;
        }
        keyboard->clock_low_ticks = 0u;
    }
    keyboard->clock_held = clock_held;
    keyboard->clear_asserted = clear_asserted;
    if (!clock_held && !clear_asserted) x86_xt_keyboard_start_serial(keyboard);
}

void x86_xt_keyboard_receiver_ready(x86_xt_keyboard *keyboard)
{
    x86_xt_keyboard_start_serial(keyboard);
}

lib_status x86_xt_keyboard_receive_native_bytes(x86_xt_keyboard *keyboard,
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;
    lib_u8 tail;

    if (keyboard == LIB_NULL || (bytes == LIB_NULL && count != 0u)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    /* The XT reference requires a sequence that cannot fit to be discarded
     * whole and reported as one FF overrun character, replacing the last
     * queued character if the FIFO is already full. */
    if (count > X86_XT_KEYBOARD_FIFO_CAPACITY - keyboard->fifo_count) {
        tail = (lib_u8)((keyboard->fifo_head + keyboard->fifo_count - 1u) %
            X86_XT_KEYBOARD_FIFO_CAPACITY);
        if (keyboard->fifo_count < X86_XT_KEYBOARD_FIFO_CAPACITY) {
            tail = (lib_u8)((keyboard->fifo_head + keyboard->fifo_count) %
                X86_XT_KEYBOARD_FIFO_CAPACITY);
            ++keyboard->fifo_count;
        }
        keyboard->fifo[tail] = 0xffu;
        x86_xt_keyboard_start_serial(keyboard);
        return LIB_STATUS_OK;
    }
    for (index = 0u; index < count; ++index) {
        lib_u8 tail = (lib_u8)((keyboard->fifo_head +
            keyboard->fifo_count) % X86_XT_KEYBOARD_FIFO_CAPACITY);
        keyboard->fifo[tail] = bytes[index];
        ++keyboard->fifo_count;
    }
    x86_xt_keyboard_start_serial(keyboard);
    return LIB_STATUS_OK;
}

void x86_xt_keyboard_advance(x86_xt_keyboard *keyboard,
    lib_u64 ticks)
{
    if (keyboard == LIB_NULL || ticks == 0u) return;
    if (keyboard->clock_held && !keyboard->bat_active) {
        keyboard->clock_low_ticks = LIB_UINT64_MAX - keyboard->clock_low_ticks < ticks ?
            LIB_UINT64_MAX : keyboard->clock_low_ticks + ticks;
        return;
    }
    while (ticks != 0u) {
        if (keyboard->bat_active) {
            if (ticks < keyboard->bat_remaining_ticks) {
                keyboard->bat_remaining_ticks -= ticks;
                return;
            }
            ticks -= keyboard->bat_remaining_ticks;
            keyboard->bat_active = LIB_FALSE;
            keyboard->bat_remaining_ticks = 0u;
            keyboard->bat_result_pending = LIB_TRUE;
        }
        if (keyboard->bat_result_pending) x86_xt_keyboard_start_serial(keyboard);
        if (keyboard->serial_active && keyboard->serial_bits_remaining == 0u) return;
        if (!keyboard->serial_active || ticks < keyboard->serial_remaining_ticks) {
            if (keyboard->serial_active) keyboard->serial_remaining_ticks -= ticks;
            return;
        }
        ticks -= keyboard->serial_remaining_ticks;
        if (--keyboard->serial_bits_remaining == 0u) {
            x86_xt_keyboard_finish_serial(keyboard);
        } else {
            keyboard->serial_remaining_ticks = keyboard->timing.clock_ticks;
        }
    }
}

lib_status x86_xt_keyboard_ticks_until_event(const x86_xt_keyboard *keyboard,
    lib_u64 *out_ticks)
{
    if (keyboard == LIB_NULL || out_ticks == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (keyboard->bat_active) *out_ticks = keyboard->bat_remaining_ticks;
    else if (keyboard->serial_active && !keyboard->clock_held) {
        *out_ticks = keyboard->serial_remaining_ticks;
    }
    else return LIB_STATUS_UNSUPPORTED;
    return *out_ticks == 0u ? LIB_STATUS_UNSUPPORTED : LIB_STATUS_OK;
}
