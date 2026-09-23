#include <assert.h>

#include "core/machine_interface.h"

int main(void)
{
    lib_u8 bytes[16u + 16384u];
    core_machine_options options = { 0 };
    core_machine *machine = LIB_NULL;
    core_run_result run;
    lib_u8 values[3];
    static const lib_u8 program[] = {
        0xa9u,0x11u, 0xa2u,1u, 0x95u,0x0fu,
        0xa9u,0x22u, 0x9du,0xffu,0x01u,
        0xa0u,1u, 0xa9u,0x33u, 0x99u,0xffu,0x01u,
        0xa0u,0x44u, 0x94u,0x0fu,
        0xa2u,0x55u, 0xa0u,1u, 0x96u,0x0fu,
        0xa9u,0u, 0x85u,0x20u, 0xa9u,2u, 0x85u,0x21u,
        0xa0u,1u, 0xa9u,0x66u, 0x91u,0x20u,
        0xa2u,4u, 0xa9u,0x77u, 0x81u,0x1cu
    };
    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u; lib_memory_copy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(core_machine_create(&machine, bytes, sizeof(bytes), &options) == LIB_STATUS_OK);
    assert(core_machine_run(machine, 23u, 100u, &run) == LIB_STATUS_OK);
    assert(!run.trap_valid && run.instructions == 23u);
    assert(core_machine_peek(machine, 0x10u, 1u, values) == LIB_STATUS_OK && values[0] == 0x55u);
    assert(core_machine_peek(machine, 0x0200u, 2u, values) == LIB_STATUS_OK);
    assert(values[0] == 0x77u && values[1] == 0x66u);
    core_machine_destroy(machine);
    return 0;
}
