#include "type.h"

#include "vm/profile/default_profile/pc_at_profile_private.h"

C_INT main(C_VOID)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();
    const vm_profile_default_pc_at_port_leaf *cmos_index;
    const vm_profile_default_pc_at_port_leaf *cmos_data;
    const vm_profile_default_pc_at_port_leaf *fdc_data;
    const vm_profile_default_pc_at_port_leaf *memory_control;
    const vm_profile_default_pc_at_route *fdc_route;
    const vm_profile_default_pc_at_route *aux_route;
    vm_profile_default_pc_at_cpu_contract contract;
    core_machine_config configuration;
    core_machine_controller_timing_rules timing_rules;

    if (profile == STD_NULL ||
        STD_STRCMP(profile->identity, "default-pc-at") != 0 ||
        profile->rom.linear_start != 0xfffffff0u ||
        profile->rom.physical_start != 0x000ffff0u ||
        profile->rom.bytes != 16u ||
        profile->rom.reset_segment != 0xf000u ||
        profile->rom.reset_offset != 0xfff0u ||
        profile->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        profile->fpu_profile != CORE_MACHINE_FPU_PROFILE_NONE ||
        profile->clock_plan.dma.numerator != 1u ||
        profile->clock_plan.dma.denominator != 1u ||
        profile->clock_plan.pit.numerator != 1u ||
        profile->clock_plan.pit.denominator != 4u ||
        profile->clock_plan.rtc.numerator != 1u ||
        profile->clock_plan.rtc.denominator != 1u ||
        profile->clock_plan.vadp.numerator != 1u ||
        profile->clock_plan.vadp.denominator != 1u ||
        profile->clock_plan.kbc.numerator != 1u ||
        profile->clock_plan.kbc.denominator != 1u ||
        profile->clock_plan.provider.numerator != 1u ||
        profile->clock_plan.provider.denominator != 1u ||
        profile->controller_timing_rules.pic_visibility !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK ||
        profile->controller_timing_rules.dma_clock !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK ||
        profile->controller_timing_rules.dma_service !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK ||
        profile->controller_timing_rules.pit_clock !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK ||
        profile->controller_timing_rules.rtc_clock !=
            CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK ||
        profile->kbc_typematic_initial_ticks != 0u ||
        profile->kbc_typematic_repeat_ticks != 0u ||
        profile->kbc_command_response_ticks != 0u ||
        profile->cga_text_timing.active_display_ticks != 48u ||
        profile->cga_text_timing.horizontal_blank_ticks != 8u ||
        profile->cga_text_timing.vertical_retrace_ticks != 8u ||
        profile->cmos.equipment != 0x21u ||
        profile->cmos.base_memory_kib != 0x027fu ||
        profile->cmos.fixed_disk_type != 0xf0u ||
        profile->cmos.fixed_disk_type_extended_0 != 0x2fu ||
        profile->route_count != 5u || profile->firmware_service_count != 14u ||
        !vm_profile_default_pc_at_descriptor_is_valid(profile)) return 1;

    if (!vm_profile_default_pc_at_cpu_contract_select(profile,
            CORE_MACHINE_CPU_PROFILE_DEFAULT, CORE_MACHINE_FPU_PROFILE_NONE,
            &contract) || contract.cpu_profile != profile->cpu_profile ||
        contract.fpu_profile != profile->fpu_profile ||
        contract.ticks_per_instruction != profile->ticks_per_instruction ||
        STD_MEMCMP(&contract.instruction_timing, &profile->instruction_timing,
            sizeof(contract.instruction_timing)) != 0 ||
        STD_MEMCMP(&contract.transaction_contract, &profile->transaction_contract,
            sizeof(contract.transaction_contract)) != 0 ||
        STD_MEMCMP(&contract.clock_plan, &profile->clock_plan,
            sizeof(contract.clock_plan)) != 0 ||
        STD_MEMCMP(&contract.controller_timing_rules,
            &profile->controller_timing_rules,
            sizeof(contract.controller_timing_rules)) != 0 ||
        !vm_profile_default_pc_at_cpu_contract_select(profile,
            CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_FPU_PROFILE_8087,
            &contract) || contract.cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 ||
        contract.fpu_profile != CORE_MACHINE_FPU_PROFILE_8087 ||
        vm_profile_default_pc_at_cpu_contract_select(profile,
            (core_machine_cpu_profile)0xffu, CORE_MACHINE_FPU_PROFILE_NONE,
            &contract) || vm_profile_default_pc_at_cpu_contract_select(profile,
            CORE_MACHINE_CPU_PROFILE_80386, (core_machine_fpu_profile)0xffu,
            &contract)) return 1;

    cmos_index = vm_profile_default_pc_at_port_leaf_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 0x0070u);
    cmos_data = vm_profile_default_pc_at_port_leaf_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 0x0071u);
    fdc_data = vm_profile_default_pc_at_port_leaf_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0x03f5u);
    memory_control = vm_profile_default_pc_at_port_leaf_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_MEMORY_CONTROL, 0x0092u);
    fdc_route = vm_profile_default_pc_at_route_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2);
    aux_route = vm_profile_default_pc_at_route_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_AUX_IRQ12);
    if (cmos_index == STD_NULL || cmos_index->read || !cmos_index->write ||
        cmos_data == STD_NULL || !cmos_data->read || !cmos_data->write ||
        fdc_data == STD_NULL || !fdc_data->read || !fdc_data->write ||
        memory_control == STD_NULL || !memory_control->read ||
        !memory_control->write ||
        fdc_route == STD_NULL || fdc_route->irq != 6u ||
        fdc_route->dma_channel != 2u || aux_route == STD_NULL ||
        aux_route->irq != 12u || aux_route->dma_channel !=
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL ||
        !vm_profile_default_pc_at_core_config_materialize(profile, &contract,
            &configuration, &timing_rules) || configuration.kbc_aux_absent) return 1;

    puts("M5:T208:S2:DEFAULT-PC-AT-PROFILE:OK");
    return 0;
}
