#include "lib/types/types_interface.h"

#include "app-nxvm/devices/xt_keyboard.h"

/* The manual gives ranges. These lower conforming points make Core
 * deterministic on its macro axis; they are not physical-time claims. */
#define CORE_MACHINE_XT_KEYBOARD_RESET_US 12500u
#define CORE_MACHINE_XT_KEYBOARD_BAT_US 300000u
#define CORE_MACHINE_XT_KEYBOARD_FIRST_EDGE_US 60u
#define CORE_MACHINE_XT_KEYBOARD_CLOCK_US 25u

static lib_u64 core_machine_xt_keyboard_us_to_ticks(
    const core_machine_xt_keyboard *keyboard, lib_u32 microseconds)
{
    lib_u64 numerator;

    if (keyboard == LIB_NULL || keyboard->ticks_per_second == 0u ||
        keyboard->ticks_per_second > UINT64_MAX / microseconds) return 1u;
    numerator = keyboard->ticks_per_second * microseconds;
    return (numerator + 999999u) / 1000000u;
}

static void core_machine_xt_keyboard_start_serial(core_machine_xt_keyboard *keyboard)
{
    if (keyboard == LIB_NULL || keyboard->serial_active || keyboard->bat_active ||
        keyboard->clock_held || keyboard->clear_asserted || keyboard->fifo_count == 0u ||
        keyboard->ppi == LIB_NULL || keyboard->ticks_per_second == 0u) return;
    keyboard->serial_byte = keyboard->fifo[keyboard->fifo_head];
    keyboard->serial_bits_remaining = 9u;
    keyboard->serial_remaining_ticks = core_machine_xt_keyboard_us_to_ticks(keyboard,
        CORE_MACHINE_XT_KEYBOARD_FIRST_EDGE_US);
    keyboard->serial_active = LIB_TRUE;
    keyboard->serial_response = LIB_FALSE;
}

static void core_machine_xt_keyboard_finish_serial(core_machine_xt_keyboard *keyboard)
{
    if (keyboard == LIB_NULL || !keyboard->serial_active || keyboard->ppi == LIB_NULL ||
        core_machine_xt_ppi_keyboard_receive_device_byte(keyboard->ppi,
            keyboard->serial_byte) != LIB_STATUS_OK) return;
    if (!keyboard->serial_response) {
        keyboard->fifo_head = (lib_u8)((keyboard->fifo_head + 1u) %
            CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY);
        --keyboard->fifo_count;
    }
    keyboard->serial_active = LIB_FALSE;
    keyboard->serial_response = LIB_FALSE;
    keyboard->serial_remaining_ticks = 0u;
}

