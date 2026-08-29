#include "type.h"

#include "core/machine/machine.h"
#include "../support/core_machine_cpu_fixture.h"

static C_VOID core_machine_dma_rtc_initialize_pic(core_machine *machine)
{
    core_machine_port_write(&machine->executor_port, 0x0020u, 0x11u);
    core_machine_port_write(&machine->executor_port, 0x0021u, 0x08u);
    core_machine_port_write(&machine->executor_port, 0x0021u, 0x04u);
    core_machine_port_write(&machine->executor_port, 0x0021u, 0x01u);
    core_machine_port_write(&machine->executor_port, 0x00a0u, 0x11u);
    core_machine_port_write(&machine->executor_port, 0x00a1u, 0x70u);
    core_machine_port_write(&machine->executor_port, 0x00a1u, 0x02u);
    core_machine_port_write(&machine->executor_port, 0x00a1u, 0x01u);
}

static C_VOID core_machine_dma_rtc_cmos_write(core_machine *machine,
    type_unsigned_8 index, type_unsigned_8 value)
{
    (C_VOID)core_machine_bus_write(machine, 0x0070u, index);
    (C_VOID)core_machine_bus_write(machine, 0x0071u, value);
}

static type_unsigned_8 core_machine_dma_rtc_cmos_read(core_machine *machine,
    type_unsigned_8 index)
{
    type_unsigned_32 value = 0u;

    (C_VOID)core_machine_bus_write(machine, 0x0070u, index);
    (C_VOID)core_machine_bus_read(machine, 0x0071u, &value);
    return (type_unsigned_8)value;
}

int main(C_VOID)
{
    core_machine_config machine_config = {0};
    core_machine_dma_wiring dma_wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_dma_wiring invalid_wiring = dma_wiring;
    core_machine_rtc_cmos_config rtc_config = {0};
    core_machine_dma_request_binding fdc_request = {0};
    core_machine_run_budget budget = {3u, 0u};
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    const type_unsigned_8 program[] = { 0x90u, 0xf4u };
    type_unsigned_8 interrupt_vector = 0u;
    C_INT nmi_masked = 0;
    C_INT interrupt_pending = 0;
    C_INT failed = 0;
    C_INT stage = 1;

    machine_config.ticks_per_instruction = 1u;
    machine_config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    rtc_config.index_port = 0x0070u;
    rtc_config.data_port = 0x0071u;
    rtc_config.irq = 8u;
    rtc_config.nmi_mask_bit = 0x80u;
    rtc_config.ticks_per_second = 1u;
    rtc_config.defaults[0].index = CORE_MACHINE_RTC_EQUIPMENT;
    rtc_config.defaults[0].value = 0x5au;
    rtc_config.default_count = 1u;

    invalid_wiring.controller_count = 1u;
    if (core_machine_create(&machine_config, &machine) != TYPE_STATUS_OK ||
        core_machine_configure_dma(machine, &invalid_wiring, &fdc_request) !=
            TYPE_STATUS_INVALID_ARGUMENT ||
        (invalid_wiring = dma_wiring, invalid_wiring.fdc_channel = 0u,
            core_machine_configure_dma(machine, &invalid_wiring, &fdc_request)) !=
            TYPE_STATUS_INVALID_ARGUMENT ||
        core_machine_configure_dma(machine, &dma_wiring, &fdc_request) !=
            TYPE_STATUS_OK ||
        core_machine_configure_rtc_cmos(machine, &rtc_config) != TYPE_STATUS_OK ||
        fdc_request.core_token == 0u || fdc_request.channel != 2u ||
        core_machine_configure_dma(machine, &dma_wiring, &fdc_request) !=
            TYPE_STATUS_INVALID_STATE ||
        test_core_machine_fixture_register_reset_mapping(machine, 0x00fffff0u,
            0x000ffff0u, 16u) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        machine->shared_dma_primary.connect.device_owner[2u] != &machine->fdc ||
        machine->shared_dma_primary.connect.device_owner[0u] != machine ||
        machine->refresh_dma_request.core_token == 0u ||
        machine->refresh_dma_request.channel != 0u ||
        machine->shared_dma_primary.data.mask != VDMA_MASK_VALID ||
        core_machine_dma_rtc_cmos_read(machine, CORE_MACHINE_RTC_EQUIPMENT) !=
            0x5au) {
        failed = 1;
        goto done;
    }

    stage = 2;
    (C_VOID)core_machine_bus_write(machine, 0x0070u, 0x80u);
    if (core_machine_get_nmi_mask(machine, &nmi_masked) != TYPE_STATUS_OK ||
        !nmi_masked) {
        failed = 1;
        goto done;
    }
    (C_VOID)core_machine_bus_write(machine, 0x0070u, 0u);
    if (core_machine_get_nmi_mask(machine, &nmi_masked) != TYPE_STATUS_OK ||
        nmi_masked) {
        failed = 1;
        goto done;
    }

    stage = 3;
    core_machine_dma_rtc_initialize_pic(machine);
    core_machine_dma_rtc_cmos_write(machine, CORE_MACHINE_RTC_REG_B,
        CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_UIE);
    if (core_machine_memory_write(machine, 0x00fffff0u, program, sizeof(program)) !=
            TYPE_STATUS_OK || core_machine_run(machine, budget, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        result.executed != 2u) {
        failed = 1;
        stage = 3;
    } else if (core_machine_dma_rtc_cmos_read(machine,
            CORE_MACHINE_RTC_SECOND) != 0x05u) {
        failed = 1;
        stage = 4;
    } else if (!(interrupt_pending = core_machine_pic_scan_interrupt(
            &machine->shared_pic_master, &machine->shared_pic_slave)) ||
        (interrupt_vector = core_machine_pic_get_interrupt(
            &machine->shared_pic_master, &machine->shared_pic_slave)) != 0x70u) {
        failed = 1;
        stage = 5;
    } else if (core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_dma_rtc_cmos_read(machine, CORE_MACHINE_RTC_EQUIPMENT) !=
        0x5au || core_machine_dma_rtc_cmos_read(machine,
            CORE_MACHINE_RTC_SECOND) != 0x05u) {
        failed = 1;
        stage = 6;
    }

done:
    if (failed && machine != STD_NULL) {
        STD_PRINTF("M5:T296:S3:DMA-RTC-AUTHORITY:DETAIL:%d C=%02x IRQ=%u asserted=%u pending=%d vector=%02x IRR=%02x/%02x\n",
            stage, machine->shared_rtc.registers[CORE_MACHINE_RTC_REG_C],
            machine->shared_rtc.irq_source.irq,
            machine->shared_rtc.irq_source.asserted,
            interrupt_pending, interrupt_vector,
            machine->shared_pic_master.data.irr, machine->shared_pic_slave.data.irr);
    }
    core_machine_destroy(machine);
    if (failed) STD_PRINTF("M5:T296:S3:DMA-RTC-AUTHORITY:FAIL:%d\n", stage);
    if (!failed) STD_PRINTF("M5:T296:S3:DMA-RTC-AUTHORITY:OK\n");
    return failed;
}
