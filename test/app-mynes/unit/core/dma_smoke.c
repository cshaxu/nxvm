#include <assert.h>
#include <string.h>

#include "core/bus.h"
#include "core/machine.h"

int main(void)
{
    lib_u8 image[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    lib_bool serviced;
    lib_u32 cycles;
    lib_u32 index;
    lib_u8 value;

    memset(image, 0, sizeof(image));
    image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1au;
    image[4] = 1u;
    image[16u + 0x3ffcu] = 0u;
    image[16u + 0x3ffdu] = 0x80u;
    image[16u + 0x3ffau] = 0u;
    image[16u + 0x3ffbu] = 0x90u;
    assert(core_machine_create(&machine, image, sizeof(image), &options) == LIB_STATUS_OK);
    machine->ppu.scanline = 240u;
    machine->ppu.dot = 340u;
    core_ppu_cpu_write(&machine->ppu, machine->cartridge, 0u, 0x80u);
    assert(core_bus_read(machine, 0u, &value) == LIB_STATUS_OK);
    assert(core_machine_service_interrupt(machine, &serviced, &cycles) == LIB_STATUS_OK);
    assert(serviced && cycles == 7u && machine->pc == 0x9000u);
    for (index = 0u; index < 256u; ++index) machine->ram[index] = (lib_u8)index;
    machine->ppu.oam_address = 0xfeu;
    assert(core_bus_write(machine, 0x4014u, 0u) == LIB_STATUS_OK);
    assert(core_machine_service_dma(machine, &serviced, &cycles) == LIB_STATUS_OK);
    assert(serviced && cycles == 513u);
    assert(machine->ppu.oam[0xfeu] == 0u && machine->ppu.oam[0xffu] == 1u);
    assert(machine->ppu.oam[0u] == 2u && machine->ppu.oam[0xfdu] == 255u);
    assert(machine->ppu.oam_address == 0xfeu);
    assert(!machine->dma_pending);
    assert(core_machine_service_dma(machine, &serviced, &cycles) == LIB_STATUS_OK);
    assert(!serviced && cycles == 0u);
    /* The halt begins on the opposite slot parity after an otherwise ordinary
     * production $4014 write; it spends the documented alignment read. */
    machine->slots = 8u;
    assert(core_bus_write(machine, 0x4014u, 0u) == LIB_STATUS_OK);
    assert(core_machine_service_dma(machine, &serviced, &cycles) == LIB_STATUS_OK);
    assert(serviced && cycles == 514u);
    /* A run budget may stop in the middle of OAM DMA; the machine retains the
     * ordered read/write phase and resumes it before executing guest code. */
    machine->slots = 0u;
    machine->ppu.scanline = 0u;
    machine->ppu.dot = 0u;
    for (index = 0u; index < 256u; ++index) machine->ram[index] = (lib_u8)(255u - index);
    assert(core_bus_write(machine, 0x4014u, 0u) == LIB_STATUS_OK);
    {
        core_run_result run;
        assert(core_machine_run(machine, 1u, 20u, &run) == LIB_STATUS_OK);
        assert(run.instructions == 0u && run.cycles == 20u && machine->dma_active &&
            machine->dma_index == 9u && machine->dma_phase == 2u &&
            machine->ppu.scanline == 0u && machine->ppu.dot == 63u);
        assert(core_machine_run(machine, 1u, 1000u, &run) == LIB_STATUS_OK);
        assert(run.instructions == 1u && !machine->dma_active &&
            machine->ppu.oam[0xfeu] == 255u && machine->ppu.oam[0xffu] == 254u);
    }
    /* A completed transfer has the same machine state whether it is serviced
     * at once or partitioned into exact run budgets. */
    {
        core_machine *full = LIB_NULL;
        core_machine *partitioned = LIB_NULL;
        static const lib_u32 budgets[] = { 100u, 100u, 100u, 100u, 100u, 13u };
        core_run_result run;

        assert(core_machine_create(&full, image, sizeof(image), &options) == LIB_STATUS_OK);
        assert(core_machine_create(&partitioned, image, sizeof(image), &options) == LIB_STATUS_OK);
        for (index = 0u; index < 256u; ++index) {
            full->ram[index] = (lib_u8)(index ^ 0xa5u);
            partitioned->ram[index] = (lib_u8)(index ^ 0xa5u);
        }
        full->ppu.scanline = partitioned->ppu.scanline = 0u;
        full->ppu.dot = partitioned->ppu.dot = 0u;
        assert(core_bus_write(full, 0x4014u, 0u) == LIB_STATUS_OK);
        assert(core_machine_service_dma(full, &serviced, &cycles) == LIB_STATUS_OK);
        assert(serviced && cycles == 513u);
        assert(core_bus_write(partitioned, 0x4014u, 0u) == LIB_STATUS_OK);
        for (index = 0u; index < sizeof(budgets) / sizeof(budgets[0]); ++index) {
            assert(core_machine_run(partitioned, 1u, budgets[index], &run) == LIB_STATUS_OK);
            assert(run.instructions == 0u && run.cycles == budgets[index]);
        }
        assert(!partitioned->dma_active && !partitioned->dma_pending);
        assert(memcmp(full->ppu.oam, partitioned->ppu.oam, sizeof(full->ppu.oam)) == 0);
        assert(full->slots == partitioned->slots && full->cycles == partitioned->cycles &&
            full->ppu.scanline == partitioned->ppu.scanline && full->ppu.dot == partitioned->ppu.dot &&
            full->pc == partitioned->pc);
        core_machine_destroy(full);
        core_machine_destroy(partitioned);
    }
    core_machine_destroy(machine);
    return 0;
}
