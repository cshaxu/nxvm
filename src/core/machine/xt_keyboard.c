#include "type.h"

#include "core/machine/xt_keyboard.h"

/* The manual gives ranges. These lower conforming points make Core
 * deterministic on its macro axis; they are not physical-time claims. */
#define CORE_MACHINE_XT_KEYBOARD_RESET_US 12500u
#define CORE_MACHINE_XT_KEYBOARD_BAT_US 300000u
#define CORE_MACHINE_XT_KEYBOARD_FIRST_EDGE_US 60u
#define CORE_MACHINE_XT_KEYBOARD_CLOCK_US 25u

static type_unsigned_64 core_machine_xt_keyboard_us_to_ticks(
    const core_machine_xt_keyboard *keyboard, type_unsigned_32 microseconds)
{
    type_unsigned_64 numerator;

    if (keyboard == STD_NULL || keyboard->ticks_per_second == 0u ||
        keyboard->ticks_per_second > UINT64_MAX / microseconds) return 1u;
    numerator = keyboard->ticks_per_second * microseconds;
    return (numerator + 999999u) / 1000000u;
}

static C_VOID core_machine_xt_keyboard_start_serial(core_machine_xt_keyboard *keyboard)
{
    if (keyboard == STD_NULL || keyboard->serial_active || keyboard->bat_active ||
        keyboard->clock_held || keyboard->clear_asserted || keyboard->fifo_count == 0u ||
        keyboard->ppi == STD_NULL || keyboard->ticks_per_second == 0u) return;
    keyboard->serial_byte = keyboard->fifo[keyboard->fifo_head];
    keyboard->serial_bits_remaining = 9u;
    keyboard->serial_remaining_ticks = core_machine_xt_keyboard_us_to_ticks(keyboard,
        CORE_MACHINE_XT_KEYBOARD_FIRST_EDGE_US);
    keyboard->serial_active = TYPE_TRUE;
    keyboard->serial_response = TYPE_FALSE;
}

static C_VOID core_machine_xt_keyboard_finish_serial(core_machine_xt_keyboard *keyboard)
{
    if (keyboard == STD_NULL || !keyboard->serial_active || keyboard->ppi == STD_NULL ||
        core_machine_xt_ppi_keyboard_receive_device_byte(keyboard->ppi,
            keyboard->serial_byte) != TYPE_STATUS_OK) return;
    if (!keyboard->serial_response) {
        keyboard->fifo_head = (type_unsigned_8)((keyboard->fifo_head + 1u) %
            CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY);
        --keyboard->fifo_count;
    }
    keyboard->serial_active = TYPE_FALSE;
    keyboard->serial_response = TYPE_FALSE;
    keyboard->serial_remaining_ticks = 0u;
}

