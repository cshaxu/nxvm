#include "lib/types/test.h"
#ifndef TEST_SHARED_DMA_FIXTURE_H
#define TEST_SHARED_DMA_FIXTURE_H
#include "x86-nxvm/board-base/dma_bus_interface.h"
#include "x86-core/machine/machine_interface.h"

static inline lib_status test_dma_core_create(x86_core_machine **out_machine)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    return x86_core_machine_neutral_create(&config, out_machine);
}

static inline lib_status test_dma_initialize(x86_nxvm_board_dma_bus **out_bus,
    x86_core_machine *machine, lib_u8 controllers)
{
    lib_status status = x86_nxvm_board_dma_initialize(out_bus, machine, controllers,
        LIB_FALSE);
    if (status == LIB_STATUS_OK)
        status = x86_core_machine_freeze_execution_providers(machine);
    if (status == LIB_STATUS_OK) status = x86_core_machine_reset(machine);
    return status;
}

static inline lib_u32 test_dma_port_read(x86_core_machine *machine, lib_u16 port)
{
    lib_u32 value;
    if (x86_core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return value;
}

static inline void test_dma_port_write(x86_core_machine *machine, lib_u16 port,
    lib_u32 value)
{
    if (x86_core_machine_bus_write(machine, port, value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}

static inline lib_u16 test_dma_register_word(x86_core_machine *machine,
    lib_bool secondary, lib_u8 selector)
{
    lib_u16 port = secondary ? (lib_u16)(0xc0u + 2u * selector) : selector;
    if (x86_core_machine_bus_write(machine, secondary ? 0xd8u : 0x0cu, 0u) !=
            LIB_STATUS_OK) lib_test_assert(LIB_FALSE);
    lib_u16 low = (lib_u16)test_dma_port_read(machine, port);
    return (lib_u16)(low | test_dma_port_read(machine, port) << 8);
}

static inline void test_dma_transfers(x86_nxvm_board_dma_bus *bus,
    x86_core_machine *machine, x86_core_machine *port_owner, lib_u64 transfers,
    lib_u8 controller_count)
{
    for (lib_u64 transfer = 0u; transfer < transfers; ++transfer) {
        lib_u16 before[16];
        lib_u8 registers = controller_count == 2u ? 16u : 8u;
        for (lib_u8 selector = 0u; selector < registers; ++selector)
            before[selector] = test_dma_register_word(port_owner, selector >= 8u, selector & 7u);
        for (lib_u8 clock = 0u; clock < 16u; ++clock) {
            lib_bool changed = LIB_FALSE;
            x86_nxvm_board_dma_advance_transaction(bus, machine, 1u);
            for (lib_u8 selector = 0u; selector < registers; ++selector)
                changed |= before[selector] != test_dma_register_word(port_owner,
                    selector >= 8u, selector & 7u);
            if (changed || (x86_nxvm_board_dma_get_signals(bus, 0u).active_channel == 4u &&
                    x86_nxvm_board_dma_get_signals(bus, 1u).active_channel == 4u)) break;
        }
    }
}
#endif
