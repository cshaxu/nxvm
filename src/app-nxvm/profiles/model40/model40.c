#include "lib/types/types_interface.h"
#include "app-nxvm/profiles/model40/model40_private.h"

#include "app-nxvm/profiles/default_profile/pc_at_profile_private.h"

static const lib_u32 vm_profile_model40_contract_ids[] = {1u};

/* D3PE identifies the battery-backed MC146818; the Model-40 standard
 * configuration has two 1.2 MiB drives, one 40 MiB fixed disk, 640 KiB of
 * conventional memory, and 1 MiB of extended memory. The relocated 384 KiB
 * of Compaq built-in memory is intentionally not reported through standard
 * CMOS size bytes.
 * Core owns each session's writable copy and derives its checksum. */
C_VOID vm_profile_model40_core_config_initialize(core_machine_config *out_config)
{
    if (out_config == LIB_NULL) return;
    *out_config = (core_machine_config) {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .cpu_80386_cr_mov_ignores_mod = LIB_TRUE,
        .a20_wrap_policy = CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB,
        .ticks_per_instruction = 1u,
        .instruction_timing = {1u, 0u, 0u, 0u, 0u, 0u},
        .transaction_contract = {
            .external_cycle_timing = {.page_bytes = 2048u, .page_miss_ticks = 2u,
                .page_hit_ticks = 0u,
                .overlap_policy = CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL,
                .first_eligible_address = 0x00000000u,
                .last_eligible_address = 0x0009ffffu},
            .external_access_wait_windows = {
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03b4u, 0x03bau, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03c0u, 0x03cfu, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x03d4u, 0x03dcu, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x07c6u, 0x07c6u, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x0bc6u, 0x0bc6u, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, 0x0fc6u, 0x0fc6u, 1u},
                {CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, 0x000a0000u,
                    0x000affffu, 1u}},
            .dma_cycle_wait_quanta = 1u,
            .dma_cycle_bus_ready_gate_enabled = LIB_TRUE,
            .cpu_cycle_bus_ready_gate_enabled = LIB_TRUE,
            .cpu_prefetch_reservation_enabled = LIB_TRUE},
        .retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC,
        /* Compaq D3PE names a 16 MHz 80386 clock; 86Box selects the same
         * rate for this DeskPro 386.  The Core elapsed axis is not proven to
         * be a processor-cycle axis, so this only enables Other-L2 macro
         * pacing and never a physical-time claim. */
        .time_axis = {CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 16000000u},
        .pic_topology = CORE_MACHINE_PIC_TOPOLOGY_CASCADED,
        .dma_controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .l1_compatibility_policy = CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS,
        .kbc_serial_delivery_ticks = 1u,
        /* The Core elapsed axis is still deterministic instruction/event
         * time, not the D3PE oscillator.  Hardware frequencies therefore
         * remain board evidence for a future physical axis; applying them
         * here would distort device progress relative to CPU execution. */
        .clock_plan = {
            .dma = {1u, 1u, 0u},
            .pit = {1u, 1u, 0u},
            /* The second 8254 is driven by DCLK.  The selected board derives
             * DCLK by dividing its approximately 10 MHz BCLK twice, while
             * the processor/memory interface is 16 MHz: 5/16. */
            .auxiliary_pit = {5u, 16u, 0u},
            .rtc = {1u, 1u, 0u},
            .vadp = {1u, 1u, 0u},
            .kbc = {1u, 1u, 0u},
            .provider = {1u, 1u, 0u}},
        .auxiliary_pit_present = LIB_TRUE,
        .auxiliary_pit_base_port = 0x0048u,
        .kbc_aux_absent = LIB_TRUE,
        /* 86Box's DeskPro P1 mask is 0xf4. Its Compaq handler resolves the
         * selected colour display to 0xb0 and sets bit 2 without an 80287,
         * so this frozen colour/no-FPU board reads 0xb4. */
        .kbc_input_port_configured = LIB_TRUE,
        .kbc_input_port = 0xb4u
    };
}

type_status vm_profile_model40_values_create(vm_profile_contract_values *out_values)
{
    vm_profile_contract_values values = {0};
    const vm_profile_contract_catalog catalog = { vm_profile_model40_contract_ids,
        sizeof(vm_profile_model40_contract_ids) /
            sizeof(vm_profile_model40_contract_ids[0]) };

    /* This is direct composition, not inheritance: reuse the shared PC/AT
     * electrical grammar, then replace Model-40-specific effective values. */
    if (out_values == LIB_NULL ||
        vm_profile_ibm_5170_values_create(0u, &values) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    values.core.id = vm_profile_model40_contract_ids[0];
    vm_profile_model40_core_config_initialize(&values.core.configuration);
    values.firmware_policy = VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BYOB;
    values.media_policy = VM_PROFILE_CONTRACT_MEDIA_POLICY_SESSION;
    values.allowed_session_options = 0u;
    if (vm_profile_contract_validate(&values, &catalog, 0u) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_values = values;
    return TYPE_STATUS_OK;
}
