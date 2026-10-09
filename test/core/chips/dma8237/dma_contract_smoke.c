#include "core/chips/dma8237/dma8237_interface.h"

typedef struct transfer_probe {
    lib_u32 cycles;
    lib_u32 terminals;
    lib_u8 channel;
    lib_u16 address;
    x86_dma_cycle kind;
    lib_status result;
    lib_u8 byte;
} transfer_probe;

static lib_status cycle(void *context, x86_dma_cycle kind, lib_u8 channel,
    lib_u16 address, lib_u8 *byte)
{
    transfer_probe *probe = context;
    ++probe->cycles;
    probe->channel = channel;
    probe->address = address;
    probe->kind = kind;
    if (kind == X86_DMA_MEMORY_READ) *byte = 0xa5u;
    probe->byte = *byte;
    return probe->result;
}

static void terminal(void *context, lib_u8 channel)
{
    transfer_probe *probe = context;
    ++probe->terminals;
    probe->channel = channel;
}

static lib_u16 read_word(x86_dma *dma, lib_u8 selector)
{
    lib_u8 low = 0u;
    lib_u8 high = 0u;
    x86_dma_write_register(dma, 12u, 0u);
    x86_dma_read_register(dma, selector, &low);
    x86_dma_read_register(dma, selector, &high);
    return (lib_u16)(low | (lib_u16)high << 8);
}

static void write_word(x86_dma *dma, lib_u8 selector, lib_u16 value)
{
    x86_dma_write_register(dma, 12u, 0u);
    x86_dma_write_register(dma, selector, (lib_u8)value);
    x86_dma_write_register(dma, selector, (lib_u8)(value >> 8));
}

static lib_bool first_transfer(x86_dma *dma, lib_u8 channel, lib_u8 mode,
    lib_u8 kind, lib_bool compressed, lib_bool fail)
{
    transfer_probe probe = { 0u };
    x86_dma_bus bus = { cycle, terminal, &probe };
    lib_u8 clocks = compressed ? 3u : 4u;
    lib_u8 index;
    lib_u8 status = 0u;
    lib_bool failed = LIB_FALSE;
    x86_dma_reset(dma);
    probe.result = fail ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
    write_word(dma, channel * 2u, 0x1234u);
    write_word(dma, channel * 2u + 1u, 0u);
    x86_dma_write_register(dma, 8u, compressed ? 8u : 0u);
    x86_dma_write_register(dma, 11u, (lib_u8)(mode << 6 | kind << 2 | channel));
    x86_dma_write_register(dma, 10u, channel);
    x86_dma_set_request(dma, channel, LIB_TRUE);
    failed |= x86_dma_select(dma, x86_dma_get_signals(dma).requests) != channel;
    x86_dma_grant(dma, channel, LIB_FALSE);
    for (index = 0u; index < clocks - 1u; ++index) {
        x86_dma_advance(dma, &bus);
        failed |= probe.cycles != 0u || probe.terminals != 0u;
        failed |= read_word(dma, channel * 2u) != 0x1234u;
    }
    x86_dma_advance(dma, &bus);
    failed |= probe.cycles != 1u || probe.channel != channel;
    failed |= probe.address != 0x1234u || probe.kind != (x86_dma_cycle)kind;
    failed |= read_word(dma, channel * 2u) != (fail ? 0x1234u : 0x1235u);
    failed |= read_word(dma, channel * 2u + 1u) != (fail ? 0u : 0xffffu);
    failed |= probe.terminals != (fail ? 0u : 1u);
    failed |= x86_dma_get_signals(dma).active_channel != 4u;
    x86_dma_read_register(dma, 8u, &status);
    failed |= (status & 15u) != (fail ? 0u : (1u << channel));
    x86_dma_read_register(dma, 8u, &status);
    failed |= (status & 15u) != 0u;
    x86_dma_advance(dma, &bus);
    failed |= probe.cycles != 1u;
    return failed;
}

