/* Copyright 2012-2014 Neko. */

#include "core/product/utils.h"
#include "core/machine/cpu.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"

#include "vm/profile/default_profile/firmware/context.h"
#include "vm/profile/default_profile/firmware/qdx.h"
#include "qdkeyb.h"

static t_nubit8 keyboard_read_byte(vm_profile_default_context *profile,
    t_nubit16 offset)
{
    t_nubit8 value = 0;
    core_machine_memory_read_real_from(profile->ram, Zero16, offset, &value,
        sizeof(value));
    return value;
}

static t_nubit16 keyboard_read_word(vm_profile_default_context *profile,
    t_nubit16 offset)
{
    t_nubit16 value = 0;
    core_machine_memory_read_real_from(profile->ram, Zero16, offset, &value,
        sizeof(value));
    return value;
}

static void keyboard_write_byte(vm_profile_default_context *profile,
    t_nubit16 offset, t_nubit8 value)
{
    core_machine_memory_write_real_to(profile->ram, Zero16, offset, &value,
        sizeof(value));
}

static void keyboard_write_word(vm_profile_default_context *profile,
    t_nubit16 offset, t_nubit16 value)
{
    core_machine_memory_write_real_to(profile->ram, Zero16, offset, &value,
        sizeof(value));
}

static t_nubit16 keyboard_buffer_next(t_nubit16 pointer)
{
    return QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START +
        (pointer - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 2u) %
        (QDKEYB_VBIOS_ADDR_KEYB_BUFFER_END -
         QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 1u);
}

static t_bool keyboard_buffer_empty(vm_profile_default_context *profile)
{
    return keyboard_read_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD) ==
        keyboard_read_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL);
}

static t_bool keyboard_buffer_full(vm_profile_default_context *profile)
{
    return keyboard_buffer_next(keyboard_read_word(profile,
        QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL)) == keyboard_read_word(profile,
        QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD);
}

static t_bool keyboard_buffer_push(vm_profile_default_context *profile,
    t_nubit16 code)
{
    t_nubit16 tail;

    if (keyboard_buffer_full(profile)) return True;
    tail = keyboard_read_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL);
    keyboard_write_word(profile, tail, code);
    keyboard_write_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL,
        keyboard_buffer_next(tail));
    return False;
}

static t_nubit16 keyboard_buffer_pop(vm_profile_default_context *profile)
{
    t_nubit16 head;
    t_nubit16 result;

    if (keyboard_buffer_empty(profile)) return 0;
    head = keyboard_read_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD);
    result = keyboard_read_word(profile, head);
    keyboard_write_word(profile, QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD,
        keyboard_buffer_next(head));
    return result;
}

static t_nubit16 keyboard_buffer_peek(vm_profile_default_context *profile)
{
    return keyboard_read_word(profile, keyboard_read_word(profile,
        QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD));
}

static t_nubit8 keyboard_flag0(vm_profile_default_context *profile)
{
    return keyboard_read_byte(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0);
}

static void keyboard_set_flag0(vm_profile_default_context *profile,
    t_nubit8 mask, int enabled)
{
    t_nubit8 value = keyboard_flag0(profile);
    if (enabled) SetBit(value, mask); else ClrBit(value, mask);
    keyboard_write_byte(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0, value);
}

static void keyboard_set_flag1(vm_profile_default_context *profile,
    t_nubit8 mask, int enabled)
{
    t_nubit8 value = keyboard_read_byte(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG1);
    if (enabled) SetBit(value, mask); else ClrBit(value, mask);
    keyboard_write_byte(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG1, value);
}

static void keyboard_request_irq(vm_profile_default_context *profile)
{
    core_machine_pic_set_irq(profile->execution->pic_master,
        profile->execution->pic_slave, 0x01);
}

static void keyboard_read_input(vm_profile_default_context *profile)
{
    t_cpu *cpu = profile->execution->cpu;
    while (keyboard_buffer_empty(profile)) core_product_utils_sleep(10);
    cpu->data.ax = keyboard_buffer_pop(profile);
    keyboard_request_irq(profile);
}

static void keyboard_get_status(vm_profile_default_context *profile)
{
    t_cpu *cpu = profile->execution->cpu;
    t_nubit16 key = keyboard_buffer_peek(profile);

    if (keyboard_buffer_empty(profile)) {
        SetBit(cpu->data.eflags, VCPU_EFLAGS_ZF);
        return;
    }
    switch (key) {
    case 0x1d00:
    case 0x2a00:
    case 0x3800:
        cpu->data.ax = Zero16;
        break;
    default:
        cpu->data.ax = key;
        break;
    }
    ClrBit(cpu->data.eflags, VCPU_EFLAGS_ZF);
}

