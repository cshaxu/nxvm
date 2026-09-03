#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"

typedef struct core_machine_dma_fixture {
    type_unsigned_8 bytes[2];
    type_unsigned_8 next;
    C_UINT terminal_count;
} core_machine_dma_fixture;

typedef struct core_machine_dma_eop_fixture {
    core_machine_dma_fixture transfer;
    t_dma *primary;
    t_dma *secondary;
    const core_machine_dma_request_binding *binding;
    type_bool terminate_on_read;
} core_machine_dma_eop_fixture;

typedef struct core_machine_dma_word_fixture {
    type_unsigned_16 words[3];
    type_unsigned_8 next;
} core_machine_dma_word_fixture;

typedef struct core_machine_dma_failure_fixture {
    C_UINT writes;
} core_machine_dma_failure_fixture;

static C_VOID core_machine_dma_fixture_read(C_VOID *owner, t_latch *latch)
{
    core_machine_dma_fixture *fixture = (core_machine_dma_fixture *)owner;

    if (fixture == STD_NULL || latch == STD_NULL) return;
    latch->data.byte = fixture->bytes[fixture->next++];
}

static C_VOID core_machine_dma_fixture_terminal(C_VOID *owner, t_latch *latch)
{
    core_machine_dma_fixture *fixture = (core_machine_dma_fixture *)owner;

    (C_VOID)latch;
    if (fixture != STD_NULL) ++fixture->terminal_count;
}

static C_VOID core_machine_dma_eop_fixture_read(C_VOID *owner, t_latch *latch)
{
    core_machine_dma_eop_fixture *fixture =
        (core_machine_dma_eop_fixture *)owner;

    if (fixture == STD_NULL || latch == STD_NULL) return;
    latch->data.byte = fixture->transfer.bytes[fixture->transfer.next++];
    if (fixture->terminate_on_read) {
        fixture->terminate_on_read = TYPE_FALSE;
        core_machine_dma_request_terminate(fixture->primary, fixture->secondary,
            fixture->binding);
    }
}

static C_VOID core_machine_dma_eop_fixture_terminal(C_VOID *owner,
    t_latch *latch)
{
    core_machine_dma_eop_fixture *fixture =
        (core_machine_dma_eop_fixture *)owner;

    (C_VOID)latch;
    if (fixture != STD_NULL) ++fixture->transfer.terminal_count;
}

static C_VOID core_machine_dma_word_fixture_read(C_VOID *owner, t_latch *latch)
{
    core_machine_dma_word_fixture *fixture =
        (core_machine_dma_word_fixture *)owner;

    if (fixture == STD_NULL || latch == STD_NULL) return;
    latch->data.word = fixture->words[fixture->next++];
}

static C_VOID core_machine_dma_failure_fixture_write(C_VOID *owner,
    t_latch *latch)
{
    core_machine_dma_failure_fixture *fixture =
        (core_machine_dma_failure_fixture *)owner;

    (C_VOID)latch;
    if (fixture != STD_NULL) ++fixture->writes;
}

static C_VOID core_machine_dma_write_channel2(t_port *port, type_unsigned_16 address,
    type_unsigned_8 page, type_unsigned_16 count, type_unsigned_8 mode)
{
    core_machine_port_write(port, 0x000cu, 0u);
    core_machine_port_write(port, 0x0004u, address & 0xffu);
    core_machine_port_write(port, 0x0004u, address >> 8);
    core_machine_port_write(port, 0x0005u, count & 0xffu);
    core_machine_port_write(port, 0x0005u, count >> 8);
    core_machine_port_write(port, 0x0081u, page);
    core_machine_port_write(port, 0x000bu, mode);
}

static C_VOID core_machine_dma_write_primary_channel(t_port *port,
    type_unsigned_8 channel, type_unsigned_16 address, type_unsigned_16 count, type_unsigned_8 mode)
{
    type_unsigned_16 address_port = (type_unsigned_16)(channel * 2u);

    core_machine_port_write(port, 0x000cu, 0u);
    core_machine_port_write(port, address_port, address & 0xffu);
    core_machine_port_write(port, address_port, address >> 8u);
    core_machine_port_write(port, (type_unsigned_16)(address_port + 1u), count & 0xffu);
    core_machine_port_write(port, (type_unsigned_16)(address_port + 1u), count >> 8u);
    core_machine_port_write(port, 0x000bu, mode);
}

static type_unsigned_16 core_machine_dma_secondary_page_port(
    type_unsigned_8 channel)
{
    static const type_unsigned_16 ports[] = {0x008fu, 0x008bu, 0x0089u,
        0x008au};

    return ports[channel];
}

static C_VOID core_machine_dma_write_secondary_channel(t_port *port,
    type_unsigned_8 channel, type_unsigned_16 address, type_unsigned_16 count,
    type_unsigned_8 page, type_unsigned_8 mode)
{
    type_unsigned_16 address_port = (type_unsigned_16)(0x00c0u + channel * 4u);

    core_machine_port_write(port, 0x00d8u, 0u);
    core_machine_port_write(port, address_port, address & 0xffu);
    core_machine_port_write(port, address_port, address >> 8u);
    core_machine_port_write(port, (type_unsigned_16)(address_port + 2u),
        count & 0xffu);
    core_machine_port_write(port, (type_unsigned_16)(address_port + 2u),
        count >> 8u);
    core_machine_port_write(port, core_machine_dma_secondary_page_port(channel),
        page);
    core_machine_port_write(port, 0x00d6u, mode);
}