lib_status core_machine_xt_keyboard_initialize(core_machine_xt_keyboard *keyboard,
    core_machine_xt_ppi_keyboard *ppi, lib_u64 ticks_per_second)
{
    if (keyboard == LIB_NULL || ppi == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    keyboard->ppi = ppi;
    keyboard->ticks_per_second = ticks_per_second;
    core_machine_xt_keyboard_reset(keyboard);
    return LIB_STATUS_OK;
}

void core_machine_xt_keyboard_reset(core_machine_xt_keyboard *keyboard)
{
    if (keyboard == LIB_NULL) return;
    keyboard->reset_low_ticks = keyboard->ticks_per_second == 0u ? UINT64_MAX :
        core_machine_xt_keyboard_us_to_ticks(keyboard, CORE_MACHINE_XT_KEYBOARD_RESET_US);
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

void core_machine_xt_keyboard_finalize(core_machine_xt_keyboard *keyboard)
{
    if (keyboard == LIB_NULL) return;
    keyboard->ppi = LIB_NULL;
    keyboard->ticks_per_second = 0u;
    core_machine_xt_keyboard_reset(keyboard);
}

void core_machine_xt_keyboard_observe_ppi_lines(void *owner,
    lib_u8 clock_held, lib_u8 clear_asserted)
{
    core_machine_xt_keyboard *keyboard = (core_machine_xt_keyboard *)owner;

    if (keyboard == LIB_NULL) return;
    if (keyboard->clock_held && !clock_held) {
        if (keyboard->clock_low_ticks >= keyboard->reset_low_ticks) {
            keyboard->bat_active = LIB_TRUE;
            keyboard->bat_remaining_ticks = core_machine_xt_keyboard_us_to_ticks(keyboard,
                CORE_MACHINE_XT_KEYBOARD_BAT_US);
            keyboard->serial_active = LIB_FALSE;
        }
        keyboard->clock_low_ticks = 0u;
    }
    keyboard->clock_held = clock_held;
    keyboard->clear_asserted = clear_asserted;
    if (!clock_held && !clear_asserted) core_machine_xt_keyboard_start_serial(keyboard);
}

void core_machine_xt_keyboard_notify_ppi_byte_released(void *owner)
{
    core_machine_xt_keyboard_start_serial((core_machine_xt_keyboard *)owner);
}

lib_status core_machine_xt_keyboard_receive_native_bytes(core_machine_xt_keyboard *keyboard,
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
    if (count > CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY - keyboard->fifo_count) {
        tail = (lib_u8)((keyboard->fifo_head + keyboard->fifo_count - 1u) %
            CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY);
        if (keyboard->fifo_count < CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY) {
            tail = (lib_u8)((keyboard->fifo_head + keyboard->fifo_count) %
                CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY);
            ++keyboard->fifo_count;
        }
        keyboard->fifo[tail] = 0xffu;
        core_machine_xt_keyboard_start_serial(keyboard);
        return LIB_STATUS_OK;
    }
    for (index = 0u; index < count; ++index) {
        lib_u8 tail = (lib_u8)((keyboard->fifo_head +
            keyboard->fifo_count) % CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY);
        keyboard->fifo[tail] = bytes[index];
        ++keyboard->fifo_count;
    }
    core_machine_xt_keyboard_start_serial(keyboard);
    return LIB_STATUS_OK;
}

void core_machine_xt_keyboard_advance(core_machine_xt_keyboard *keyboard,
    lib_u64 ticks)
{
    if (keyboard == LIB_NULL || ticks == 0u) return;
    if (keyboard->clock_held && !keyboard->bat_active) {
        keyboard->clock_low_ticks = UINT64_MAX - keyboard->clock_low_ticks < ticks ?
            UINT64_MAX : keyboard->clock_low_ticks + ticks;
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
        if (keyboard->bat_result_pending && !keyboard->clock_held &&
            !keyboard->clear_asserted && !keyboard->serial_active) {
            keyboard->serial_byte = 0xaau;
            keyboard->serial_bits_remaining = 9u;
            keyboard->serial_remaining_ticks = core_machine_xt_keyboard_us_to_ticks(keyboard,
                CORE_MACHINE_XT_KEYBOARD_FIRST_EDGE_US);
            keyboard->serial_active = LIB_TRUE;
            keyboard->serial_response = LIB_TRUE;
            keyboard->bat_result_pending = LIB_FALSE;
        }
        if (!keyboard->serial_active || ticks < keyboard->serial_remaining_ticks) {
            if (keyboard->serial_active) keyboard->serial_remaining_ticks -= ticks;
            return;
        }
        ticks -= keyboard->serial_remaining_ticks;
        if (--keyboard->serial_bits_remaining == 0u) {
            core_machine_xt_keyboard_finish_serial(keyboard);
        } else {
            keyboard->serial_remaining_ticks = core_machine_xt_keyboard_us_to_ticks(keyboard,
                CORE_MACHINE_XT_KEYBOARD_CLOCK_US);
        }
    }
}

lib_status core_machine_xt_keyboard_ticks_until_event(const core_machine_xt_keyboard *keyboard,
    lib_u64 *out_ticks)
{
    if (keyboard == LIB_NULL || out_ticks == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (keyboard->bat_active) *out_ticks = keyboard->bat_remaining_ticks;
    else if (keyboard->serial_active) *out_ticks = keyboard->serial_remaining_ticks;
    else return LIB_STATUS_UNSUPPORTED;
    return *out_ticks == 0u ? LIB_STATUS_UNSUPPORTED : LIB_STATUS_OK;
}