static void keyboard_int_09(vm_profile_default_context *profile)
{
    core_machine_port_write(profile->execution->port, 0x0020, 0x20);
}

static void keyboard_int_16(vm_profile_default_context *profile)
{
    t_cpu *cpu = profile->execution->cpu;

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
            ((t_nubit16)cpu->data.ch << 8) | cpu->data.cl);
        break;
    default:
        break;
    }
}

static int keyboard_get_modifier(void *context,
    core_machine_keyboard_modifier modifier)
{
    vm_profile_default_context *profile = context;
    t_nubit8 flags = keyboard_flag0(profile);

    switch (modifier) {
    case CORE_MACHINE_KEYBOARD_MODIFIER_ALT:
        return GetBit(flags, QDKEYB_FLAG0_D_ALT);
    case CORE_MACHINE_KEYBOARD_MODIFIER_CONTROL:
        return GetBit(flags, QDKEYB_FLAG0_D_CTRL);
    case CORE_MACHINE_KEYBOARD_MODIFIER_SHIFT:
        return GetBit(flags, QDKEYB_FLAG0_D_LSHIFT) ||
            GetBit(flags, QDKEYB_FLAG0_D_RSHIFT);
    case CORE_MACHINE_KEYBOARD_MODIFIER_CAPS_LOCK:
        return GetBit(flags, QDKEYB_FLAG0_A_CAPLCK);
    case CORE_MACHINE_KEYBOARD_MODIFIER_NUM_LOCK:
        return GetBit(flags, QDKEYB_FLAG0_A_NUMLCK);
    }
    return False;
}

static void keyboard_apply_host_state(void *context,
    uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    vm_profile_default_context *profile = context;

    keyboard_set_flag0(profile, QDKEYB_FLAG0_D_RSHIFT,
        (asynchronous_keys & NXVM_KEYBOARD_ASYNC_RIGHT_SHIFT) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_D_LSHIFT,
        (asynchronous_keys & NXVM_KEYBOARD_ASYNC_LEFT_SHIFT) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_D_CTRL,
        (asynchronous_keys & NXVM_KEYBOARD_ASYNC_CONTROL) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_D_ALT,
        (asynchronous_keys & NXVM_KEYBOARD_ASYNC_ALT) != 0u);
    keyboard_set_flag1(profile, QDKEYB_FLAG1_D_SCRLCK,
        (asynchronous_keys & NXVM_KEYBOARD_ASYNC_SCROLL_LOCK) != 0u);
    keyboard_set_flag1(profile, QDKEYB_FLAG1_D_NUMLCK,
        (asynchronous_keys & NXVM_KEYBOARD_ASYNC_NUM_LOCK) != 0u);
    keyboard_set_flag1(profile, QDKEYB_FLAG1_D_CAPLCK,
        (asynchronous_keys & NXVM_KEYBOARD_ASYNC_CAPS_LOCK) != 0u);
    keyboard_set_flag1(profile, QDKEYB_FLAG1_D_INSERT,
        (asynchronous_keys & NXVM_KEYBOARD_ASYNC_INSERT) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_A_SCRLCK,
        (toggle_keys & NXVM_KEYBOARD_TOGGLE_SCROLL_LOCK) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_A_NUMLCK,
        (toggle_keys & NXVM_KEYBOARD_TOGGLE_NUM_LOCK) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_A_CAPLCK,
        (toggle_keys & NXVM_KEYBOARD_TOGGLE_CAPS_LOCK) != 0u);
    keyboard_set_flag0(profile, QDKEYB_FLAG0_A_INSERT,
        (toggle_keys & NXVM_KEYBOARD_TOGGLE_INSERT) != 0u);
    keyboard_set_flag1(profile, QDKEYB_FLAG1_D_PAUSE,
        (toggle_keys & NXVM_KEYBOARD_TOGGLE_PAUSE) != 0u);
}

static void keyboard_receive_key_press(void *context, uint16_t code)
{
    vm_profile_default_context *profile = context;
    (void)keyboard_buffer_push(profile, code);
    keyboard_request_irq(profile);
}

static const core_machine_keyboard_provider keyboard_provider = {
    keyboard_get_modifier,
    keyboard_apply_host_state,
    keyboard_receive_key_press
};

void vm_profile_default_keyboard_initialize(t_qdx *qdx)
{
    if (qdx == NULL) return;
    qdx->table[0x09] = keyboard_int_09;
    qdx->table[0x16] = keyboard_int_16;
}

const core_machine_keyboard_provider *vm_profile_default_keyboard_provider(void)
{
    return &keyboard_provider;
}
