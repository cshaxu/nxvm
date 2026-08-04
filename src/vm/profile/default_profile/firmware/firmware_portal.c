/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"

#include "vm/profile/default_profile/firmware/context.h"
#include "vm/profile/default_profile/firmware/firmware_portal.h"
#include "vm/profile/default_profile/firmware/qdcga.h"
#include "vm/profile/default_profile/firmware/qddisk.h"
#include "vm/profile/default_profile/firmware/qdkeyb.h"

static C_VOID vm_profile_default_firmware_portal_copy_flags(
    core_machine_cpu_execution_context *execution)
{
    t_cpu *cpu;
    uint16_t flags;

    if (execution == STD_NULL || execution->cpu == STD_NULL) return;
    cpu = execution->cpu;
    if (core_machine_cpu_execution_read_linear(execution,
            cpu->data.ss.base + cpu->data.sp + 4u, TYPE_REFERENCE_OF(flags),
            sizeof(flags))) {
        core_machine_cpu_execution_request_stop(execution);
        return;
    }
    TYPE_MAKE_BIT(flags, VCPU_EFLAGS_ZF,
        TYPE_GET_BIT(cpu->data.eflags, VCPU_EFLAGS_ZF));
    TYPE_MAKE_BIT(flags, VCPU_EFLAGS_CF,
        TYPE_GET_BIT(cpu->data.eflags, VCPU_EFLAGS_CF));
    if (core_machine_cpu_execution_write_linear(execution,
            cpu->data.ss.base + cpu->data.sp + 4u, TYPE_REFERENCE_OF(flags),
            sizeof(flags))) {
        core_machine_cpu_execution_request_stop(execution);
    }
}

static C_VOID vm_profile_default_firmware_portal_dispatch(C_VOID *opaque,
    core_machine_cpu_execution_context *execution, uint8_t vector)
{
    vm_profile_default_context *profile =
        (vm_profile_default_context *)opaque;

    if (profile == STD_NULL || execution == STD_NULL) return;
    switch (vector) {
    case VM_PROFILE_DEFAULT_PORTAL_STOP:
        core_machine_cpu_execution_request_stop(execution);
        break;
    case VM_PROFILE_DEFAULT_PORTAL_KEYBOARD_IRQ1:
        vm_profile_default_keyboard_handle_irq1(profile);
        vm_profile_default_firmware_portal_copy_flags(execution);
        break;
    case VM_PROFILE_DEFAULT_PORTAL_VIDEO_INT10:
        vm_profile_default_cga_handle_int10(profile);
        vm_profile_default_firmware_portal_copy_flags(execution);
        break;
    case VM_PROFILE_DEFAULT_PORTAL_KEYBOARD_INT16:
        vm_profile_default_keyboard_handle_int16(profile);
        vm_profile_default_firmware_portal_copy_flags(execution);
        break;
    case VM_PROFILE_DEFAULT_PORTAL_HDD_READ:
        vm_profile_default_disk_handle_hdd_read(profile);
        break;
    case VM_PROFILE_DEFAULT_PORTAL_HDD_WRITE:
        vm_profile_default_disk_handle_hdd_write(profile);
        break;
    }
}

type_status vm_profile_default_firmware_portal_install(core_machine *machine,
    vm_profile_default_context *profile, uint32_t origin_linear_start,
    uint32_t origin_linear_bytes)
{
    static const uint8_t vectors[] = {
        VM_PROFILE_DEFAULT_PORTAL_STOP,
        VM_PROFILE_DEFAULT_PORTAL_KEYBOARD_IRQ1,
        VM_PROFILE_DEFAULT_PORTAL_VIDEO_INT10,
        VM_PROFILE_DEFAULT_PORTAL_KEYBOARD_INT16,
        VM_PROFILE_DEFAULT_PORTAL_HDD_READ,
        VM_PROFILE_DEFAULT_PORTAL_HDD_WRITE
    };
    core_machine_firmware_interrupt_portal portal;
    STD_SIZE_T index;

    if (machine == STD_NULL || profile == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    portal.origin_linear_start = origin_linear_start;
    portal.origin_linear_bytes = origin_linear_bytes;
    portal.provider = vm_profile_default_firmware_portal_dispatch;
    portal.context = profile;
    for (index = 0u; index < sizeof(vectors) / sizeof(vectors[0]); ++index) {
        portal.vector = vectors[index];
        if (core_machine_install_firmware_interrupt_portal(machine, &portal) !=
            TYPE_STATUS_OK) {
            return TYPE_STATUS_FAULT;
        }
    }
    return TYPE_STATUS_OK;
}
