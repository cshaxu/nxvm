#include <assert.h>

#include "core/bus.h"
#include "core/machine.h"
#include "core/machine_interface.h"

static core_machine *make_machine(const lib_u8 *program, lib_size size)
{
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; lib_memory_copy(bytes + 16u, program, size);
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    return machine;
}

int main(void)
{
    core_machine *machine;
    core_run_result run;
    core_observation observation;
    lib_u8 value;
    static const lib_u8 ram_and_open_bus[] = {
        0xa9u, 0x5au, 0x8du, 0u, 0u, 0xadu, 0u, 0x08u, 0xadu, 0x20u, 0x40u
    };
    static const lib_u8 ppu_read[] = { 0xadu, 0x08u, 0x20u };
    static const lib_u8 apu_write[] = { 0xa9u, 1u, 0x8du, 0x17u, 0x40u };
    static const lib_u8 nop[] = { 0xeau };

    machine = make_machine(ram_and_open_bus, sizeof(ram_and_open_bus));
    assert(core_machine_run(machine, 3u, 20u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 3u && run.cycles == 10u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.a == 0x5au);
    assert(core_machine_peek(machine, 0u, 1u, &value) == LIB_STATUS_OK && value == 0x5au);
    {
        lib_u8 values[2] = { 0xa5u, 0x5au };

        assert(core_machine_peek(machine, 0x1fffu, 2u, values) == LIB_STATUS_UNSUPPORTED);
        assert(values[0] == 0xa5u && values[1] == 0x5au);
    }
    assert(core_machine_run(machine, 1u, 10u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 1u && run.cycles == 4u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.a == 0x40u);
    core_machine_destroy(machine);

    machine = make_machine(ppu_read, sizeof(ppu_read));
    assert(core_machine_run(machine, 1u, 10u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 1u && run.cycles == 4u);
    assert(core_machine_observe(machine, &observation) == LIB_STATUS_OK && observation.a == 0u);
    core_machine_destroy(machine);

    machine = make_machine(apu_write, sizeof(apu_write));
    assert(core_machine_run(machine, 2u, 10u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 2u && machine->apu.registers[23] == 1u);
    core_machine_destroy(machine);

    machine = make_machine(nop, sizeof(nop));
    assert(core_bus_write(machine, 0u, 0xa5u) == LIB_STATUS_OK);
    assert(core_bus_read(machine, 0x4000u, &value) == LIB_STATUS_OK && value == 0xa5u);
    core_controller_set_buttons(&machine->controller, 1u);
    assert(core_bus_write(machine, 0x4016u, 1u) == LIB_STATUS_OK);
    assert(core_bus_write(machine, 0x4016u, 0u) == LIB_STATUS_OK);
    assert(core_bus_write(machine, 0u, 0xa0u) == LIB_STATUS_OK);
    assert(core_bus_read(machine, 0x4016u, &value) == LIB_STATUS_OK && value == 0xa1u);
    assert(core_bus_write(machine, 0u, 0xa0u) == LIB_STATUS_OK);
    assert(core_bus_read(machine, 0x4017u, &value) == LIB_STATUS_OK && value == 0xa0u);
    core_machine_destroy(machine);
    return 0;
}
