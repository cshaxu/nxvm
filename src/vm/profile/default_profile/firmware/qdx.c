/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/product/utils.h"

#include "core/machine/cpu.h"

#include "core/machine/cpu_instructions.h"


#include "vm/profile/default_profile/firmware/context.h"

#include "vm/profile/default_profile/firmware/qdcga.h"

#include "vm/profile/default_profile/firmware/qdkeyb.h"

#include "vm/profile/default_profile/firmware/qddisk.h"

#include "vm/profile/default_profile/firmware/qdx.h"

static C_VOID vm_profile_default_qdx_dispatch(
    core_machine_cpu_execution_context *execution)
{
    vm_profile_default_context *profile =
        vm_profile_default_context_from_execution(execution);
    t_cpu *cpu;
    t_cpuins *instructions;
    ntvdm64_type_unsigned_8 command_id;
    ntvdm64_type_unsigned_16 flags;

    if (profile == STD_NULL || profile->qdx == STD_NULL || execution == STD_NULL ||
        execution->cpu == STD_NULL || execution->instructions == STD_NULL) return;
    cpu = execution->cpu;
    instructions = execution->instructions;
    cpu->data.eip++;
    if (core_machine_cpu_execution_read_linear(execution,
            cpu->data.cs.base + cpu->data.eip, NTVDM64_TYPE_REFERENCE_OF(command_id), 1)) {
        STD_PRINTF("Cannot read data from L%08X.\n",
            cpu->data.cs.base + cpu->data.eip);
        core_machine_cpu_execution_request_stop(execution);
    } else {
        cpu->data.eip++;
    }
    switch (command_id) {
    case 0x00:
    case 0xff:
        STD_PRINTF("\nNXVM CPU STOP at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
            cpu->data.cs.selector, cpu->data.eip, command_id);
        STD_PRINTF("This happens because of the special instruction.\n");
        core_machine_cpu_execution_request_stop(execution);
        break;
    case 0x01:
    case 0xfe:
        STD_PRINTF("\nNXVM CPU RESET at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
            cpu->data.cs.selector, cpu->data.eip, command_id);
        STD_PRINTF("This happens because of the special instruction.\n");
        core_machine_cpu_execution_request_reset(execution);
        break;
    default:
        if (profile->qdx->table[command_id] != STD_NULL) {
            profile->qdx->table[command_id](profile);
        }
        if (command_id < 0x20) {
            if (core_machine_cpu_execution_read_linear(execution,
                    cpu->data.ss.base + cpu->data.sp + 4, NTVDM64_TYPE_REFERENCE_OF(flags), 2)) {
                STD_PRINTF("Cannot read data from L%08X.\n",
                    cpu->data.ss.base + cpu->data.sp + 4);
                core_machine_cpu_execution_request_stop(execution);
            }
            NTVDM64_TYPE_MAKE_BIT(flags, VCPU_EFLAGS_ZF, NTVDM64_TYPE_GET_BIT(cpu->data.eflags, VCPU_EFLAGS_ZF));
            NTVDM64_TYPE_MAKE_BIT(flags, VCPU_EFLAGS_CF, NTVDM64_TYPE_GET_BIT(cpu->data.eflags, VCPU_EFLAGS_CF));
            if (core_machine_cpu_execution_write_linear(execution,
                    cpu->data.ss.base + cpu->data.sp + 4, NTVDM64_TYPE_REFERENCE_OF(flags), 2)) {
                STD_PRINTF("Cannot write data to L%08X.\n",
                    cpu->data.ss.base + cpu->data.sp + 4);
                core_machine_cpu_execution_request_stop(execution);
            }
        }
        break;
    }
    instructions->data.flagIgnore = NTVDM64_TYPE_TRUE;
}

C_VOID vm_profile_default_qdx_initialize(vm_profile_default_context *profile,
    core_machine_cpu_execution_context *execution)
{
    ntvdm64_type_native_unsigned index;

    if (profile == STD_NULL || profile->qdx == STD_NULL || execution == STD_NULL ||
        execution->instructions == STD_NULL) return;
    for (index = 0; index < 0x100; ++index) {
        profile->qdx->table[index] = STD_NULL;
    }
    vm_profile_default_keyboard_initialize(profile->qdx);
    vm_profile_default_cga_initialize(profile->qdx);
    vm_profile_default_disk_initialize(profile->qdx);
    execution->instructions->connect.insTable[0xf1] =
        vm_profile_default_qdx_dispatch;
}

C_VOID vm_profile_default_qdx_reset(vm_profile_default_context *profile)
{
    vm_profile_default_cga_reset(profile);
}

C_VOID vm_profile_default_qdx_refresh(vm_profile_default_context *profile)
{
    (C_VOID)profile;
}

C_VOID vm_profile_default_qdx_finalize(vm_profile_default_context *profile)
{
    (C_VOID)profile;
}
