#include <assert.h>

#include "core/apu.h"
#include "core/bus.h"
#include "core/machine.h"

static core_machine *make_machine(void)
{
    lib_u8 image[16u + 16384u];
    core_machine *machine = LIB_NULL;
    core_machine_options options = { 0 };

    lib_memory_set(image, 0, sizeof(image));
    image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1au;
    image[4] = 1u;
    image[16u + 0x3ffcu] = 0u;
    image[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, image, sizeof(image), &options) == LIB_STATUS_OK);
    return machine;
}

int main(void)
{
    core_machine *machine = make_machine();
    core_machine *dmc_machine;
    core_apu apu;
    core_run_result run;
    lib_i32 samples[CORE_APU_SAMPLE_CAPACITY];
    lib_u16 address;
    lib_u16 silent_count;
    lib_u8 value;
    lib_u32 index;

    assert(core_bus_write(machine, 0x4015u, 1u) == LIB_STATUS_OK);
    assert(core_bus_write(machine, 0x4000u, 0x3fu) == LIB_STATUS_OK);
    assert(core_bus_write(machine, 0x4002u, 16u) == LIB_STATUS_OK);
    assert(core_bus_write(machine, 0x4003u, 0xf8u) == LIB_STATUS_OK);
    assert(core_bus_read(machine, 0x4015u, &value) == LIB_STATUS_OK && (value & 1u) != 0u);

    core_apu_reset(&apu);
    for (index = 0u; index < 2000u; ++index) core_apu_tick(&apu);
    silent_count = core_apu_take_samples(&apu, samples, CORE_APU_SAMPLE_CAPACITY);
    assert(silent_count != 0u);
    for (index = 0u; index < silent_count; ++index) assert(samples[index] == 0);
    core_apu_reset(&apu);
    core_apu_cpu_write(&apu, 21u, 1u);
    core_apu_cpu_write(&apu, 0u, 0u);
    core_apu_cpu_write(&apu, 3u, 0xf8u);
    for (index = 0u; index < 7457u; ++index) core_apu_tick(&apu);
    assert(!apu.envelope_start[0] && apu.envelope_decay[0] == 15u);
    for (index = 0u; index < 7457u; ++index) core_apu_tick(&apu);
    assert(apu.envelope_decay[0] == 14u);
    core_apu_reset(&apu);
    core_apu_cpu_write(&apu, 21u, 1u);
    core_apu_cpu_write(&apu, 2u, 100u);
    core_apu_cpu_write(&apu, 3u, 0u);
    core_apu_cpu_write(&apu, 1u, 0x81u);
    for (index = 0u; index < 14913u; ++index) core_apu_tick(&apu);
    assert(apu.pulse_timer[0] == 150u);
    core_apu_reset(&apu);
    core_apu_cpu_write(&apu, 21u, 4u);
    core_apu_cpu_write(&apu, 8u, 1u);
    core_apu_cpu_write(&apu, 11u, 0u);
    for (index = 0u; index < 7457u; ++index) core_apu_tick(&apu);
    assert(apu.triangle_linear == 1u);
    for (index = 0u; index < 7457u; ++index) core_apu_tick(&apu);
    assert(apu.triangle_linear == 0u);

    for (index = 0u; index < 20000u; ++index) core_apu_tick(&machine->apu);
    assert(machine->apu.sample_count == CORE_APU_SAMPLE_CAPACITY);
    assert(machine->apu.dropped_samples != 0u);
    assert(core_apu_take_samples(&machine->apu, samples, CORE_APU_SAMPLE_CAPACITY) ==
        CORE_APU_SAMPLE_CAPACITY);
    for (index = 0u; index < CORE_APU_SAMPLE_CAPACITY; ++index)
        assert(samples[index] >= -32768 && samples[index] <= 32767);
    for (index = 1u; index < CORE_APU_SAMPLE_CAPACITY; ++index)
        if (samples[index] != samples[0]) break;
    assert(index != CORE_APU_SAMPLE_CAPACITY);

    assert(core_bus_write(machine, 0x4017u, 0u) == LIB_STATUS_OK);
    for (index = 0u; index < 29829u; ++index) core_apu_tick(&machine->apu);
    assert(core_apu_irq_asserted(&machine->apu));
    core_machine_refresh_apu_irq(machine);
    assert(machine->irq_asserted);
    assert(core_bus_read(machine, 0x4015u, &value) == LIB_STATUS_OK && (value & 0x40u) != 0u);
    core_machine_refresh_apu_irq(machine);
    assert(!machine->apu_irq_asserted);

    assert(core_bus_write(machine, 0x4010u, 0x8fu) == LIB_STATUS_OK);
    assert(core_bus_write(machine, 0x4012u, 0u) == LIB_STATUS_OK);
    assert(core_bus_write(machine, 0x4013u, 0u) == LIB_STATUS_OK);
    assert(core_bus_write(machine, 0x4015u, 0x11u) == LIB_STATUS_OK);
    for (index = 0u; index < 428u; ++index) core_apu_tick(&machine->apu);
    assert(core_apu_take_dmc_request(&machine->apu, &address) && address == 0xc000u);
    core_apu_complete_dmc_read(&machine->apu, 1u);
    assert(core_apu_irq_asserted(&machine->apu));
    assert(core_bus_read(machine, 0x4015u, &value) == LIB_STATUS_OK && (value & 0x80u) != 0u);
    core_machine_destroy(machine);

    dmc_machine = make_machine();
    assert(core_bus_write(dmc_machine, 0x4012u, 0u) == LIB_STATUS_OK);
    assert(core_bus_write(dmc_machine, 0x4013u, 0u) == LIB_STATUS_OK);
    assert(core_bus_write(dmc_machine, 0x4015u, 0x10u) == LIB_STATUS_OK);
    for (index = 0u; index < 428u; ++index) core_apu_tick(&dmc_machine->apu);
    assert(dmc_machine->apu.dmc_dma_requested);
    assert(core_machine_run(dmc_machine, 1u, 10u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid);
    assert(run.cycles >= 4u);
    assert(!dmc_machine->dmc_dma_active && dmc_machine->apu.dmc_bytes_remaining == 0u);
    core_machine_destroy(dmc_machine);
    return 0;
}
