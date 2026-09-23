#include <assert.h>
#include <string.h>

#include "core/bus.h"
#include "core/machine.h"
#include "core/machine_interface.h"

static core_machine *make_machine(void)
{
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    bytes[16u] = 0x58u; bytes[17u] = 0xeau;       /* CLI; NOP */
    bytes[18u] = 0xd0u; bytes[19u] = 0u;          /* BNE +0 */
    bytes[20u] = 0x78u; bytes[21u] = 0xeau;       /* SEI; NOP */
    bytes[16u + 0x1000u] = 0xeau; bytes[16u + 0x1001u] = 0xeau;
    bytes[16u + 0x1100u] = 0xeau; bytes[16u + 0x1101u] = 0xeau;
    bytes[16u + 0x3ffau] = 0u; bytes[16u + 0x3ffbu] = 0x90u;
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    bytes[16u + 0x3ffeu] = 0u; bytes[16u + 0x3fffu] = 0x91u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    return machine;
}

static void assert_entry(core_machine *machine, lib_u16 pc, lib_u16 return_pc,
    lib_u8 saved_p, lib_u8 expected_s)
{
    core_observation observation;
    lib_u8 stack[3];
    core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
    lib_u32 count = 0u;
    lib_u32 cycles = 0u;
    lib_bool serviced = LIB_FALSE;
    assert(core_machine_service_interrupt(machine, &serviced, &cycles) == LIB_STATUS_OK);
    assert(serviced && cycles == 7u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
    assert(observation.pc == pc);
    assert(observation.s == expected_s && (observation.p & 0x04u) != 0u);
    assert(core_machine_peek(machine, (lib_u16)(0x0100u | (expected_s + 1u)),
        3u, stack) == LIB_STATUS_OK);
    assert(stack[0] == saved_p && stack[1] == (lib_u8)return_pc &&
        stack[2] == (lib_u8)(return_pc >> 8u));
    assert(core_machine_trace_copy(machine, trace, CORE_MACHINE_TRACE_CAPACITY,
        &count) == LIB_STATUS_OK);
    assert(count == 7u);
    assert(trace[0].address == return_pc && trace[1].address == return_pc);
    assert(trace[2].address == (lib_u16)(0x0100u | (expected_s + 3u)) &&
        trace[2].kind == CORE_BUS_TRANSFER_WRITE &&
        trace[2].value == (lib_u8)(return_pc >> 8u));
    assert(trace[3].address == (lib_u16)(0x0100u | (expected_s + 2u)) &&
        trace[3].kind == CORE_BUS_TRANSFER_WRITE &&
        trace[3].value == (lib_u8)return_pc);
    assert(trace[4].address == (lib_u16)(0x0100u | (expected_s + 1u)) &&
        trace[4].kind == CORE_BUS_TRANSFER_WRITE &&
        trace[4].value == saved_p);
    assert(trace[5].address == (pc == 0x9000u ? 0xfffau : 0xfffeu) &&
        trace[6].address == (pc == 0x9000u ? 0xfffbu : 0xffffu));
}

int main(void)
{
    core_machine *machine;
    core_run_result run;
    lib_bool serviced;
    lib_u32 cycles;

    machine = make_machine();
    assert(core_machine_run(machine, 1u, 10u, &run) == LIB_STATUS_OK);
    assert(run.instructions == 1u && run.cycles == 2u);
    assert(core_machine_set_interrupt_lines(machine, LIB_TRUE, LIB_FALSE) == LIB_STATUS_OK);
    assert(core_machine_service_interrupt(machine, &serviced, &cycles) == LIB_STATUS_OK);
    assert(!serviced && cycles == 0u);
    assert(core_machine_run(machine, 1u, 10u, &run) == LIB_STATUS_OK);
    assert(run.instructions == 1u && run.cycles == 2u);
    assert_entry(machine, 0x9100u, 0x8002u, 0x20u, 0xfau);
    core_machine_destroy(machine);

    machine = make_machine();
    assert(core_machine_set_interrupt_lines(machine, LIB_FALSE, LIB_TRUE) == LIB_STATUS_OK);
    assert_entry(machine, 0x9000u, 0x8000u, 0x24u, 0xfau);
    assert(core_machine_run(machine, 1u, 10u, &run) == LIB_STATUS_OK);
    assert(run.instructions == 1u && run.cycles == 2u);
    assert(core_machine_set_interrupt_lines(machine, LIB_FALSE, LIB_FALSE) == LIB_STATUS_OK);
    assert(core_machine_set_interrupt_lines(machine, LIB_TRUE, LIB_TRUE) == LIB_STATUS_OK);
    assert_entry(machine, 0x9000u, 0x9001u, 0x24u, 0xf7u);
    core_machine_destroy(machine);

    machine = make_machine();
    assert(core_machine_set_interrupt_lines(machine, LIB_FALSE, LIB_TRUE) == LIB_STATUS_OK);
    assert(core_machine_run(machine, 1u, 1u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.reason == CORE_MACHINE_STOP_BUDGET &&
        run.instructions == 0u && run.cycles == 7u);
    {
        core_observation observation;
        assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK);
        assert(observation.pc == 0x9000u && observation.instructions == 0u &&
            observation.cycles == 14u);
    }
    core_machine_destroy(machine);

    /* A real CPU bus transfer advances three PPU dots.  Starting at vblank's
     * preceding dot makes the PPU raise its own line during CLI's production
     * fetch; the next interrupt service must therefore enter the NMI vector
     * without an external line injection. */
    machine = make_machine();
    machine->ppu.control = 0x80u;
    machine->ppu.scanline = 241u;
    machine->ppu.dot = 0u;
    assert(core_machine_step(machine, &cycles) == LIB_STATUS_OK && cycles == 2u);
    assert(machine->ppu_nmi_asserted && machine->nmi_pending);
    assert_entry(machine, 0x9000u, 0x8001u, 0x20u, 0xfau);
    core_machine_destroy(machine);

    machine = make_machine();
    machine->ppu.control = 0x80u;
    machine->ppu.scanline = 241u;
    machine->ppu.dot = 0u;
    assert(core_bus_read(machine, 0x2002u, &machine->data_latch) == LIB_STATUS_OK);
    assert((machine->ppu.status & 0x80u) == 0u && !machine->ppu.nmi_line &&
        !machine->ppu_nmi_asserted && !machine->nmi_pending);
    assert(core_machine_service_interrupt(machine, &serviced, &cycles) == LIB_STATUS_OK &&
        !serviced && cycles == 0u);
    core_machine_destroy(machine);

    machine = make_machine();
    machine->ppu.scanline = 241u;
    machine->ppu.dot = 0u;
    assert(core_bus_write(machine, 0x2000u, 0x80u) == LIB_STATUS_OK);
    assert((machine->ppu.status & 0x80u) != 0u && machine->ppu.nmi_line &&
        machine->ppu_nmi_asserted && machine->nmi_pending);
    assert_entry(machine, 0x9000u, 0x8000u, 0x24u, 0xfau);
    core_machine_destroy(machine);

    machine = make_machine();
    machine->ppu.status = 0x80u;
    machine->ppu.control = 0x80u;
    machine->ppu.nmi_line = LIB_TRUE;
    assert(core_machine_set_ppu_nmi_line(machine, LIB_TRUE) == LIB_STATUS_OK);
    assert(core_machine_set_interrupt_lines(machine, LIB_FALSE, LIB_TRUE) == LIB_STATUS_OK);
    assert(core_bus_write(machine, 0x2000u, 0u) == LIB_STATUS_OK);
    assert(!machine->ppu.nmi_line && !machine->ppu_nmi_asserted && machine->nmi_asserted);
    core_machine_destroy(machine);
    return 0;
}
