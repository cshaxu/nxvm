/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/cpu.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"


#include "vm/profile/default_profile/firmware/context.h"


#include "qdkeyb.h"

static type_unsigned_8 keyboard_read_byte(vm_profile_default_context *profile,
    type_unsigned_16 offset)
{
    type_unsigned_8 value = 0;
    core_machine_memory_read_real_from(vm_profile_default_context_memory(profile), TYPE_ZERO_16, offset, &value,
        sizeof(value));
    return value;
}

static type_unsigned_16 keyboard_read_word(vm_profile_default_context *profile,
    type_unsigned_16 offset)
{
    type_unsigned_16 value = 0;
    core_machine_memory_read_real_from(vm_profile_default_context_memory(profile), TYPE_ZERO_16, offset, &value,
        sizeof(value));
    return value;
}

static C_VOID keyboard_write_byte(vm_profile_default_context *profile,
    type_unsigned_16 offset, type_unsigned_8 value)
{
    core_machine_memory_write_real_to(vm_profile_default_context_memory(profile), TYPE_ZERO_16, offset, &value,
        sizeof(value));
}

static C_VOID keyboard_write_word(vm_profile_default_context *profile,
    type_unsigned_16 offset, type_unsigned_16 value)
{
    core_machine_memory_write_real_to(vm_profile_default_context_memory(profile), TYPE_ZERO_16, offset, &value,
        sizeof(value));
}

static type_unsigned_16 keyboard_buffer_next(type_unsigned_16 pointer)
{
    return QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START +
        (pointer - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 2u) %
        (QDKEYB_VBIOS_ADDR_KEYB_BUFFER_END -
         QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 1u);
}

static type_bool keyboard_buffer_empty(vm_profile_default_context *profile)
{
    return keyboard_read_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD) ==
        keyboard_read_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL);
}

static type_bool keyboard_buffer_full(vm_profile_default_context *profile)
{
    return keyboard_buffer_next(keyboard_read_word(profile,
        QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL)) == keyboard_read_word(profile,
        QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD);
}

static type_bool keyboard_buffer_push(vm_profile_default_context *profile,
    type_unsigned_16 code)
{
    type_unsigned_16 tail;

    if (keyboard_buffer_full(profile)) return TYPE_TRUE;
    tail = keyboard_read_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL);
    keyboard_write_word(profile, tail, code);
    keyboard_write_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL,
        keyboard_buffer_next(tail));
    return TYPE_FALSE;
}

static type_unsigned_16 keyboard_buffer_pop(vm_profile_default_context *profile)
{
    type_unsigned_16 head;
    type_unsigned_16 result;

    if (keyboard_buffer_empty(profile)) return 0;
    head = keyboard_read_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD);
    result = keyboard_read_word(profile, head);
    keyboard_write_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD,
        keyboard_buffer_next(head));
    return result;
}

static type_unsigned_16 keyboard_buffer_peek(vm_profile_default_context *profile)
{
    return keyboard_read_word(profile, keyboard_read_word(profile,
        QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD));
}

static type_unsigned_8 keyboard_flag0(vm_profile_default_context *profile)
{
    return keyboard_read_byte(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0);
}

static C_VOID keyboard_set_flag0(vm_profile_default_context *profile,
    type_unsigned_8 mask, C_INT enabled)
{
    type_unsigned_8 value = keyboard_flag0(profile);
    if (enabled) TYPE_SET_BIT(value, mask); else TYPE_CLEAR_BIT(value, mask);
    keyboard_write_byte(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0, value);
}

static C_VOID keyboard_set_flag1(vm_profile_default_context *profile,
    type_unsigned_8 mask, C_INT enabled)
{
    type_unsigned_8 value = keyboard_read_byte(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG1);
    if (enabled) TYPE_SET_BIT(value, mask); else TYPE_CLEAR_BIT(value, mask);
    keyboard_write_byte(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG1, value);
}