static lib_bool memory_to_memory(x86_dma *dma)
{
    transfer_probe probe = { 0u };
    x86_dma_bus bus = { cycle, terminal, &probe };
    lib_u8 clock;
    lib_u8 value = 0u;
    lib_bool failed = LIB_FALSE;
    x86_dma_reset(dma);
    write_word(dma, 0u, 0x2345u);
    write_word(dma, 2u, 0x6789u);
    write_word(dma, 3u, 0u);
    x86_dma_write_register(dma, 8u, 1u);
    x86_dma_write_register(dma, 9u, 4u);
    failed |= x86_dma_get_signals(dma).requests != 1u;
    x86_dma_grant(dma, 0u, LIB_FALSE);
    for (clock = 0u; clock < 3u; ++clock) {
        x86_dma_advance(dma, &bus);
        failed |= probe.cycles != 0u;
    }
    x86_dma_advance(dma, &bus);
    failed |= probe.cycles != 1u || probe.kind != X86_DMA_MEMORY_READ;
    failed |= probe.channel != 0u || probe.address != 0x2345u;
    x86_dma_read_register(dma, 13u, &value);
    failed |= value != 0xa5u;
    failed |= read_word(dma, 0u) != 0x2345u || read_word(dma, 2u) != 0x6789u;
    for (clock = 0u; clock < 3u; ++clock) {
        x86_dma_advance(dma, &bus);
        failed |= probe.cycles != 1u;
    }
    x86_dma_advance(dma, &bus);
    failed |= probe.cycles != 2u || probe.kind != X86_DMA_MEMORY_WRITE;
    failed |= probe.channel != 1u || probe.address != 0x6789u || probe.byte != 0xa5u;
    failed |= read_word(dma, 0u) != 0x2346u || read_word(dma, 2u) != 0x678au;
    failed |= read_word(dma, 3u) != 0xffffu || probe.terminals != 2u;
    failed |= x86_dma_get_signals(dma).active_channel != 4u;
    x86_dma_read_register(dma, 8u, &value);
    failed |= (value & 15u) != 2u;
    return failed;
}

static lib_bool reset_and_cascade(x86_dma *dma)
{
    transfer_probe probe = { 0u };
    x86_dma_bus bus = { cycle, terminal, &probe };
    lib_u8 channel;
    lib_u8 clock;
    lib_bool failed = LIB_FALSE;
    x86_dma_reset(dma);
    /* A guest-programmed cascade slot must release without a local cycle. */
    x86_dma_write_register(dma, 11u, 0xc0u);
    x86_dma_grant(dma, 0u, LIB_FALSE);
    for (clock = 0u; clock < 4u; ++clock) x86_dma_advance(dma, &bus);
    failed |= probe.cycles != 0u || probe.terminals != 0u;
    failed |= x86_dma_get_signals(dma).active_channel != 4u;
    /* Reset clears programmed data, software requests and grants, and masks
     * every channel; re-unmasking exposes the externally asserted requests. */
    write_word(dma, 4u, 0x1234u);
    write_word(dma, 5u, 0x5678u);
    x86_dma_write_register(dma, 11u, 0x82u);
    x86_dma_write_register(dma, 9u, 6u);
    x86_dma_grant(dma, 2u, LIB_FALSE);
    x86_dma_terminate(dma);
    x86_dma_reset(dma);
    failed |= x86_dma_get_signals(dma).requests != 0u;
    failed |= x86_dma_get_signals(dma).active_channel != 4u;
    failed |= read_word(dma, 4u) != 0u || read_word(dma, 5u) != 0u;
    x86_dma_write_register(dma, 9u, 6u);
    failed |= x86_dma_get_signals(dma).requests != 0u;
    for (channel = 0u; channel < 4u; ++channel) x86_dma_set_request(dma, channel, LIB_TRUE);
    failed |= x86_dma_get_signals(dma).requests != 0u;
    x86_dma_write_register(dma, 14u, 0u);
    failed |= x86_dma_get_signals(dma).requests != 15u;
    return failed;
}

int main(void)
{
    x86_dma *dma = LIB_NULL;
    lib_u8 channel;
    lib_u8 mode;
    lib_u8 kind;
    lib_u8 compressed;
    lib_bool failed = LIB_FALSE;
    if (x86_dma_create(&dma) != LIB_STATUS_OK) return 1;
    for (channel = 0u; channel < 4u; ++channel) {
        for (mode = 0u; mode < 3u; ++mode) {
            for (kind = 0u; kind < 3u; ++kind) {
                for (compressed = 0u; compressed < 2u; ++compressed) {
                    failed |= first_transfer(dma, channel, mode, kind,
                        compressed != 0u, LIB_FALSE);
                    failed |= first_transfer(dma, channel, mode, kind,
                        compressed != 0u, LIB_TRUE);
                }
            }
        }
    }
    failed |= memory_to_memory(dma);
    failed |= reset_and_cascade(dma);
    x86_dma_destroy(dma);
    return failed ? 1 : 0;
}
