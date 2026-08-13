#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"

typedef struct core_machine_dma_fixture {
    type_unsigned_8 bytes[2];
    type_unsigned_8 next;
    C_UINT terminal_count;
} core_machine_dma_fixture;

typedef struct core_machine_dma_word_fixture {
    type_unsigned_16 words[3];
    type_unsigned_8 next;
} core_machine_dma_word_fixture;

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

static C_VOID core_machine_dma_word_fixture_read(C_VOID *owner, t_latch *latch)
{
    core_machine_dma_word_fixture *fixture =
        (core_machine_dma_word_fixture *)owner;

    if (fixture == STD_NULL || latch == STD_NULL) return;
    latch->data.word = fixture->words[fixture->next++];
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
    t_latch latch = {0};
    t_dma primary = {0};
    t_dma secondary = {0};
    t_ram memory = {0};
    t_port port;
    core_machine_dma_request_binding binding = {0};
    core_machine_dma_request_binding word_bindings[3] = {{0}};
    core_machine_dma_fixture fixture = {{0xa5u, 0x5au}, 0u, 0u};
    core_machine_dma_word_fixture word_fixture = {{0x1234u, 0x5678u,
        0x9abcu}, 0u};
    type_unsigned_8 bytes[2] = {0};
    type_unsigned_8 zeroes[2] = {0};
    type_unsigned_16 words[2] = {0};
    type_unsigned_8 channel;
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
    core_machine_dma_initialize(&latch, &primary, &secondary, &port);
    core_machine_dma_reset(&latch, &primary, &secondary);
    if (core_machine_dma_bind_channel(&latch, &primary, &secondary, 2u,
            &provider, &fixture, &binding) != TYPE_STATUS_OK ||
        core_machine_dma_bind_channel(&latch, &primary, &secondary, 5u,
            &word_provider, &word_fixture, &word_bindings[0]) != TYPE_STATUS_OK ||
        core_machine_dma_bind_channel(&latch, &primary, &secondary, 6u,
            &word_provider, &word_fixture, &word_bindings[1]) != TYPE_STATUS_OK ||
        core_machine_dma_bind_channel(&latch, &primary, &secondary, 7u,
            &word_provider, &word_fixture, &word_bindings[2]) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }

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
        STD_FPRINTF(STD_STDERR,
            "DMA first: %02x %02x tc=%u status=%02x mask=%02x req=%02x slave=%02x/%02x\n",
            bytes[0], bytes[1], fixture.terminal_count, primary.data.status,
            primary.data.mask, primary.data.request, secondary.data.status,
            secondary.data.mask);
        failed = 1;
    }
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11234u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0xa5u || bytes[1] != 0x5au ||
        fixture.terminal_count != 1u ||
        (primary.data.status & VDMA_STATUS_TC(2u)) == 0u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) == 0u) {
        STD_FPRINTF(STD_STDERR,
            "DMA second: %02x %02x tc=%u status=%02x mask=%02x\n",
            bytes[0], bytes[1], fixture.terminal_count, primary.data.status,
            primary.data.mask);
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
        STD_FPRINTF(STD_STDERR, "DMA masked: %02x status=%02x mask=%02x\n",
            bytes[0], primary.data.status, primary.data.mask);
        failed = 1;
    }
    core_machine_port_write(&port, 0x000au, 0x02u);
    core_machine_dma_request_deassert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11234u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA deassert: %02x status=%02x mask=%02x\n",
            bytes[0], primary.data.status, primary.data.mask);
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
        STD_FPRINTF(STD_STDERR, "DMA auto-init: %02x addr=%04x count=%04x mask=%02x\n",
            bytes[0], primary.data.currAddr[2], primary.data.currCount[2],
            primary.data.mask);
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
        STD_FPRINTF(STD_STDERR, "DMA decrement: %02x addr=%04x\n", bytes[0],
            primary.data.currAddr[2]);
        failed = 1;
    }

    /* Demand and single modes consume one request per grant. A second byte
     * requires a fresh device request, never an implicit block continuation. */
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
        bytes[0] != 0x11u || bytes[1] != 0u || fixture.terminal_count != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA demand first: %02x %02x tc=%u\n",
            bytes[0], bytes[1], fixture.terminal_count);
        failed = 1;
    }
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11240u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[1] != 0x22u || fixture.terminal_count != 1u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) == 0u) {
        STD_FPRINTF(STD_STDERR, "DMA demand second: %02x %02x tc=%u mask=%02x\n",
            bytes[0], bytes[1], fixture.terminal_count, primary.data.mask);
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
        bytes[0] != 0x33u || bytes[1] != 0u || fixture.terminal_count != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA single first: %02x %02x tc=%u\n",
            bytes[0], bytes[1], fixture.terminal_count);
        failed = 1;
    }
    core_machine_dma_request_assert(&primary, &secondary, &binding);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x11242u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[1] != 0x44u || fixture.terminal_count != 1u ||
        (primary.data.mask & VDMA_MASK_DRQ(2u)) == 0u) {
        STD_FPRINTF(STD_STDERR, "DMA single second: %02x %02x tc=%u mask=%02x\n",
            bytes[0], bytes[1], fixture.terminal_count, primary.data.mask);
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
    core_machine_port_write(&port, 0x0009u, 0x05u);
    core_machine_port_write(&port, 0x00d2u, 0x04u);
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x0300u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x55u || bytes[1] != 0u ||
        (primary.data.status & VDMA_STATUS_TC(0u)) != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA m2m first: %02x %02x status=%02x\n",
            bytes[0], bytes[1], primary.data.status);
        failed = 1;
    }
    core_machine_dma_advance(&latch, &primary, &secondary, &memory, 1u);
    if (core_machine_memory_read_physical(&memory, 0x0300u,
            (type_virtual_address)bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0x55u || bytes[1] != 0x66u ||
        (primary.data.status & VDMA_STATUS_TC(0u)) == 0u ||
        primary.data.request != 0u || primary.data.isr != 0u) {
        STD_FPRINTF(STD_STDERR, "DMA m2m second: %02x %02x status=%02x req=%02x isr=%02x\n",
            bytes[0], bytes[1], primary.data.status, primary.data.request,
            primary.data.isr);
        failed = 1;
    }

    /* PC/AT page ports select their documented primary/secondary channels;
     * word-controller page bit zero is not an address bit. */
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
        core_machine_port_read(&port, 0x0089u) != 0x18u ||
        core_machine_port_read(&port, 0x008au) != 0x1au ||
        core_machine_port_read(&port, 0x008bu) != 0x1au ||
        core_machine_port_read(&port, 0x008fu) != 0x1eu) {
        failed = 1;
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

done:
    core_machine_dma_finalize(&latch, &primary, &secondary);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T269:S1:DMA-GRANT:PORT:OK\n");
    STD_PRINTF("M5:T269:S4:DMA-MODES:OK\n");
    STD_PRINTF("M5:T230:S3:DMA-CHANNEL:OK\n");
    STD_PRINTF("M5:T348:S2:DMA-PORT-PAGE:OK\n");
    return 0;
}
