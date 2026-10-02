#ifndef TEST_NXVM_DMA_FIXTURE_H
#define TEST_NXVM_DMA_FIXTURE_H
#include "port_owner_fixture.h"

/* Keep the legacy port-only chip fixtures synthetic while exercising DMA's
 * single production registration path through a Core-owned port table. */
static inline lib_status test_dma_initialize(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_port *port, lib_u8 controller_count)
{
    core_machine *machine = test_port_owner_open(port);
    lib_status status;

    if (machine == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = core_machine_dma_initialize(latch, primary, secondary,
        machine, controller_count);
    test_port_owner_close(port, machine);
    return status;
}

/* With no valid software request, assay which external DREQ inputs are
 * blocked. Restore their original levels without advancing the chip. Like
 * any architectural status read, this consumes the TC status bits. */
static inline lib_u8 test_dma_blocked_inputs(x86_dma *dma)
{
    lib_u8 status = 0u;
    lib_u8 channel;
    lib_u8 blocked;
    x86_dma_read_register(dma, 8u, &status);
    for (channel = 0u; channel < 4u; ++channel) x86_dma_set_request(dma, channel, LIB_TRUE);
    blocked = (lib_u8)(~x86_dma_get_signals(dma).requests & 15u);
    for (channel = 0u; channel < 4u; ++channel) {
        x86_dma_set_request(dma, channel, (status & (0x10u << channel)) != 0u);
    }
    return blocked;
}

static lib_u16 test_dma_register_word(t_port *port, lib_bool secondary,
    lib_u8 selector)
{
    lib_u16 address = secondary ? (lib_u16)(0xc0u + 2u * selector) : selector;
    lib_u16 low;
    core_machine_port_write(port, secondary ? 0xd8u : 0x0cu, 0u);
    low = (lib_u16)core_machine_port_read(port, address);
    return (lib_u16)(low | core_machine_port_read(port, address) << 8);
}

/* Advance the real phase path until one observable transfer or release.
 * These unit fixtures own the idle CPU's port bus. Reading address/count here
 * is intentional; unlike boot diagnostics it may reset the byte flip-flop.
 * M2M half-cycle tests use explicit clock counts instead of this helper. */
static inline void test_dma_transfers(t_latch *latch, t_dma *primary, t_dma *secondary,
    core_machine *machine, t_port *port, lib_u64 transfers)
{
    lib_u64 transfer;
    for (transfer = 0u; transfer < transfers; ++transfer) {
        lib_u16 before[16];
        lib_u8 selector;
        lib_u8 clock;
        lib_u8 registers = secondary->device != LIB_NULL ? 16u : 8u;
        for (selector = 0u; selector < registers; ++selector) {
            before[selector] = test_dma_register_word(port, selector >= 8u, selector & 7u);
        }
        for (clock = 0u; clock < 16u; ++clock) {
            lib_bool changed = LIB_FALSE;
            core_machine_dma_advance_transaction(latch, primary, secondary,
                machine, 1u);
            for (selector = 0u; selector < registers; ++selector) {
                changed |= before[selector] != test_dma_register_word(port,
                    selector >= 8u, selector & 7u);
            }
            if (changed || (x86_dma_get_signals(primary->device).active_channel == 4u &&
                    (secondary->device == LIB_NULL ||
                     x86_dma_get_signals(secondary->device).active_channel == 4u))) break;
        }
    }
}
#endif