static uint8_t keyboard_ascii_for_scan(vm_profile_default_context *profile,
    uint8_t scan_code)
{
    static const uint8_t normal[0x59] = {
        [0x01] = 0x1bu, [0x02] = '1', [0x03] = '2', [0x04] = '3',
        [0x05] = '4', [0x06] = '5', [0x07] = '6', [0x08] = '7',
        [0x09] = '8', [0x0a] = '9', [0x0b] = '0', [0x0c] = '-',
        [0x0d] = '=', [0x0e] = 0x08u, [0x0f] = 0x09u,
        [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
        [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
        [0x18] = 'o', [0x19] = 'p', [0x1a] = '[', [0x1b] = ']',
        [0x1c] = 0x0du, [0x1e] = 'a', [0x1f] = 's', [0x20] = 'd',
        [0x21] = 'f', [0x22] = 'g', [0x23] = 'h', [0x24] = 'j',
        [0x25] = 'k', [0x26] = 'l', [0x27] = ';', [0x28] = '\'',
        [0x29] = '`', [0x2b] = '\\', [0x2c] = 'z', [0x2d] = 'x',
        [0x2e] = 'c', [0x2f] = 'v', [0x30] = 'b', [0x31] = 'n',
        [0x32] = 'm', [0x33] = ',', [0x34] = '.', [0x35] = '/',
        [0x37] = '*', [0x39] = ' ', [0x4a] = '-', [0x4e] = '+'
    };
    static const uint8_t shifted[0x59] = {
        [0x01] = 0x1bu, [0x02] = '!', [0x03] = '@', [0x04] = '#',
        [0x05] = '$', [0x06] = '%', [0x07] = '^', [0x08] = '&',
        [0x09] = '*', [0x0a] = '(', [0x0b] = ')', [0x0c] = '_',
        [0x0d] = '+', [0x0e] = 0x08u, [0x10] = 'Q', [0x11] = 'W',
        [0x12] = 'E', [0x13] = 'R', [0x14] = 'T', [0x15] = 'Y',
        [0x16] = 'U', [0x17] = 'I', [0x18] = 'O', [0x19] = 'P',
        [0x1a] = '{', [0x1b] = '}', [0x1c] = 0x0du, [0x1e] = 'A',
        [0x1f] = 'S', [0x20] = 'D', [0x21] = 'F', [0x22] = 'G',
        [0x23] = 'H', [0x24] = 'J', [0x25] = 'K', [0x26] = 'L',
        [0x27] = ':', [0x28] = '"', [0x29] = '~', [0x2b] = '|',
        [0x2c] = 'Z', [0x2d] = 'X', [0x2e] = 'C', [0x2f] = 'V',
        [0x30] = 'B', [0x31] = 'N', [0x32] = 'M', [0x33] = '<',
        [0x34] = '>', [0x35] = '?', [0x37] = '*', [0x39] = ' ',
        [0x4a] = '-', [0x4e] = '+'
    };
    uint8_t flags = keyboard_flag0(profile);
    uint8_t value;
    type_bool shifted_state;

    if (scan_code >= sizeof(normal)) return 0u;
    value = normal[scan_code];
    if ((flags & QDKEYB_FLAG0_D_CTRL) != 0u && value >= 'a' && value <= 'z') {
        return (uint8_t)(value - 'a' + 1u);
    }
    if ((flags & QDKEYB_FLAG0_D_ALT) != 0u) return 0u;
    shifted_state = ((flags & (QDKEYB_FLAG0_D_LSHIFT | QDKEYB_FLAG0_D_RSHIFT)) != 0u);
    if (value >= 'a' && value <= 'z') {
        if (((flags & QDKEYB_FLAG0_A_CAPLCK) != 0u) != shifted_state) {
            return (uint8_t)(value - 'a' + 'A');
        }
        return value;
    }
    return shifted_state ? shifted[scan_code] : value;
}

static uint16_t keyboard_translate_scan(vm_profile_default_context *profile,
    uint8_t scan_code)
{
    return ((uint16_t)scan_code << 8) | keyboard_ascii_for_scan(profile,
        scan_code);
}

static C_VOID keyboard_read_input(vm_profile_default_context *profile)
{
    t_cpu *cpu = vm_profile_default_context_execution(profile)->cpu;
    if (keyboard_buffer_empty(profile)) {
        profile->keyboard_waiting = TYPE_TRUE;
        cpu->data.flagHalt = TYPE_TRUE;
        return;
    }
    cpu->data.ax = keyboard_buffer_pop(profile);
}

static C_VOID keyboard_get_status(vm_profile_default_context *profile)
{
    t_cpu *cpu = vm_profile_default_context_execution(profile)->cpu;
    type_unsigned_16 key = keyboard_buffer_peek(profile);

    if (keyboard_buffer_empty(profile)) {
        TYPE_SET_BIT(cpu->data.eflags, VCPU_EFLAGS_ZF);
        return;
    }
    switch (key) {
    case 0x1d00:
    case 0x2a00:
    case 0x3800:
        cpu->data.ax = TYPE_ZERO_16;
        break;
    default:
        cpu->data.ax = key;
        break;
    }
    TYPE_CLEAR_BIT(cpu->data.eflags, VCPU_EFLAGS_ZF);
}

C_VOID vm_profile_default_keyboard_handle_irq1(vm_profile_default_context *profile)
{
    core_machine_cpu_execution_context *execution =
        vm_profile_default_context_execution(profile);
    t_cpu *cpu;
    uint8_t scan_code;

    if (execution == STD_NULL || execution->cpu == STD_NULL) return;
    cpu = execution->cpu;
    scan_code = (uint8_t)core_machine_port_read(execution->port, 0x0060u);
    if (scan_code != 0u) {
        (C_VOID)keyboard_buffer_push(profile,
            keyboard_translate_scan(profile, scan_code));
    }
    if (profile->keyboard_waiting && !keyboard_buffer_empty(profile)) {
        cpu->data.ax = keyboard_buffer_pop(profile);
        profile->keyboard_waiting = TYPE_FALSE;
    }
    core_machine_port_write(execution->port, 0x0020, 0x20);
}

C_VOID vm_profile_default_keyboard_handle_int16(vm_profile_default_context *profile)
{
    t_cpu *cpu = vm_profile_default_context_execution(profile)->cpu;

    switch (cpu->data.ah) {
    case 0x00:
    case 0x10:
        keyboard_read_input(profile);
        break;
    case 0x01:
    case 0x11:
        keyboard_get_status(profile);
        break;
    case 0x02:
        cpu->data.al = keyboard_flag0(profile);
        break;
    case 0x05:
        cpu->data.al = keyboard_buffer_push(profile,
            ((type_unsigned_16)cpu->data.ch << 8) | cpu->data.cl);
        break;
    default:
        break;
    }
}

static C_INT keyboard_get_modifier(C_VOID *context,
    core_machine_keyboard_modifier modifier)
{
    vm_profile_default_context *profile = context;
    type_unsigned_8 flags = keyboard_flag0(profile);

    switch (modifier) {
    case CORE_MACHINE_KEYBOARD_MODIFIER_ALT:
        return TYPE_GET_BIT(flags, QDKEYB_FLAG0_D_ALT);
    case CORE_MACHINE_KEYBOARD_MODIFIER_CONTROL:
        return TYPE_GET_BIT(flags, QDKEYB_FLAG0_D_CTRL);
    case CORE_MACHINE_KEYBOARD_MODIFIER_SHIFT:
        return TYPE_GET_BIT(flags, QDKEYB_FLAG0_D_LSHIFT) ||
            TYPE_GET_BIT(flags, QDKEYB_FLAG0_D_RSHIFT);
    case CORE_MACHINE_KEYBOARD_MODIFIER_CAPS_LOCK:
        return TYPE_GET_BIT(flags, QDKEYB_FLAG0_A_CAPLCK);
    case CORE_MACHINE_KEYBOARD_MODIFIER_NUM_LOCK:
        return TYPE_GET_BIT(flags, QDKEYB_FLAG0_A_NUMLCK);
    }
    return TYPE_FALSE;
}

static C_VOID keyboard_apply_host_state(C_VOID *context,
    uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    vm_profile_default_context *profile = context;

    keyboard_set_flag0(profile, QDKEYB_FLAG0_D_RSHIFT,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_RIGHT_SHIFT) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_D_LSHIFT,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_LEFT_SHIFT) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_D_CTRL,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_CONTROL) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_D_ALT,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_ALT) != 0u);
    keyboard_set_flag1(profile, QDKEYB_FLAG1_D_SCRLCK,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_SCROLL_LOCK) != 0u);
    keyboard_set_flag1(profile, QDKEYB_FLAG1_D_NUMLCK,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_NUM_LOCK) != 0u);
    keyboard_set_flag1(profile, QDKEYB_FLAG1_D_CAPLCK,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_CAPS_LOCK) != 0u);
    keyboard_set_flag1(profile, QDKEYB_FLAG1_D_INSERT,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_INSERT) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_A_SCRLCK,
        (toggle_keys & CORE_MACHINE_KEYBOARD_TOGGLE_SCROLL_LOCK) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_A_NUMLCK,
        (toggle_keys & CORE_MACHINE_KEYBOARD_TOGGLE_NUM_LOCK) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_A_CAPLCK,
        (toggle_keys & CORE_MACHINE_KEYBOARD_TOGGLE_CAPS_LOCK) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_A_INSERT,
        (toggle_keys & CORE_MACHINE_KEYBOARD_TOGGLE_INSERT) != 0u);
    keyboard_set_flag1(profile, QDKEYB_FLAG1_D_PAUSE,
        (toggle_keys & CORE_MACHINE_KEYBOARD_TOGGLE_PAUSE) != 0u);
}

static const core_machine_keyboard_provider keyboard_provider = {
    keyboard_get_modifier,
    keyboard_apply_host_state
};

C_VOID vm_profile_default_keyboard_reset(vm_profile_default_context *profile)
{
    if (profile != STD_NULL) profile->keyboard_waiting = TYPE_FALSE;
}

const core_machine_keyboard_provider *vm_profile_default_keyboard_provider(C_VOID)
{
    return &keyboard_provider;
}