type_status core_machine_xt_keyboard_initialize(core_machine_xt_keyboard *keyboard,
    core_machine_xt_ppi_keyboard *ppi, type_unsigned_64 ticks_per_second)
{
    if (keyboard == STD_NULL || ppi == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    keyboard->ppi = ppi;
    keyboard->ticks_per_second = ticks_per_second;
    core_machine_xt_keyboard_reset(keyboard);
    return TYPE_STATUS_OK;
}

C_VOID core_machine_xt_keyboard_reset(core_machine_xt_keyboard *keyboard)
{
    if (keyboard == STD_NULL) return;
    keyboard->reset_low_ticks = keyboard->ticks_per_second == 0u ? UINT64_MAX :
        core_machine_xt_keyboard_us_to_ticks(keyboard, CORE_MACHINE_XT_KEYBOARD_RESET_US);
    keyboard->serial_remaining_ticks = 0u;
    keyboard->bat_remaining_ticks = 0u;
    keyboard->clock_low_ticks = 0u;
    keyboard->fifo_head = 0u;
    keyboard->fifo_count = 0u;
    keyboard->serial_byte = 0u;
    keyboard->serial_bits_remaining = 0u;
    keyboard->clock_held = TYPE_TRUE;
    keyboard->clear_asserted = TYPE_FALSE;
    keyboard->serial_active = TYPE_FALSE;
    keyboard->serial_response = TYPE_FALSE;
    keyboard->bat_active = TYPE_FALSE;
    keyboard->bat_result_pending = TYPE_FALSE;
}

C_VOID core_machine_xt_keyboard_finalize(core_machine_xt_keyboard *keyboard)
{
    if (keyboard == STD_NULL) return;
    keyboard->ppi = STD_NULL;
    keyboard->ticks_per_second = 0u;
    core_machine_xt_keyboard_reset(keyboard);
}

C_VOID core_machine_xt_keyboard_observe_ppi_lines(C_VOID *owner,
    type_bool clock_held, type_bool clear_asserted)
{
    core_machine_xt_keyboard *keyboard = (core_machine_xt_keyboard *)owner;

    if (keyboard == STD_NULL) return;
    if (keyboard->clock_held && !clock_held) {
        if (keyboard->clock_low_ticks >= keyboard->reset_low_ticks) {
            keyboard->bat_active = TYPE_TRUE;
            keyboard->bat_remaining_ticks = core_machine_xt_keyboard_us_to_ticks(keyboard,
                CORE_MACHINE_XT_KEYBOARD_BAT_US);
            keyboard->serial_active = TYPE_FALSE;
        }
        keyboard->clock_low_ticks = 0u;
    }
    keyboard->clock_held = clock_held;
    keyboard->clear_asserted = clear_asserted;
    if (!clock_held && !clear_asserted) core_machine_xt_keyboard_start_serial(keyboard);
}

C_VOID core_machine_xt_keyboard_notify_ppi_byte_released(C_VOID *owner)
{
    core_machine_xt_keyboard_start_serial((core_machine_xt_keyboard *)owner);
}

type_status core_machine_xt_keyboard_receive_native_bytes(core_machine_xt_keyboard *keyboard,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    STD_SIZE_T index;
    type_unsigned_8 tail;

    if (keyboard == STD_NULL || (bytes == STD_NULL && count != 0u)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    /* The XT reference requires a sequence that cannot fit to be discarded
     * whole and reported as one FF overrun character, replacing the last
     * queued character if the FIFO is already full. */
    if (count > CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY - keyboard->fifo_count) {
        tail = (type_unsigned_8)((keyboard->fifo_head + keyboard->fifo_count - 1u) %
            CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY);
        if (keyboard->fifo_count < CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY) {
            tail = (type_unsigned_8)((keyboard->fifo_head + keyboard->fifo_count) %
                CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY);
            ++keyboard->fifo_count;
        }
        keyboard->fifo[tail] = 0xffu;
        core_machine_xt_keyboard_start_serial(keyboard);
        return TYPE_STATUS_OK;
    }
    for (index = 0u; index < count; ++index) {
        type_unsigned_8 tail = (type_unsigned_8)((keyboard->fifo_head +
            keyboard->fifo_count) % CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY);
        keyboard->fifo[tail] = bytes[index];
        ++keyboard->fifo_count;
    }
    core_machine_xt_keyboard_start_serial(keyboard);
    return TYPE_STATUS_OK;
}

C_VOID core_machine_xt_keyboard_advance(core_machine_xt_keyboard *keyboard,
    type_unsigned_64 ticks)
{
    if (keyboard == STD_NULL || ticks == 0u) return;
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
            keyboard->bat_active = TYPE_FALSE;
            keyboard->bat_remaining_ticks = 0u;
            keyboard->bat_result_pending = TYPE_TRUE;
        }
        if (keyboard->bat_result_pending && !keyboard->clock_held &&
            !keyboard->clear_asserted && !keyboard->serial_active) {
            keyboard->serial_byte = 0xaau;
            keyboard->serial_bits_remaining = 9u;
            keyboard->serial_remaining_ticks = core_machine_xt_keyboard_us_to_ticks(keyboard,
                CORE_MACHINE_XT_KEYBOARD_FIRST_EDGE_US);
            keyboard->serial_active = TYPE_TRUE;
            keyboard->serial_response = TYPE_TRUE;
            keyboard->bat_result_pending = TYPE_FALSE;
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

type_status core_machine_xt_keyboard_ticks_until_event(const core_machine_xt_keyboard *keyboard,
    type_unsigned_64 *out_ticks)
{
    if (keyboard == STD_NULL || out_ticks == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (keyboard->bat_active) *out_ticks = keyboard->bat_remaining_ticks;
    else if (keyboard->serial_active) *out_ticks = keyboard->serial_remaining_ticks;
    else return TYPE_STATUS_UNSUPPORTED;
    return *out_ticks == 0u ? TYPE_STATUS_UNSUPPORTED : TYPE_STATUS_OK;
}
