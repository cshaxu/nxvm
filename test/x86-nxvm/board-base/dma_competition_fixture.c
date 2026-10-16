#include "dma_competition_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"

lib_status test_dma_competition_program_channel2(x86_core_machine *machine)
{
    const struct {lib_u16 port; lib_u8 value;} program[] = {
        {0xcu, 0u}, {0x4u, 0x34u}, {0x4u, 0x12u},
        {0x5u, 0u}, {0x5u, 0u}, {0x81u, 1u},
        {0xbu, 0x46u}, {0xau, 0x02u}, {0xd4u, 0u}
    };

    for (lib_size index = 0u; index < sizeof(program) / sizeof(program[0]); ++index) {
        const lib_status status = x86_core_machine_bus_write(machine,
            program[index].port, program[index].value);
        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

lib_status test_dma_competition_create(const x86_nxvm_board_config *config,
    const x86_nxvm_board_dma_channel_provider *provider, void *source,
    x86_core_machine **out_machine, x86_nxvm_board_dma_bus **out_dma,
    x86_nxvm_board_dma_request_binding *out_binding)
{
    x86_nxvm_board_state *board = LIB_NULL;
    lib_status status;
    *out_dma = LIB_NULL;
    status = x86_nxvm_board_create(config, out_machine, &board);
    if (status != LIB_STATUS_OK) return status;
    status = x86_nxvm_board_dma_bind_channel(board->shared_dma, 2u,
        provider, source, out_binding);
    if (status != LIB_STATUS_OK) {
        x86_core_machine_destroy(*out_machine);
        *out_machine = LIB_NULL;
        return status;
    }
    *out_dma = board->shared_dma;
    return LIB_STATUS_OK;
}