static type_unsigned_16 core_machine_dma_read_pair(t_port *port,
    type_unsigned_16 clear_port, type_unsigned_16 value_port)
{
    type_unsigned_16 value;

    core_machine_port_write(port, clear_port, 0u);
    value = (type_unsigned_16)core_machine_port_read(port, value_port);
    return (type_unsigned_16)(value |
        (core_machine_port_read(port, value_port) << 8u));
}

static C_VOID core_machine_dma_advance_phases(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_ram *memory, type_unsigned_64 ticks)
{
    core_machine_dma_advance_transaction(latch, primary, secondary, memory,
        STD_NULL, ticks);
}

C_INT main(C_VOID)
{
    static const core_machine_dma_channel_provider provider = {
        core_machine_dma_fixture_read,
        STD_NULL,
        core_machine_dma_fixture_terminal
    };
    static const core_machine_dma_channel_provider word_provider = {
        core_machine_dma_word_fixture_read, STD_NULL, STD_NULL
    };
    static const core_machine_dma_channel_provider eop_provider = {
        core_machine_dma_eop_fixture_read, STD_NULL,
        core_machine_dma_eop_fixture_terminal
    };
    static const core_machine_dma_channel_provider failure_provider = {
        STD_NULL, core_machine_dma_failure_fixture_write, STD_NULL
    };
    t_latch latch = {0};
    t_dma primary = {0};
    t_dma secondary = {0};
    t_ram memory = {0};
    t_port port;
    core_machine_dma_request_binding binding = {0};
    core_machine_dma_request_binding priority_binding = {0};
    core_machine_dma_request_binding eop_binding = {0};
    core_machine_dma_request_binding failure_binding = {0};
    core_machine_dma_request_binding word_bindings[3] = {{0}};
    core_machine_dma_fixture fixture = {{0xa5u, 0x5au}, 0u, 0u};
    core_machine_dma_fixture priority_fixture = {{0x71u, 0x72u}, 0u, 0u};
    core_machine_dma_eop_fixture eop_fixture = {{{0x91u, 0x92u}, 0u, 0u},
        STD_NULL, STD_NULL, STD_NULL, TYPE_FALSE};
    core_machine_dma_failure_fixture failure_fixture = {0u};
    core_machine_dma_word_fixture word_fixture = {{0x1234u, 0x5678u,
        0x9abcu}, 0u};
    type_unsigned_8 bytes[2] = {0};
    type_unsigned_8 zeroes[2] = {0};
    type_unsigned_16 words[2] = {0};
    type_unsigned_8 channel;
    type_unsigned_16 page_port;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    if (core_machine_memory_allocate_for(&memory, 2u * 1024u * 1024u) !=
            TYPE_STATUS_OK ||
        core_machine_dma_bind_channel(&latch, &primary, &secondary, 2u,
            &provider, &fixture, &binding) != TYPE_STATUS_INVALID_STATE) {
        failed = 1;
        goto done;
    }
    core_machine_dma_initialize(&latch, &primary, &secondary, &port, 2u);
    core_machine_dma_reset(&latch, &primary, &secondary);
    if (core_machine_dma_bind_channel(&latch, &primary, &secondary, 2u,
            &provider, &fixture, &binding) != TYPE_STATUS_OK ||
        core_machine_dma_bind_channel(&latch, &primary, &secondary, 0u,
            &failure_provider, &failure_fixture, &failure_binding) != TYPE_STATUS_OK ||
        core_machine_dma_bind_channel(&latch, &primary, &secondary, 1u,
            &provider, &priority_fixture, &priority_binding) != TYPE_STATUS_OK ||
        core_machine_dma_bind_channel(&latch, &primary, &secondary, 3u,
            &eop_provider, &eop_fixture, &eop_binding) != TYPE_STATUS_OK ||
        core_machine_dma_bind_channel(&latch, &primary, &secondary, 5u,
            &word_provider, &word_fixture, &word_bindings[0]) != TYPE_STATUS_OK ||
        core_machine_dma_bind_channel(&latch, &primary, &secondary, 6u,
            &word_provider, &word_fixture, &word_bindings[1]) != TYPE_STATUS_OK ||
        core_machine_dma_bind_channel(&latch, &primary, &secondary, 7u,
            &word_provider, &word_fixture, &word_bindings[2]) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    eop_fixture.primary = &primary;
    eop_fixture.secondary = &secondary;
    eop_fixture.binding = &eop_binding;

    /* Intel's normal service is S1 -> S2 -> S3 -> S4, while TM removes
     * S3. Memory/I/O work commits only in S4. */
    core_machine_dma_write_channel2(&port, 0x1200u, 0u, 0u, 0x86u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_port_write(&port, 0x00d4u, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance_phases(&latch, &primary, &secondary, &memory, 1u);
    core_machine_dma_advance_phases(&latch, &primary, &secondary, &memory, 3u);
    if (fixture.next != 0u || primary.data.phase != VDMA_PHASE_S4) {
        failed = 1;
    }
    core_machine_dma_advance_phases(&latch, &primary, &secondary, &memory, 1u);
    if (fixture.next != 1u || primary.data.isr != 0u) {
        failed = 1;
    }
    /* Verify is a real peripheral service cycle, but never touches RAM. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    fixture.next = 0u;
    bytes[0] = 0u;
    if (core_machine_memory_write_physical(&memory, 0x11200u,
            (type_virtual_address)bytes, sizeof(bytes[0])) != TYPE_STATUS_OK) {
        failed = 1;
    }
    core_machine_dma_write_channel2(&port, 0x1200u, 0u, 0u, 0x82u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance_phases(&latch, &primary, &secondary, &memory, 5u);
    if (fixture.next != 1u || core_machine_memory_read_physical(&memory, 0x11200u,
            (type_virtual_address)bytes, sizeof(bytes[0])) != TYPE_STATUS_OK ||
        bytes[0] != 0u) {
        failed = 1;
    }
    core_machine_dma_reset(&latch, &primary, &secondary);
    fixture.next = 0u;
    core_machine_dma_write_channel2(&port, 0x1200u, 0u, 0u, 0x86u);
    core_machine_port_write(&port, 0x0008u, VDMA_COMMAND_TM);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_port_write(&port, 0x00d4u, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance_phases(&latch, &primary, &secondary, &memory, 3u);
    if (fixture.next != 0u || primary.data.phase != VDMA_PHASE_S4) {
        failed = 1;
    }
    core_machine_dma_advance_phases(&latch, &primary, &secondary, &memory, 1u);
    if (fixture.next != 1u || primary.data.isr != 0u) {
        failed = 1;
    }
    core_machine_dma_reset(&latch, &primary, &secondary);
    fixture.next = 0u;
    fixture.terminal_count = 0u;

    /* Block-mode device -> RAM: count is inclusive, but each core DMA grant
     * may expose only one byte. */
    core_machine_dma_write_channel2(&port, 0x1234u, 0x01u, 1u, 0x86u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_port_write(&port, 0x00d4u, 0x00u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11234u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0xa5u || bytes[1] != 0u || fixture.terminal_count != 0u ||
        (primary.data.status & VDMA_STATUS_TC(2u)) != 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) != 0u) {
        failed = 1;
    }
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11234u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0xa5u || bytes[1] != 0x5au ||
        fixture.terminal_count != 1u ||
        (primary.data.status & VDMA_STATUS_TC(2u)) == 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) == 0u) {
        failed = 1;
    }

    /* A masked request and an explicit deassertion cannot move guest memory. */
    fixture.next = 0u;
    bytes[0] = bytes[1] = 0u;
    if (core_machine_memory_write_physical(&memory, 0x11234u,
            (type_virtual_address)zeroes, sizeof(zeroes)) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_write_channel2(&port, 0x1234u, 0x01u, 0u, 0x46u);
    core_machine_port_write(&port, 0x000au, 0x06u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11234u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0u) {
        failed = 1;
    }
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_deassert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11234u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0u) {
        failed = 1;
    }

    /* Auto-initialize reloads the programmed address/count after terminal
     * count, so a later request uses the same guest byte again. */
    fixture.bytes[0] = 0x3cu;
    fixture.bytes[1] = 0xc3u;
    fixture.next = 0u;
    core_machine_dma_write_channel2(&port, 0x1236u, 0x01u, 0u, 0x96u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11236u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK ||
        bytes[0] != 0xc3u || primary.data.currAddr[2] != 0x1236u ||
        primary.data.currCount[2] != 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) != 0u) {
        failed = 1;
    }

    /* Address-decrement changes only the core-owned current address. */
    fixture.bytes[0] = 0x7eu;
    fixture.next = 0u;
    core_machine_dma_write_channel2(&port, 0x1238u, 0x01u, 0u, 0xa6u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11238u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK ||
        bytes[0] != 0x7eu || primary.data.currAddr[2] != 0x1237u) {
        failed = 1;
    }

    /* A held hardware DREQ remains a level until its owner deasserts it. Demand
     * and single modes therefore receive one deterministic grant per tick while
     * the request remains asserted. */
    fixture.bytes[0] = 0x11u;
    fixture.bytes[1] = 0x22u;
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_dma_write_channel2(&port, 0x1240u, 0x01u, 1u, 0x06u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_port_write(&port, 0x00d4u, 0x00u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11240u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x11u || bytes[1] != 0x22u || fixture.terminal_count != 1u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) == 0u) {
        failed = 1;
    }
    fixture.bytes[0] = 0x33u;
    fixture.bytes[1] = 0x44u;
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_dma_write_channel2(&port, 0x1242u, 0x01u, 1u, 0x46u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_port_write(&port, 0x00d4u, 0x00u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11242u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x33u || bytes[1] != 0x44u || fixture.terminal_count != 1u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) == 0u) {
        failed = 1;
    }
    /* Memory-to-memory uses the same grant boundary. Channel 0 is the
     * source request and channel 1 supplies the destination count. */
    bytes[0] = 0x55u;
    bytes[1] = 0x66u;
    if (core_machine_memory_write_physical(&memory, 0x0200u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        core_machine_memory_write_physical(&memory, 0x0300u,
            (type_virtual_address)zeroes, sizeof(zeroes)) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_dma_write_primary_channel(&port, 0u, 0x0200u, 1u, 0x80u);
    core_machine_dma_write_primary_channel(&port, 1u, 0x0300u, 1u, 0x81u);
    core_machine_port_write(&port, 0x0008u, VDMA_COMMAND_M2M);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_port_write(&port, 0x00d4u, 0u);
    core_machine_port_write(&port, 0x0009u, 0x04u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x0300u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0u || bytes[1] != 0u || primary.data.temp != 0x55u ||
        (primary.data.status & VDMA_STATUS_TC(0u)) != 0u) {
        failed = 1;
    }
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x0300u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x55u || bytes[1] != 0u ||
        (primary.data.status & VDMA_STATUS_TC(1u)) != 0u) {
        failed = 1;
    }
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    if (core_machine_memory_read_physical(&memory, 0x0300u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x55u || bytes[1] != 0x66u ||
        (primary.data.status & VDMA_STATUS_TC(1u)) == 0u ||
        primary.data.request != 0u || primary.data.isr != 0u) {
        failed = 1;
    }

    /* In M2M the source and destination each use their own address-direction
     * mode bit; channel 0's direction must not leak into channel 1. */
    bytes[0] = 0x31u;
    bytes[1] = 0x32u;
    if (core_machine_memory_write_physical(&memory, 0x0220u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        core_machine_memory_write_physical(&memory, 0x0320u,
            (type_virtual_address)zeroes, sizeof(zeroes)) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_dma_write_primary_channel(&port, 0u, 0x0221u, 1u, 0xa0u);
    core_machine_dma_write_primary_channel(&port, 1u, 0x0320u, 1u, 0x81u);
    core_machine_port_write(&port, 0x0008u, VDMA_COMMAND_M2M);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_port_write(&port, 0x00d4u, 0u);
    core_machine_port_write(&port, 0x0009u, 0x04u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 4u);
    if (core_machine_memory_read_physical(&memory, 0x0320u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x32u || bytes[1] != 0x31u ||
        primary.data.currAddr[0] != 0x021fu ||
        primary.data.currAddr[1] != 0x0322u) {
        failed = 1;
    }

    /* PC/AT page ports retain their complete readable latch; the word
     * controller ignores page bit zero only while it forms its address. */
    core_machine_port_write(&port, 0x0081u, 0x11u);
    core_machine_port_write(&port, 0x0082u, 0x12u);
    core_machine_port_write(&port, 0x0083u, 0x13u);
    core_machine_port_write(&port, 0x0087u, 0x17u);
    core_machine_port_write(&port, 0x0089u, 0x19u);
    core_machine_port_write(&port, 0x008au, 0x1au);
    core_machine_port_write(&port, 0x008bu, 0x1bu);
    core_machine_port_write(&port, 0x008fu, 0x1fu);
    if (core_machine_port_read(&port, 0x0081u) != 0x11u ||
        core_machine_port_read(&port, 0x0082u) != 0x12u ||
        core_machine_port_read(&port, 0x0083u) != 0x13u ||
        core_machine_port_read(&port, 0x0087u) != 0x17u ||
        core_machine_port_read(&port, 0x0089u) != 0x19u ||
        core_machine_port_read(&port, 0x008au) != 0x1au ||
        core_machine_port_read(&port, 0x008bu) != 0x1bu ||
        core_machine_port_read(&port, 0x008fu) != 0x1fu) {
        failed = 1;
    }
    /* IBM 5170 POST writes and immediately reads the whole page-register
     * block.  Every decoded latch must preserve all eight written bits. */
    for (page_port = 0x0080u; page_port <= 0x008fu; ++page_port) {
        type_unsigned_8 value = (type_unsigned_8)(page_port - 0x0080u);

        core_machine_port_write(&port, page_port, value);
        if (core_machine_port_read(&port, page_port) != value) failed = 1;
    }

    core_machine_dma_write_primary_channel(&port, 0u, 0x1234u, 0x5678u,
        0x84u);
    if (core_machine_dma_read_pair(&port, 0x000cu, 0x0000u) != 0x1234u ||
        core_machine_dma_read_pair(&port, 0x000cu, 0x0001u) != 0x5678u) {
        failed = 1;
    }

    /* Both controller control families retain their sparse PC/AT routes. The
     * transfer-policy consequences are owned by S3. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_port_write(&port, 0x0008u, 0x10u);
    core_machine_port_write(&port, 0x0009u, 0x06u);
    core_machine_port_write(&port, 0x000au, 0x06u);
    core_machine_port_write(&port, 0x000fu, 0x05u);
    if (primary.data.command != 0x10u || primary.data.request != 0x04u ||
        primary.data.mask != 0x05u) {
        failed = 1;
    }
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_port_write(&port, 0x000du, 0u);
    if (primary.data.command != 0u || primary.data.request != 0u ||
        primary.data.mask != VDMA_MASK_VALID) {
        failed = 1;
    }
    core_machine_port_write(&port, 0x00d0u, 0x10u);
    core_machine_port_write(&port, 0x00d2u, 0x06u);
    core_machine_port_write(&port, 0x00d4u, 0x06u);
    core_machine_port_write(&port, 0x00deu, 0x05u);
    if (secondary.data.command != 0x10u || secondary.data.request != 0x04u ||
        secondary.data.mask != 0x05u) {
        failed = 1;
    }
    core_machine_port_write(&port, 0x00dcu, 0u);
    core_machine_port_write(&port, 0x00dau, 0u);
    if (secondary.data.command != 0u || secondary.data.request != 0u ||
        secondary.data.mask != VDMA_MASK_VALID) {
        failed = 1;
    }

    /* Channels 5--7 retain the real word address layout and their sparse
     * secondary-controller ports. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_port_write(&port, 0x00dcu, 0u);
    word_fixture.next = 0u;
    for (channel = 1u; channel <= 3u; ++channel) {
        type_unsigned_32 physical = 0x20000u +
            ((type_unsigned_32)(0x0100u + channel) << 1u);

        core_machine_dma_write_secondary_channel(&port, channel,
            (type_unsigned_16)(0x0100u + channel), 0u, 0x02u,
            (type_unsigned_8)(0x84u | channel));
        if (core_machine_dma_read_pair(&port, 0x00d8u,
                (type_unsigned_16)(0x00c0u + channel * 4u)) !=
                (type_unsigned_16)(0x0100u + channel) ||
            core_machine_dma_read_pair(&port, 0x00d8u,
                (type_unsigned_16)(0x00c2u + channel * 4u)) != 0u) {
            failed = 1;
        }
        core_machine_dma_request_assert(&primary, &secondary,
            &word_bindings[channel - 1u]);
        core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
        if (core_machine_memory_read_physical(&memory, physical,
                (type_virtual_address)words, sizeof(words[0])) != TYPE_STATUS_OK ||
            words[0] != word_fixture.words[channel - 1u]) {
            failed = 1;
        }
    }

    /* Increment and decrement wrap the controller address while retaining the
     * programmed PC/AT page for byte and word channels. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_port_write(&port, 0x00d4u, 0u);
    fixture.bytes[0] = 0x41u;
    fixture.bytes[1] = 0x42u;
    fixture.next = 0u;
    core_machine_dma_write_channel2(&port, 0xffffu, 0x01u, 1u, 0x86u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    if (core_machine_memory_read_physical(&memory, 0x1ffffu,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK ||
        bytes[0] != 0x41u ||
        core_machine_memory_read_physical(&memory, 0x10000u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0x42u) {
        failed = 1;
    }

    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_port_write(&port, 0x00dcu, 0u);
    word_fixture.words[0] = 0x369cu;
    word_fixture.words[1] = 0x48adu;
    word_fixture.next = 0u;
    core_machine_dma_write_secondary_channel(&port, 1u, 0u, 0x01u, 0x02u,
        0xa5u);
    core_machine_dma_request_assert(&primary, &secondary, &word_bindings[0]);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    if (core_machine_memory_read_physical(&memory, 0x20000u,
            (type_virtual_address)words, sizeof(words[0])) != TYPE_STATUS_OK ||
        words[0] != 0x369cu ||
        core_machine_memory_read_physical(&memory, 0x3fffeu,
            (type_virtual_address)words, sizeof(words[0])) != TYPE_STATUS_OK ||
        words[0] != 0x48adu) {
        failed = 1;
    }

    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_port_write(&port, 0x00d4u, 0u);
    fixture.bytes[0] = 0x43u;
    fixture.bytes[1] = 0x44u;
    fixture.next = 0u;
    core_machine_dma_write_channel2(&port, 0u, 0x01u, 1u, 0xa6u);
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    if (core_machine_memory_read_physical(&memory, 0x10000u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK ||
        bytes[0] != 0x43u ||
        core_machine_memory_read_physical(&memory, 0x1ffffu,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0x44u) {
        failed = 1;
    }

    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_port_write(&port, 0x00dcu, 0u);
    word_fixture.words[0] = 0x1357u;
    word_fixture.words[1] = 0x2468u;
    word_fixture.next = 0u;
    core_machine_dma_write_secondary_channel(&port, 1u, 0xffffu, 0x01u,
        0x02u, 0x85u);
    core_machine_dma_request_assert(&primary, &secondary, &word_bindings[0]);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    if (core_machine_memory_read_physical(&memory, 0x3fffeu,
            (type_virtual_address)words, sizeof(words[0])) != TYPE_STATUS_OK ||
        words[0] != 0x1357u ||
        core_machine_memory_read_physical(&memory, 0x20000u,
            (type_virtual_address)words, sizeof(words[0])) != TYPE_STATUS_OK ||
        words[0] != 0x2468u) {
        failed = 1;
    }

    /* Software request bits are non-maskable, but the 8237A admits them only
     * for block mode. A primary request reaches the primary controller through
     * the secondary controller's reserved cascade channel. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    fixture.bytes[0] = 0x61u;
    fixture.bytes[1] = 0x62u;
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_write_channel2(&port, 0x1800u, 0u, 1u, 0x86u);
    core_machine_port_write(&port, 0x000au, 0x06u);
    core_machine_port_write(&port, 0x0009u, 0x06u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    if (core_machine_memory_read_physical(&memory, 0x1800u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x61u || bytes[1] != 0x62u ||
        fixture.terminal_count != 1u || primary.data.request != 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) == 0u) {
        failed = 1;
    }

    core_machine_dma_reset(&latch, &primary, &secondary);
    bytes[0] = 0u;
    fixture.bytes[0] = 0x63u;
    fixture.next = 0u;
    core_machine_dma_write_channel2(&port, 0x1810u, 0u, 0u, 0x06u);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_port_write(&port, 0x0009u, 0x06u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x1810u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0u ||
        !VDMA_GetREQUEST_DRQ(primary.data.request, 2u)) {
        failed = 1;
    }

    /* A programmed cascade slot delegates priority only: it must not invent
     * a transfer, terminal count, mask update or device completion. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_write_channel2(&port, 0x1820u, 0u, 0u, 0xc6u);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (fixture.next != 0u || fixture.terminal_count != 0u ||
        (primary.data.status & VDMA_STATUS_TC(2u)) != 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) != 0u ||
        primary.data.isr != 0u) {
        failed = 1;
    }

    /* Channel 4 is the board cascade path, not a bindable or software-forced
     * primary transfer source. */
    if (core_machine_dma_bind_channel(&latch, &primary, &secondary, 4u,
            &provider, &fixture, &binding) != TYPE_STATUS_INVALID_ARGUMENT) {
        failed = 1;
    }
    core_machine_port_write(&port, 0x00d2u, 0x04u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x1810u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0u) {
        failed = 1;
    }

    /* Fixed priority selects the lower primary channel first. Rotating
     * priority moves the last served channel behind its peer. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    fixture.bytes[0] = 0x72u;
    fixture.next = 0u;
    priority_fixture.bytes[0] = 0x71u;
    priority_fixture.next = 0u;
    core_machine_dma_write_primary_channel(&port, 1u, 0x1900u, 0u, 0x45u);
    core_machine_dma_write_primary_channel(&port, 2u, 0x1902u, 0u, 0x46u);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &priority_binding);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x1900u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0x71u ||
        core_machine_memory_read_physical(&memory, 0x1902u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0u) {
        failed = 1;
    }

    core_machine_dma_reset(&latch, &primary, &secondary);
    fixture.bytes[0] = 0x92u;
    fixture.bytes[1] = 0x93u;
    fixture.next = 0u;
    priority_fixture.bytes[0] = 0x81u;
    priority_fixture.bytes[1] = 0x82u;
    priority_fixture.next = 0u;
    core_machine_dma_write_primary_channel(&port, 1u, 0x1910u, 1u, 0x45u);
    core_machine_dma_write_primary_channel(&port, 2u, 0x1912u, 1u, 0x46u);
    core_machine_port_write(&port, 0x0008u, VDMA_COMMAND_R);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &priority_binding);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    if (core_machine_memory_read_physical(&memory, 0x1910u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0x81u ||
        core_machine_memory_read_physical(&memory, 0x1912u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0x92u) {
        failed = 1;
    }

    /* Secondary local channels retain the same rotation rule without letting
     * their word transfers escape the secondary controller. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    word_fixture.words[0] = 0xa135u;
    word_fixture.words[1] = 0xb246u;
    word_fixture.next = 0u;
    core_machine_dma_write_secondary_channel(&port, 1u, 0x0a00u, 1u, 0u,
        0x45u);
    core_machine_dma_write_secondary_channel(&port, 2u, 0x0a01u, 1u, 0u,
        0x46u);
    core_machine_port_write(&port, 0x00d0u, VDMA_COMMAND_R);
    core_machine_port_write(&port, 0x00dcu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &word_bindings[0]);
    core_machine_dma_request_assert(&primary, &secondary, &word_bindings[1]);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    if (core_machine_memory_read_physical(&memory, 0x1400u,
            (type_virtual_address)words, sizeof(words[0])) != TYPE_STATUS_OK ||
        words[0] != 0xa135u ||
        core_machine_memory_read_physical(&memory, 0x1402u,
            (type_virtual_address)words, sizeof(words[0])) != TYPE_STATUS_OK ||
        words[0] != 0xb246u) {
        failed = 1;
    }

    /* A masked primary DREQ must not become a real secondary cascade request
     * and starve an unrelated unmasked secondary channel. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    words[0] = 0u;
    fixture.next = 0u;
    word_fixture.words[0] = 0xd357u;
    word_fixture.next = 0u;
    core_machine_dma_write_channel2(&port, 0x1a20u, 0u, 0u, 0x86u);
    core_machine_dma_write_secondary_channel(&port, 1u, 0x0b20u, 0u, 0u,
        0x85u);
    core_machine_port_write(&port, 0x00dcu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_request_assert(&primary, &secondary, &word_bindings[0]);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x1640u,
            (type_virtual_address)words, sizeof(words[0])) != TYPE_STATUS_OK ||
        words[0] != 0xd357u || fixture.next != 0u ||
        !VDMA_GetSTATUS_DRQ(primary.data.status, 2u)) {
        failed = 1;
    }

    /* Either controller disable gate prevents a pending bound request from
     * publishing a device or memory transfer until software reenables it. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    bytes[0] = 0u;
    fixture.bytes[0] = 0xa1u;
    fixture.next = 0u;
    core_machine_dma_write_channel2(&port, 0x1a00u, 0u, 0u, 0x86u);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_port_write(&port, 0x0008u, VDMA_COMMAND_CTRL);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    core_machine_port_write(&port, 0x0008u, 0u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x1a00u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0xa1u) {
        failed = 1;
    }

    core_machine_dma_reset(&latch, &primary, &secondary);
    words[0] = 0u;
    word_fixture.words[0] = 0xb357u;
    word_fixture.next = 0u;
    core_machine_dma_write_secondary_channel(&port, 1u, 0x0af0u, 0u, 0u,
        0x85u);
    core_machine_port_write(&port, 0x00dcu, 0u);
    core_machine_port_write(&port, 0x0008u, VDMA_COMMAND_CTRL);
    core_machine_dma_request_assert(&primary, &secondary, &word_bindings[0]);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x15e0u,
            (type_virtual_address)words, sizeof(words[0])) != TYPE_STATUS_OK ||
        words[0] != 0xb357u) {
        failed = 1;
    }

    core_machine_dma_reset(&latch, &primary, &secondary);
    words[0] = 0u;
    word_fixture.words[0] = 0xc357u;
    word_fixture.next = 0u;
    core_machine_dma_write_secondary_channel(&port, 1u, 0x0b00u, 0u, 0u,
        0x85u);
    core_machine_port_write(&port, 0x00dcu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &word_bindings[0]);
    core_machine_port_write(&port, 0x00d0u, VDMA_COMMAND_CTRL);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    core_machine_port_write(&port, 0x00d0u, 0u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x1600u,
            (type_virtual_address)words, sizeof(words[0])) != TYPE_STATUS_OK ||
        words[0] != 0xc357u) {
        failed = 1;
    }

    /* External EOP is accepted only from the active opaque binding. It closes
     * the active service, records TC/masking, and leaves other channels alone. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    eop_fixture.transfer.bytes[0] = 0xd1u;
    eop_fixture.transfer.bytes[1] = 0xd2u;
    eop_fixture.transfer.next = 0u;
    eop_fixture.transfer.terminal_count = 0u;
    eop_fixture.terminate_on_read = TYPE_TRUE;
    core_machine_dma_write_primary_channel(&port, 3u, 0x1b00u, 2u, 0x87u);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &eop_binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    if (core_machine_memory_read_physical(&memory, 0x1b00u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0xd1u || bytes[1] != 0u ||
        eop_fixture.transfer.terminal_count != 1u ||
        (primary.data.status & VDMA_STATUS_TC(3u)) == 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(3u)) == 0u) {
        failed = 1;
    }

    core_machine_dma_reset(&latch, &primary, &secondary);
    eop_fixture.transfer.bytes[0] = 0xe1u;
    eop_fixture.transfer.next = 0u;
    eop_fixture.transfer.terminal_count = 0u;
    eop_fixture.terminate_on_read = TYPE_TRUE;
    core_machine_dma_write_primary_channel(&port, 3u, 0x1b10u, 2u, 0x97u);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &eop_binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (eop_fixture.transfer.terminal_count != 1u ||
        primary.data.currAddr[3] != 0x1b10u || primary.data.currCount[3] != 2u ||
        (primary.data.mask & VDMA_MASK_DRQ(3u)) != 0u ||
        (primary.data.status & VDMA_STATUS_TC(3u)) != 0u) {
        failed = 1;
    }

    /* A rejected physical route is a preflight failure: no provider, memory,
     * latch, address/count, request, terminal, or mask state may publish. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    fixture.bytes[0] = 0xf1u;
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_write_channel2(&port, 0x0000u, 0x20u, 0u, 0x86u);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (fixture.next != 0u || fixture.terminal_count != 0u ||
        primary.data.currAddr[2] != 0u || primary.data.currCount[2] != 0u ||
        !VDMA_GetSTATUS_DRQ(primary.data.status, 2u) ||
        primary.data.isr != 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) != 0u) {
        failed = 1;
    }

    core_machine_dma_reset(&latch, &primary, &secondary);
    failure_fixture.writes = 0u;
    core_machine_dma_write_primary_channel(&port, 0u, 0u, 0u, 0x88u);
    core_machine_port_write(&port, 0x0087u, 0x20u);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &failure_binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (failure_fixture.writes != 0u || primary.data.currAddr[0] != 0u ||
        primary.data.currCount[0] != 0u ||
        !VDMA_GetSTATUS_DRQ(primary.data.status, 0u) ||
        primary.data.isr != 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(0u)) != 0u) {
        failed = 1;
    }

    /* M2M terminates through channel 1's count. Auto-init restores both
     * participating current register pairs and leaves their mask/TC clear. */
    bytes[0] = 0x5cu;
    zeroes[0] = 0u;
    if (core_machine_memory_write_physical(&memory, 0x0210u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK ||
        core_machine_memory_write_physical(&memory, 0x0310u,
            (type_virtual_address)zeroes, 1u) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_reset(&latch, &primary, &secondary);
    priority_fixture.terminal_count = 0u;
    core_machine_dma_write_primary_channel(&port, 0u, 0x0210u, 0u, 0x90u);
    core_machine_dma_write_primary_channel(&port, 1u, 0x0310u, 0u, 0x91u);
    core_machine_port_write(&port, 0x0008u, VDMA_COMMAND_M2M);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_port_write(&port, 0x0009u, 0x04u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    if (core_machine_memory_read_physical(&memory, 0x0310u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0x5cu ||
        primary.data.currAddr[0] != 0x0210u || primary.data.currCount[0] != 0u ||
        primary.data.currAddr[1] != 0x0310u || primary.data.currCount[1] != 0u ||
        (primary.data.mask & (VDMA_MASK_DRQ(0u) | VDMA_MASK_DRQ(1u))) != 0u ||
        (primary.data.status & VDMA_STATUS_TC(0u)) != 0u ||
        (primary.data.status & VDMA_STATUS_TC(1u)) == 0u ||
        priority_fixture.terminal_count != 1u) {
        failed = 1;
    }

    /* An active channel-0 binding may terminate M2M after one committed
     * primitive. Channel 1 remains the terminal-count owner, and the second
     * source byte must remain unconsumed. */
    bytes[0] = 0x61u;
    bytes[1] = 0x62u;
    zeroes[0] = 0u;
    zeroes[1] = 0u;
    if (core_machine_memory_write_physical(&memory, 0x0220u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        core_machine_memory_write_physical(&memory, 0x0320u,
            (type_virtual_address)zeroes, sizeof(zeroes)) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_reset(&latch, &primary, &secondary);
    priority_fixture.terminal_count = 0u;
    core_machine_dma_write_primary_channel(&port, 0u, 0x0220u, 1u, 0x80u);
    core_machine_dma_write_primary_channel(&port, 1u, 0x0320u, 1u, 0x81u);
    core_machine_port_write(&port, 0x0008u, VDMA_COMMAND_M2M);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_port_write(&port, 0x0009u, 0x04u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    core_machine_dma_request_terminate(&primary, &secondary, &failure_binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x0320u,
            (type_virtual_address)zeroes, sizeof(zeroes)) != TYPE_STATUS_OK ||
        zeroes[0] != 0x61u || zeroes[1] != 0u ||
        priority_fixture.terminal_count != 1u ||
        (primary.data.status & VDMA_STATUS_TC(1u)) == 0u ||
        (primary.data.mask & (VDMA_MASK_DRQ(0u) | VDMA_MASK_DRQ(1u))) !=
            (VDMA_MASK_DRQ(0u) | VDMA_MASK_DRQ(1u)) ||
        primary.data.currAddr[0] != 0x0221u ||
        primary.data.currAddr[1] != 0x0321u) {
        failed = 1;
    }

    /* M2M validates both physical routes before it moves its temporary latch,
     * count, current addresses, or software request. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_dma_write_primary_channel(&port, 0u, 0u, 0u, 0x80u);
    core_machine_dma_write_primary_channel(&port, 1u, 0x0400u, 0u, 0x81u);
    core_machine_port_write(&port, 0x0087u, 0x20u);
    core_machine_port_write(&port, 0x0008u, VDMA_COMMAND_M2M);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_port_write(&port, 0x0009u, 0x04u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (primary.data.currAddr[0] != 0u || primary.data.currAddr[1] != 0x0400u ||
        primary.data.currCount[1] != 0u || !VDMA_GetREQUEST_DRQ(
            primary.data.request, 0u) || primary.data.isr != 0u ||
        primary.data.temp != 0u) {
        failed = 1;
    }

    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_dma_write_primary_channel(&port, 0u, 0x0410u, 0u, 0x80u);
    core_machine_dma_write_primary_channel(&port, 1u, 0u, 0u, 0x81u);
    core_machine_port_write(&port, 0x0083u, 0x20u);
    core_machine_port_write(&port, 0x0008u, VDMA_COMMAND_M2M);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_port_write(&port, 0x0009u, 0x04u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 2u);
    if (primary.data.currAddr[0] != 0x0410u || primary.data.currAddr[1] != 0u ||
        primary.data.currCount[1] != 0u || !VDMA_GetREQUEST_DRQ(
            primary.data.request, 0u) || primary.data.isr != 0u ||
        primary.data.temp != 0u) {
        failed = 1;
    }

    /* Reset clears transient controller state but does not revoke a machine
     * binding; the previously issued opaque request remains usable. */
    core_machine_dma_reset(&latch, &primary, &secondary);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_reset(&latch, &primary, &secondary);
    if (primary.connect.read_provider[2] != core_machine_dma_fixture_read ||
        primary.connect.device_owner[2] != &fixture ||
        primary.data.status != 0u || primary.data.request != 0u ||
        primary.data.isr != 0u || primary.data.acknowledged != 0u ||
        primary.data.temp != 0u ||
        primary.data.flagEOP || primary.data.mask != VDMA_MASK_VALID) {
        failed = 1;
    }
    fixture.bytes[0] = 0x6du;
    fixture.next = 0u;
    core_machine_dma_write_channel2(&port, 0x1c00u, 0u, 0u, 0x86u);
    core_machine_port_write(&port, 0x000eu, 0u);
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x1c00u,
            (type_virtual_address)bytes, 1u) != TYPE_STATUS_OK ||
        bytes[0] != 0x6du) {
        failed = 1;
    }

done:
    core_machine_dma_finalize(&latch, &primary, &secondary);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T269:S1:DMA-GRANT:PORT:OK\n");
    STD_PRINTF("M5:T269:S4:DMA-MODES:OK\n");
    STD_PRINTF("M5:T230:S3:DMA-CHANNEL:OK\n");
    STD_PRINTF("M5:T348:S2:DMA-PORT-PAGE:OK\n");
    STD_PRINTF("M5:T348:S3:DMA-REQUEST-CASCADE:OK\n");
    STD_PRINTF("M5:T348:S4:DMA-TRANSACTION-LIFECYCLE:OK\n");
    return 0;
}
