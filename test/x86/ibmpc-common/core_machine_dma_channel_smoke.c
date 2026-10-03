#include "dma_fixture.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86/ibmpc-common/dma_bus.h"
#include "x86/core/memory_interface.h"

static lib_bool dma_fail_allocation;
static void *dma_test_allocate_zero(lib_size count, lib_size bytes)
{
    return dma_fail_allocation ? LIB_NULL : lib_allocate_zero(count, bytes);
}
/* Same DMA-owner fault seam; no Core-private or production test API. */
#define lib_allocate_zero dma_test_allocate_zero
#include "x86/ibmpc-common/dma_bus.c"
#undef lib_allocate_zero

/* Same-owner signal assay. Restore external DREQ levels and preserve the
 * original status-read side effect of consuming terminal-count bits. */
static lib_u8 test_dma_blocked_inputs(x86_dma *dma)
{
    lib_u8 status = 0u;
    x86_dma_read_register(dma, 8u, &status);
    for (lib_u8 channel = 0u; channel < 4u; ++channel)
        x86_dma_set_request(dma, channel, LIB_TRUE);
    lib_u8 blocked = (lib_u8)(~x86_dma_get_signals(dma).requests & 15u);
    for (lib_u8 channel = 0u; channel < 4u; ++channel)
        x86_dma_set_request(dma, channel, (status & (0x10u << channel)) != 0u);
    return blocked;
}


typedef struct core_machine_dma_fixture {
    lib_u8 bytes[2];
    lib_u8 next;
    lib_u32 terminal_count;
} core_machine_dma_fixture;

typedef struct core_machine_dma_eop_fixture {
    core_machine_dma_fixture transfer;
    core_machine_dma_bus *bus;
    const core_machine_dma_request_binding *binding;
    lib_u8 terminate_on_read;
} core_machine_dma_eop_fixture;

typedef struct core_machine_dma_word_fixture {
    lib_u16 words[3];
    lib_u8 next;
} core_machine_dma_word_fixture;

typedef struct core_machine_dma_failure_fixture {
    lib_u32 writes;
} core_machine_dma_failure_fixture;

static void core_machine_dma_fixture_read(void *owner, t_latch *latch)
{
    core_machine_dma_fixture *fixture = (core_machine_dma_fixture *)owner;

    if (fixture == LIB_NULL || latch == LIB_NULL) return;
    latch->data.byte = fixture->bytes[fixture->next++];
}

static void core_machine_dma_fixture_terminal(void *owner, t_latch *latch)
{
    core_machine_dma_fixture *fixture = (core_machine_dma_fixture *)owner;

    (void)latch;
    if (fixture != LIB_NULL) ++fixture->terminal_count;
}

static void core_machine_dma_eop_fixture_read(void *owner, t_latch *latch)
{
    core_machine_dma_eop_fixture *fixture =
        (core_machine_dma_eop_fixture *)owner;

    if (fixture == LIB_NULL || latch == LIB_NULL) return;
    latch->data.byte = fixture->transfer.bytes[fixture->transfer.next++];
    if (fixture->terminate_on_read) {
        fixture->terminate_on_read = LIB_FALSE;
        core_machine_dma_request_terminate(fixture->bus,
            fixture->binding);
    }
}

static void core_machine_dma_eop_fixture_terminal(void *owner,
    t_latch *latch)
{
    core_machine_dma_eop_fixture *fixture =
        (core_machine_dma_eop_fixture *)owner;

    (void)latch;
    if (fixture != LIB_NULL) ++fixture->transfer.terminal_count;
}

static void core_machine_dma_word_fixture_read(void *owner, t_latch *latch)
{
    core_machine_dma_word_fixture *fixture =
        (core_machine_dma_word_fixture *)owner;

    if (fixture == LIB_NULL || latch == LIB_NULL) return;
    latch->data.word = fixture->words[fixture->next++];
}

static void core_machine_dma_failure_fixture_write(void *owner,
    t_latch *latch)
{
    core_machine_dma_failure_fixture *fixture =
        (core_machine_dma_failure_fixture *)owner;

    (void)latch;
    if (fixture != LIB_NULL) ++fixture->writes;
}

static void core_machine_dma_write_channel2(core_machine *port, lib_u16 address,
    lib_u8 page, lib_u16 count, lib_u8 mode)
{
    core_machine_bus_write(port, 0x000cu, 0u);
    core_machine_bus_write(port, 0x0004u, address & 0xffu);
    core_machine_bus_write(port, 0x0004u, address >> 8);
    core_machine_bus_write(port, 0x0005u, count & 0xffu);
    core_machine_bus_write(port, 0x0005u, count >> 8);
    core_machine_bus_write(port, 0x0081u, page);
    core_machine_bus_write(port, 0x000bu, mode);
}

static void core_machine_dma_write_primary_channel(core_machine *port,
    lib_u8 channel, lib_u16 address, lib_u16 count, lib_u8 mode)
{
    lib_u16 address_port = (lib_u16)(channel * 2u);

    core_machine_bus_write(port, 0x000cu, 0u);
    core_machine_bus_write(port, address_port, address & 0xffu);
    core_machine_bus_write(port, address_port, address >> 8u);
    core_machine_bus_write(port, (lib_u16)(address_port + 1u), count & 0xffu);
    core_machine_bus_write(port, (lib_u16)(address_port + 1u), count >> 8u);
    core_machine_bus_write(port, 0x000bu, mode);
}

static lib_u16 core_machine_dma_secondary_page_port(
    lib_u8 channel)
{
    static const lib_u16 ports[] = {0x008fu, 0x008bu, 0x0089u,
        0x008au};

    return ports[channel];
}

static void core_machine_dma_write_secondary_channel(core_machine *port,
    lib_u8 channel, lib_u16 address, lib_u16 count,
    lib_u8 page, lib_u8 mode)
{
    lib_u16 address_port = (lib_u16)(0x00c0u + channel * 4u);

    core_machine_bus_write(port, 0x00d8u, 0u);
    core_machine_bus_write(port, address_port, address & 0xffu);
    core_machine_bus_write(port, address_port, address >> 8u);
    core_machine_bus_write(port, (lib_u16)(address_port + 2u),
        count & 0xffu);
    core_machine_bus_write(port, (lib_u16)(address_port + 2u),
        count >> 8u);
    core_machine_bus_write(port, core_machine_dma_secondary_page_port(channel),
        page);
    core_machine_bus_write(port, 0x00d6u, mode);
}

static lib_u16 core_machine_dma_read_pair(core_machine *port,
    lib_u16 clear_port, lib_u16 value_port)
{
    lib_u16 value;

    core_machine_bus_write(port, clear_port, 0u);
    value = (lib_u16)test_dma_port_read(port, value_port);
    return (lib_u16)(value |
        (test_dma_port_read(port, value_port) << 8u));
}

static void core_machine_dma_advance_phases(core_machine_dma_bus *bus, core_machine *machine, lib_u64 ticks)
{
    core_machine_dma_advance_transaction(bus, machine,
        ticks);
}

typedef struct core_machine_dma_phase_fixture {
    lib_u32 reads;
    lib_u32 writes;
    lib_u32 terminals;
    lib_u16 last_write;
} core_machine_dma_phase_fixture;

static void core_machine_dma_phase_read(void *owner, t_latch *latch)
{
    core_machine_dma_phase_fixture *fixture = owner;
    ++fixture->reads;
    latch->data.word = 0xa55au;
}

static void core_machine_dma_phase_write(void *owner, t_latch *latch)
{
    core_machine_dma_phase_fixture *fixture = owner;
    ++fixture->writes;
    fixture->last_write = latch->data.word;
}

static void core_machine_dma_phase_terminal(void *owner, t_latch *latch)
{
    core_machine_dma_phase_fixture *fixture = owner;
    (void)latch;
    ++fixture->terminals;
}

static lib_status dma_conflicting_port(void *owner, lib_u16 address, lib_u32 value)
{
    (void)address;
    (void)value;
    *(lib_u8 *)owner = 0x5au;
    return LIB_STATUS_OK;
}

static lib_bool dma_construction_rollback(void)
{
    core_machine_dma_bus *bus = LIB_NULL;
    core_machine *machine = LIB_NULL;
    lib_u8 marker = 0u;
    lib_u32 value = 0u;
    const core_machine_port_route route = {
        .address = 0xd4u, .write = dma_conflicting_port, .owner = &marker
    };
    lib_bool failed = LIB_FALSE;
    if (test_dma_core_create(&machine) != LIB_STATUS_OK) return LIB_TRUE;
    failed |= core_machine_install_port_routes(machine, &route, 1u) != LIB_STATUS_OK;
    failed |= core_machine_dma_initialize(&bus, machine, 2u) != LIB_STATUS_INVALID_STATE;
    failed |= bus != LIB_NULL;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_bus_read(machine, 0u, &value) != LIB_STATUS_UNSUPPORTED;
    failed |= core_machine_bus_write(machine, 0u, 0u) != LIB_STATUS_UNSUPPORTED;
    failed |= core_machine_bus_write(machine, 0x81u, 0u) != LIB_STATUS_UNSUPPORTED;
    failed |= core_machine_bus_write(machine, 0xd4u, 0u) != LIB_STATUS_OK;
    failed |= marker != 0x5au;
    core_machine_destroy(machine);
    machine = LIB_NULL;
    if (test_dma_core_create(&machine) != LIB_STATUS_OK) return LIB_TRUE;
    dma_fail_allocation = LIB_TRUE;
    failed |= core_machine_dma_initialize(&bus, machine, 2u) != LIB_STATUS_NO_MEMORY;
    failed |= bus != LIB_NULL;
    dma_fail_allocation = LIB_FALSE;
    failed |= test_dma_initialize(&bus, machine, 2u) != LIB_STATUS_OK;
    core_machine_destroy(machine);
    core_machine_dma_finalize(bus);
    return failed;
}

static lib_i32 core_machine_dma_first_service_matrix(void)
{
    static const core_machine_dma_channel_provider provider = {
        core_machine_dma_phase_read, core_machine_dma_phase_write,
        core_machine_dma_phase_terminal
    };
    static const lib_u8 channels[] = {0u, 1u, 2u, 3u, 5u, 6u, 7u};
    core_machine_dma_bus *bus = LIB_NULL;
    core_machine *machine = LIB_NULL;


    core_machine_dma_request_binding bindings[7] = {{0}};
    core_machine_dma_phase_fixture fixture = {0};
    lib_u32 row;
    lib_i32 failed = 0;
    if (test_dma_core_create(&machine) !=
            LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    if (test_dma_initialize(&bus, machine, 2u) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    for (row = 0u; row < 7u; ++row) {
        if (core_machine_dma_bind_channel(bus,
                channels[row], &provider, &fixture, &bindings[row]) != LIB_STATUS_OK) {
            failed = 1;
            goto done;
        }
    }
    /* Seven channels x normal/TM x demand/single/block x verify/write/read.
     * Use the real clock-phase entry, not the accelerated fixture helper. */
    for (row = 0u; row < 126u; ++row) {
        lib_u8 index = (lib_u8)(row / 18u);
        lib_u8 channel = channels[index] & 3u;
        lib_bool word = channels[index] >= 5u;
        lib_bool compressed = (row / 9u) % 2u != 0u;
        lib_u8 mode = (lib_u8)(((row / 3u) % 3u) << 6u);
        lib_u8 transfer = (lib_u8)((row % 3u) << 2u);
        lib_u16 address_port = word ? (lib_u16)(0xc0u + channel * 4u) :
            (lib_u16)(channel * 2u);
        lib_u16 count_port = (lib_u16)(address_port + (word ? 2u : 1u));
        lib_u16 clear_port = word ? 0xd8u : 0x0cu;
        lib_u16 status_port = word ? 0xd0u : 0x08u;
        lib_u32 physical = word ? 0x2000u : 0x1000u;
        lib_u8 bytes[2] = {0x11u, 0x22u};
        lib_u8 phase;
        lib_i32 row_failed = 0;

        core_machine_dma_reset(bus);
        lib_memory_set(&fixture, 0u, sizeof(fixture));
        row_failed |= core_machine_memory_write(machine, physical,
            bytes, sizeof(bytes)) != LIB_STATUS_OK;
        if (word) {
            core_machine_dma_write_secondary_channel(machine, channel, 0x1000u,
                0u, 0u, (lib_u8)(mode | transfer | channel));
        } else {
            core_machine_dma_write_primary_channel(machine, channel, 0x1000u,
                0u, (lib_u8)(mode | transfer | channel));
        }
        core_machine_bus_write(machine, status_port,
            compressed ? 0x08u : 0u);
        core_machine_bus_write(machine, word ? 0xd4u : 0x0au, channel);
        core_machine_dma_request_assert(bus, &bindings[index]);
        /* One arbitration tick enters S1; no transfer until S4 executes. */
        for (phase = 0u; phase < (compressed ? 3u : 4u); ++phase) {
            core_machine_dma_advance_phases(bus,
                machine, 1u);
            row_failed |= fixture.reads != 0u || fixture.writes != 0u ||
                fixture.terminals != 0u;
            row_failed |= core_machine_dma_read_pair(machine, clear_port,
                address_port) != 0x1000u;
            row_failed |= core_machine_dma_read_pair(machine, clear_port,
                count_port) != 0u;
            row_failed |= (test_dma_port_read(machine, status_port) & 0x0fu) != 0u;
            row_failed |= core_machine_memory_read(machine, physical,
                bytes, sizeof(bytes)) != LIB_STATUS_OK ||
                bytes[0] != 0x11u || bytes[1] != 0x22u;
        }
        core_machine_dma_advance_phases(bus, machine, 1u);
        row_failed |= fixture.reads != (transfer == 8u ? 0u : 1u) ||
            fixture.writes != (transfer == 8u ? 1u : 0u) || fixture.terminals != 1u;
        row_failed |= transfer == 8u && fixture.last_write != (word ? 0x2211u : 0x11u);
        row_failed |= core_machine_dma_read_pair(machine, clear_port, address_port) != 0x1001u;
        row_failed |= core_machine_dma_read_pair(machine, clear_port, count_port) != 0xffffu;
        row_failed |= (test_dma_port_read(machine, status_port) & 0x0fu) != (1u << channel);
        row_failed |= core_machine_memory_read(machine, physical,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
            bytes[0] != (transfer == 4u ? 0x5au : 0x11u) ||
            bytes[1] != (transfer == 4u && word ? 0xa5u : 0x22u);
        core_machine_dma_advance_phases(bus, machine, 6u);
        row_failed |= fixture.reads + fixture.writes != 1u || fixture.terminals != 1u;
        if (row_failed) {
            lib_c_printf("DMA first-service mismatch: channel=%u TM=%u mode=%02x transfer=%02x\n",
                channels[index], compressed, mode, transfer);
            failed = 1;
        }
    }
done:
    core_machine_dma_finalize(bus);
    core_machine_destroy(machine);

    return failed;
}

lib_i32 main(void)
{
    static const core_machine_dma_channel_provider provider = {
        core_machine_dma_fixture_read,
        LIB_NULL,
        core_machine_dma_fixture_terminal
    };
    static const core_machine_dma_channel_provider word_provider = {
        core_machine_dma_word_fixture_read, LIB_NULL, LIB_NULL
    };
    static const core_machine_dma_channel_provider eop_provider = {
        core_machine_dma_eop_fixture_read, LIB_NULL,
        core_machine_dma_eop_fixture_terminal
    };
    static const core_machine_dma_channel_provider failure_provider = {
        LIB_NULL, core_machine_dma_failure_fixture_write, LIB_NULL
    };
    core_machine_dma_bus *bus = LIB_NULL;
    core_machine *machine = LIB_NULL;


    core_machine_dma_request_binding binding = {0};
    core_machine_dma_request_binding priority_binding = {0};
    core_machine_dma_request_binding eop_binding = {0};
    core_machine_dma_request_binding failure_binding = {0};
    core_machine_dma_request_binding word_bindings[3] = {{0}};
    core_machine_dma_fixture fixture = {{0xa5u, 0x5au}, 0u, 0u};
    core_machine_dma_fixture priority_fixture = {{0x71u, 0x72u}, 0u, 0u};
    core_machine_dma_eop_fixture eop_fixture = {{{0x91u, 0x92u}, 0u, 0u},
        LIB_NULL, LIB_NULL, LIB_FALSE};
    core_machine_dma_failure_fixture failure_fixture = {0u};
    core_machine_dma_word_fixture word_fixture = {{0x1234u, 0x5678u,
        0x9abcu}, 0u};
    lib_u8 bytes[2] = {0};
    lib_u8 zeroes[2] = {0};
    lib_u16 words[2] = {0};
    lib_u8 channel;
    lib_u16 page_port;
    lib_u8 dma_status;
    lib_i32 failed = core_machine_dma_first_service_matrix() || dma_construction_rollback();
    if (test_dma_core_create(&machine) != LIB_STATUS_OK ||
        core_machine_dma_bind_channel(bus, 2u,
            &provider, &fixture, &binding) != LIB_STATUS_INVALID_ARGUMENT) {
        failed = 1;
        goto done;
    }
    if (test_dma_initialize(&bus, machine, 2u) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_reset(bus);
    if (core_machine_dma_bind_channel(bus, 2u,
            &provider, &fixture, &binding) != LIB_STATUS_OK ||
        core_machine_dma_bind_channel(bus, 0u,
            &failure_provider, &failure_fixture, &failure_binding) != LIB_STATUS_OK ||
        core_machine_dma_bind_channel(bus, 1u,
            &provider, &priority_fixture, &priority_binding) != LIB_STATUS_OK ||
        core_machine_dma_bind_channel(bus, 3u,
            &eop_provider, &eop_fixture, &eop_binding) != LIB_STATUS_OK ||
        core_machine_dma_bind_channel(bus, 5u,
            &word_provider, &word_fixture, &word_bindings[0]) != LIB_STATUS_OK ||
        core_machine_dma_bind_channel(bus, 6u,
            &word_provider, &word_fixture, &word_bindings[1]) != LIB_STATUS_OK ||
        core_machine_dma_bind_channel(bus, 7u,
            &word_provider, &word_fixture, &word_bindings[2]) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    eop_fixture.bus = bus;
    eop_fixture.binding = &eop_binding;

    /* Intel's normal service is S1 -> S2 -> S3 -> S4, while TM removes
     * S3. Memory/I/O work commits only in S4. */
    core_machine_dma_write_channel2(machine, 0x1200u, 0u, 0u, 0x86u);
    core_machine_bus_write(machine, 0x000au, 0x02u);
    core_machine_bus_write(machine, 0x00d4u, 0u);
    core_machine_dma_request_assert(bus, &binding);
    core_machine_dma_advance_phases(bus, machine, 1u);
    core_machine_dma_advance_phases(bus, machine, 3u);
    if (fixture.next != 0u || x86_dma_get_signals(bus->primary.device).active_channel != 2u) {
        failed = 1;
    }
    core_machine_dma_advance_phases(bus, machine, 1u);
    if (fixture.next != 1u || x86_dma_get_signals(bus->primary.device).active_channel != 4u) {
        failed = 1;
    }
    /* Verify is a real peripheral service cycle, but never touches RAM. */
    core_machine_dma_reset(bus);
    fixture.next = 0u;
    bytes[0] = 0u;
    if (core_machine_memory_write(machine, 0x11200u,
            bytes, sizeof(bytes[0])) != LIB_STATUS_OK) {
        failed = 1;
    }
    core_machine_dma_write_channel2(machine, 0x1200u, 0u, 0u, 0x82u);
    core_machine_bus_write(machine, 0x000au, 0x02u);
    core_machine_dma_request_assert(bus, &binding);
    core_machine_dma_advance_phases(bus, machine, 5u);
    if (fixture.next != 1u || core_machine_memory_read(machine, 0x11200u,
            bytes, sizeof(bytes[0])) != LIB_STATUS_OK ||
        bytes[0] != 0u) {
        failed = 1;
    }
    core_machine_dma_reset(bus);
    fixture.next = 0u;
    core_machine_dma_write_channel2(machine, 0x1200u, 0u, 0u, 0x86u);
    core_machine_bus_write(machine, 0x0008u, 0x08u);
    core_machine_bus_write(machine, 0x000au, 0x02u);
    core_machine_bus_write(machine, 0x00d4u, 0u);
    core_machine_dma_request_assert(bus, &binding);
    core_machine_dma_advance_phases(bus, machine, 3u);
    if (fixture.next != 0u || x86_dma_get_signals(bus->primary.device).active_channel != 2u) {
        failed = 1;
    }
    core_machine_dma_advance_phases(bus, machine, 1u);
    if (fixture.next != 1u || x86_dma_get_signals(bus->primary.device).active_channel != 4u) {
        failed = 1;
    }
    core_machine_dma_reset(bus);
    fixture.next = 0u;
    fixture.terminal_count = 0u;

    /* Block-mode device -> RAM: count is inclusive, but each core DMA grant
     * may expose only one byte. */
    core_machine_dma_write_channel2(machine, 0x1234u, 0x01u, 1u, 0x86u);
    core_machine_bus_write(machine, 0x000au, 0x02u);
    core_machine_dma_request_assert(bus, &binding);
    core_machine_bus_write(machine, 0x00d4u, 0x00u);
    test_dma_transfers(bus, machine, machine, 1u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (core_machine_memory_read(machine, 0x11234u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0xa5u || bytes[1] != 0u || fixture.terminal_count != 0u ||
        (dma_status & (1u << 2u)) != 0u ||
        (test_dma_blocked_inputs(bus->primary.device) & (1u << 2u)) != 0u) {
        failed = 1;
    }
    test_dma_transfers(bus, machine, machine, 1u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (core_machine_memory_read(machine, 0x11234u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0xa5u || bytes[1] != 0x5au ||
        fixture.terminal_count != 1u ||
        (dma_status & (1u << 2u)) == 0u ||
        (test_dma_blocked_inputs(bus->primary.device) & (1u << 2u)) == 0u) {
        failed = 1;
    }

    /* A masked request and an explicit deassertion cannot move guest memory. */
    fixture.next = 0u;
    bytes[0] = bytes[1] = 0u;
    if (core_machine_memory_write(machine, 0x11234u,
            zeroes, sizeof(zeroes)) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_write_channel2(machine, 0x1234u, 0x01u, 0u, 0x46u);
    core_machine_bus_write(machine, 0x000au, 0x06u);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x11234u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0u) {
        failed = 1;
    }
    core_machine_bus_write(machine, 0x000au, 0x02u);
    core_machine_dma_request_deassert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x11234u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0u) {
        failed = 1;
    }

    /* Auto-initialize reloads the programmed address/count after terminal
     * count, so a later request uses the same guest byte again. */
    fixture.bytes[0] = 0x3cu;
    fixture.bytes[1] = 0xc3u;
    fixture.next = 0u;
    core_machine_dma_write_channel2(machine, 0x1236u, 0x01u, 0u, 0x96u);
    core_machine_bus_write(machine, 0x000au, 0x02u);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 1u);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x11236u,
            bytes, 1u) != LIB_STATUS_OK ||
        bytes[0] != 0xc3u || test_dma_register_word(machine, LIB_FALSE, 2u * 2u) != 0x1236u ||
        test_dma_register_word(machine, LIB_FALSE, 2u * 2u + 1u) != 0u ||
        (test_dma_blocked_inputs(bus->primary.device) & (1u << 2u)) != 0u) {
        failed = 1;
    }

    /* Address-decrement changes only the core-owned current address. */
    fixture.bytes[0] = 0x7eu;
    fixture.next = 0u;
    core_machine_dma_write_channel2(machine, 0x1238u, 0x01u, 0u, 0xa6u);
    core_machine_bus_write(machine, 0x000au, 0x02u);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x11238u,
            bytes, 1u) != LIB_STATUS_OK ||
        bytes[0] != 0x7eu || test_dma_register_word(machine, LIB_FALSE, 2u * 2u) != 0x1237u) {
        failed = 1;
    }

    /* A held hardware DREQ remains a level until its owner deasserts it. Demand
     * and single modes therefore receive one deterministic grant per tick while
     * the request remains asserted. */
    fixture.bytes[0] = 0x11u;
    fixture.bytes[1] = 0x22u;
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_reset(bus);
    core_machine_dma_write_channel2(machine, 0x1240u, 0x01u, 1u, 0x06u);
    core_machine_bus_write(machine, 0x000au, 0x02u);
    core_machine_bus_write(machine, 0x00d4u, 0x00u);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 1u);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x11240u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0x11u || bytes[1] != 0x22u || fixture.terminal_count != 1u ||
        (test_dma_blocked_inputs(bus->primary.device) & (1u << 2u)) == 0u) {
        failed = 1;
    }
    fixture.bytes[0] = 0x33u;
    fixture.bytes[1] = 0x44u;
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_reset(bus);
    core_machine_dma_write_channel2(machine, 0x1242u, 0x01u, 1u, 0x46u);
    core_machine_bus_write(machine, 0x000au, 0x02u);
    core_machine_bus_write(machine, 0x00d4u, 0x00u);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 1u);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x11242u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0x33u || bytes[1] != 0x44u || fixture.terminal_count != 1u ||
        (test_dma_blocked_inputs(bus->primary.device) & (1u << 2u)) == 0u) {
        failed = 1;
    }
    /* Memory-to-memory uses the same grant boundary. Channel 0 is the
     * source request and channel 1 supplies the destination count. */
    bytes[0] = 0x55u;
    bytes[1] = 0x66u;
    if (core_machine_memory_write(machine, 0x0200u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0300u,
            zeroes, sizeof(zeroes)) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_reset(bus);
    core_machine_dma_write_primary_channel(machine, 0u, 0x0200u, 1u, 0x80u);
    core_machine_dma_write_primary_channel(machine, 1u, 0x0300u, 1u, 0x81u);
    core_machine_bus_write(machine, 0x0008u, 0x01u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_bus_write(machine, 0x00d4u, 0u);
    core_machine_bus_write(machine, 0x0009u, 0x04u);
    core_machine_dma_advance_phases(bus, machine, 5u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (core_machine_memory_read(machine, 0x0300u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0u || bytes[1] != 0u || test_dma_port_read(machine, 0x0du) != 0x55u ||
        (dma_status & (1u << 0u)) != 0u) {
        failed = 1;
    }
    core_machine_dma_advance_phases(bus, machine, 4u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (core_machine_memory_read(machine, 0x0300u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0x55u || bytes[1] != 0u ||
        (dma_status & (1u << 1u)) != 0u) {
        failed = 1;
    }
    core_machine_dma_advance_phases(bus, machine, 7u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (core_machine_memory_read(machine, 0x0300u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0x55u || bytes[1] != 0x66u ||
        (dma_status & (1u << 1u)) == 0u ||
        x86_dma_get_signals(bus->primary.device).requests != 0u || x86_dma_get_signals(bus->primary.device).active_channel != 4u) {
        failed = 1;
    }

    /* In M2M the source and destination each use their own address-direction
     * mode bit; channel 0's direction must not leak into channel 1. */
    bytes[0] = 0x31u;
    bytes[1] = 0x32u;
    if (core_machine_memory_write(machine, 0x0220u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0320u,
            zeroes, sizeof(zeroes)) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_reset(bus);
    core_machine_dma_write_primary_channel(machine, 0u, 0x0221u, 1u, 0xa0u);
    core_machine_dma_write_primary_channel(machine, 1u, 0x0320u, 1u, 0x81u);
    core_machine_bus_write(machine, 0x0008u, 0x01u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_bus_write(machine, 0x00d4u, 0u);
    core_machine_bus_write(machine, 0x0009u, 0x04u);
    core_machine_dma_advance_phases(bus, machine, 16u);
    if (core_machine_memory_read(machine, 0x0320u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0x32u || bytes[1] != 0x31u ||
        test_dma_register_word(machine, LIB_FALSE, 0u * 2u) != 0x021fu ||
        test_dma_register_word(machine, LIB_FALSE, 1u * 2u) != 0x0322u) {
        failed = 1;
    }

    /* PC/AT page ports retain their complete readable latch; the word
     * controller ignores page bit zero only while it forms its address. */
    core_machine_bus_write(machine, 0x0081u, 0x11u);
    core_machine_bus_write(machine, 0x0082u, 0x12u);
    core_machine_bus_write(machine, 0x0083u, 0x13u);
    core_machine_bus_write(machine, 0x0087u, 0x17u);
    core_machine_bus_write(machine, 0x0089u, 0x19u);
    core_machine_bus_write(machine, 0x008au, 0x1au);
    core_machine_bus_write(machine, 0x008bu, 0x1bu);
    core_machine_bus_write(machine, 0x008fu, 0x1fu);
    if (test_dma_port_read(machine, 0x0081u) != 0x11u ||
        test_dma_port_read(machine, 0x0082u) != 0x12u ||
        test_dma_port_read(machine, 0x0083u) != 0x13u ||
        test_dma_port_read(machine, 0x0087u) != 0x17u ||
        test_dma_port_read(machine, 0x0089u) != 0x19u ||
        test_dma_port_read(machine, 0x008au) != 0x1au ||
        test_dma_port_read(machine, 0x008bu) != 0x1bu ||
        test_dma_port_read(machine, 0x008fu) != 0x1fu) {
        failed = 1;
    }
    /* IBM 5170 POST writes and immediately reads the whole page-register
     * block.  Every decoded latch must preserve all eight written bits. */
    for (page_port = 0x0080u; page_port <= 0x008fu; ++page_port) {
        lib_u8 value = (lib_u8)(page_port - 0x0080u);

        core_machine_bus_write(machine, page_port, value);
        if (test_dma_port_read(machine, page_port) != value) failed = 1;
    }

    core_machine_dma_write_primary_channel(machine, 0u, 0x1234u, 0x5678u,
        0x84u);
    if (core_machine_dma_read_pair(machine, 0x000cu, 0x0000u) != 0x1234u ||
        core_machine_dma_read_pair(machine, 0x000cu, 0x0001u) != 0x5678u) {
        failed = 1;
    }

    /* Exercise the same control register family through both sparse maps. */
    for (channel = 0u; channel < 2u; ++channel) {
        x86_dma *chip = channel ? bus->secondary.device : bus->primary.device;
        lib_u16 base = channel ? 0xc0u : 0u;
        lib_u16 stride = channel ? 2u : 1u;
        core_machine_dma_reset(bus);
        core_machine_bus_write(machine, (lib_u16)(base + stride * 8u), 0x10u);
        core_machine_bus_write(machine, (lib_u16)(base + stride * 11u), 0x82u);
        core_machine_bus_write(machine, (lib_u16)(base + stride * 9u), 6u);
        core_machine_bus_write(machine, (lib_u16)(base + stride * 10u), 6u);
        core_machine_bus_write(machine, (lib_u16)(base + stride * 15u), 5u);
        if (x86_dma_get_signals(chip).requests != 4u) failed = 1;
        core_machine_bus_write(machine, (lib_u16)(base + stride * 9u), 2u);
        if (test_dma_blocked_inputs(chip) != 5u) failed = 1;
        core_machine_bus_write(machine, (lib_u16)(base + stride * 14u), 0u);
        if (test_dma_blocked_inputs(chip) != 0u) failed = 1;
        core_machine_bus_write(machine, (lib_u16)(base + stride * 13u), 0u);
        if (test_dma_blocked_inputs(chip) != 15u ||
            x86_dma_get_signals(chip).requests != 0u ||
            !x86_dma_get_signals(chip).enabled) failed = 1;
    }

    /* Channels 5--7 retain the real word address layout and their sparse
     * secondary-controller ports. */
    core_machine_dma_reset(bus);
    core_machine_bus_write(machine, 0x00dcu, 0u);
    word_fixture.next = 0u;
    for (channel = 1u; channel <= 3u; ++channel) {
        lib_u32 physical = 0x20000u +
            ((lib_u32)(0x0100u + channel) << 1u);

        core_machine_dma_write_secondary_channel(machine, channel,
            (lib_u16)(0x0100u + channel), 0u, 0x02u,
            (lib_u8)(0x84u | channel));
        if (core_machine_dma_read_pair(machine, 0x00d8u,
                (lib_u16)(0x00c0u + channel * 4u)) !=
                (lib_u16)(0x0100u + channel) ||
            core_machine_dma_read_pair(machine, 0x00d8u,
                (lib_u16)(0x00c2u + channel * 4u)) != 0u) {
            failed = 1;
        }
        core_machine_dma_request_assert(bus,
            &word_bindings[channel - 1u]);
        test_dma_transfers(bus, machine, machine, 1u);
        if (core_machine_memory_read(machine, physical,
                words, sizeof(words[0])) != LIB_STATUS_OK ||
            words[0] != word_fixture.words[channel - 1u]) {
            failed = 1;
        }
    }

    /* Increment and decrement wrap the controller address while retaining the
     * programmed PC/AT page for byte and word channels. */
    core_machine_dma_reset(bus);
    core_machine_bus_write(machine, 0x00d4u, 0u);
    fixture.bytes[0] = 0x41u;
    fixture.bytes[1] = 0x42u;
    fixture.next = 0u;
    core_machine_dma_write_channel2(machine, 0xffffu, 0x01u, 1u, 0x86u);
    core_machine_bus_write(machine, 0x000au, 0x02u);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 2u);
    if (core_machine_memory_read(machine, 0x1ffffu,
            bytes, 1u) != LIB_STATUS_OK ||
        bytes[0] != 0x41u ||
        core_machine_memory_read(machine, 0x10000u,
            bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0x42u) {
        failed = 1;
    }

    core_machine_dma_reset(bus);
    core_machine_bus_write(machine, 0x00dcu, 0u);
    word_fixture.words[0] = 0x369cu;
    word_fixture.words[1] = 0x48adu;
    word_fixture.next = 0u;
    core_machine_dma_write_secondary_channel(machine, 1u, 0u, 0x01u, 0x02u,
        0xa5u);
    core_machine_dma_request_assert(bus, &word_bindings[0]);
    test_dma_transfers(bus, machine, machine, 2u);
    if (core_machine_memory_read(machine, 0x20000u,
            words, sizeof(words[0])) != LIB_STATUS_OK ||
        words[0] != 0x369cu ||
        core_machine_memory_read(machine, 0x3fffeu,
            words, sizeof(words[0])) != LIB_STATUS_OK ||
        words[0] != 0x48adu) {
        failed = 1;
    }

    core_machine_dma_reset(bus);
    core_machine_bus_write(machine, 0x00d4u, 0u);
    fixture.bytes[0] = 0x43u;
    fixture.bytes[1] = 0x44u;
    fixture.next = 0u;
    core_machine_dma_write_channel2(machine, 0u, 0x01u, 1u, 0xa6u);
    core_machine_bus_write(machine, 0x000au, 0x02u);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 2u);
    if (core_machine_memory_read(machine, 0x10000u,
            bytes, 1u) != LIB_STATUS_OK ||
        bytes[0] != 0x43u ||
        core_machine_memory_read(machine, 0x1ffffu,
            bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0x44u) {
        failed = 1;
    }

    core_machine_dma_reset(bus);
    core_machine_bus_write(machine, 0x00dcu, 0u);
    word_fixture.words[0] = 0x1357u;
    word_fixture.words[1] = 0x2468u;
    word_fixture.next = 0u;
    core_machine_dma_write_secondary_channel(machine, 1u, 0xffffu, 0x01u,
        0x02u, 0x85u);
    core_machine_dma_request_assert(bus, &word_bindings[0]);
    test_dma_transfers(bus, machine, machine, 2u);
    if (core_machine_memory_read(machine, 0x3fffeu,
            words, sizeof(words[0])) != LIB_STATUS_OK ||
        words[0] != 0x1357u ||
        core_machine_memory_read(machine, 0x20000u,
            words, sizeof(words[0])) != LIB_STATUS_OK ||
        words[0] != 0x2468u) {
        failed = 1;
    }

    /* Software request bits are non-maskable, but the 8237A admits them only
     * for block mode. A primary request reaches the primary controller through
     * the secondary controller's reserved cascade channel. */
    core_machine_dma_reset(bus);
    fixture.bytes[0] = 0x61u;
    fixture.bytes[1] = 0x62u;
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_write_channel2(machine, 0x1800u, 0u, 1u, 0x86u);
    core_machine_bus_write(machine, 0x000au, 0x06u);
    core_machine_bus_write(machine, 0x0009u, 0x06u);
    test_dma_transfers(bus, machine, machine, 2u);
    if (core_machine_memory_read(machine, 0x1800u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0x61u || bytes[1] != 0x62u ||
        fixture.terminal_count != 1u || x86_dma_get_signals(bus->primary.device).requests != 0u ||
        (test_dma_blocked_inputs(bus->primary.device) & (1u << 2u)) == 0u) {
        failed = 1;
    }

    core_machine_dma_reset(bus);
    bytes[0] = 0u;
    fixture.bytes[0] = 0x63u;
    fixture.next = 0u;
    core_machine_dma_write_channel2(machine, 0x1810u, 0u, 0u, 0x06u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_bus_write(machine, 0x0009u, 0x06u);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x1810u,
            bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0u) {
        failed = 1;
    }

    core_machine_bus_write(machine, 0x000bu, 0x86u);
    if ((x86_dma_get_signals(bus->primary.device).requests & 4u) == 0u) failed = 1;

    /* A programmed cascade slot delegates priority only: it must not invent
     * a transfer, terminal count, mask update or device completion. */
    core_machine_dma_reset(bus);
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_write_channel2(machine, 0x1820u, 0u, 0u, 0xc6u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 1u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (fixture.next != 0u || fixture.terminal_count != 0u ||
        (dma_status & (1u << 2u)) != 0u ||
        (test_dma_blocked_inputs(bus->primary.device) & (1u << 2u)) != 0u ||
        x86_dma_get_signals(bus->primary.device).active_channel != 4u) {
        failed = 1;
    }

    /* Channel 4 is the board cascade path, not a bindable or software-forced
     * primary transfer source. */
    if (core_machine_dma_bind_channel(bus, 4u,
            &provider, &fixture, &binding) != LIB_STATUS_INVALID_ARGUMENT) {
        failed = 1;
    }
    core_machine_bus_write(machine, 0x00d2u, 0x04u);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x1810u,
            bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0u) {
        failed = 1;
    }

    /* Fixed priority selects the lower primary channel first. Rotating
     * priority moves the last served channel behind its peer. */
    core_machine_dma_reset(bus);
    fixture.bytes[0] = 0x72u;
    fixture.next = 0u;
    priority_fixture.bytes[0] = 0x71u;
    priority_fixture.next = 0u;
    core_machine_dma_write_primary_channel(machine, 1u, 0x1900u, 0u, 0x45u);
    core_machine_dma_write_primary_channel(machine, 2u, 0x1902u, 0u, 0x46u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_dma_request_assert(bus, &priority_binding);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x1900u,
            bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0x71u ||
        core_machine_memory_read(machine, 0x1902u,
            bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0u) {
        failed = 1;
    }

    core_machine_dma_reset(bus);
    fixture.bytes[0] = 0x92u;
    fixture.bytes[1] = 0x93u;
    fixture.next = 0u;
    priority_fixture.bytes[0] = 0x81u;
    priority_fixture.bytes[1] = 0x82u;
    priority_fixture.next = 0u;
    core_machine_dma_write_primary_channel(machine, 1u, 0x1910u, 1u, 0x45u);
    core_machine_dma_write_primary_channel(machine, 2u, 0x1912u, 1u, 0x46u);
    core_machine_bus_write(machine, 0x0008u, 0x10u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_dma_request_assert(bus, &priority_binding);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 2u);
    if (core_machine_memory_read(machine, 0x1910u,
            bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0x81u ||
        core_machine_memory_read(machine, 0x1912u,
            bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0x92u) {
        failed = 1;
    }

    /* Secondary local channels retain the same rotation rule without letting
     * their word transfers escape the secondary controller. */
    core_machine_dma_reset(bus);
    word_fixture.words[0] = 0xa135u;
    word_fixture.words[1] = 0xb246u;
    word_fixture.next = 0u;
    core_machine_dma_write_secondary_channel(machine, 1u, 0x0a00u, 1u, 0u,
        0x45u);
    core_machine_dma_write_secondary_channel(machine, 2u, 0x0a01u, 1u, 0u,
        0x46u);
    core_machine_bus_write(machine, 0x00d0u, 0x10u);
    core_machine_bus_write(machine, 0x00dcu, 0u);
    core_machine_dma_request_assert(bus, &word_bindings[0]);
    core_machine_dma_request_assert(bus, &word_bindings[1]);
    test_dma_transfers(bus, machine, machine, 2u);
    if (core_machine_memory_read(machine, 0x1400u,
            words, sizeof(words[0])) != LIB_STATUS_OK ||
        words[0] != 0xa135u ||
        core_machine_memory_read(machine, 0x1402u,
            words, sizeof(words[0])) != LIB_STATUS_OK ||
        words[0] != 0xb246u) {
        failed = 1;
    }

    /* A masked primary DREQ must not become a real secondary cascade request
     * and starve an unrelated unmasked secondary channel. */
    core_machine_dma_reset(bus);
    words[0] = 0u;
    fixture.next = 0u;
    word_fixture.words[0] = 0xd357u;
    word_fixture.next = 0u;
    core_machine_dma_write_channel2(machine, 0x1a20u, 0u, 0u, 0x86u);
    core_machine_dma_write_secondary_channel(machine, 1u, 0x0b20u, 0u, 0u,
        0x85u);
    core_machine_bus_write(machine, 0x00dcu, 0u);
    core_machine_dma_request_assert(bus, &binding);
    core_machine_dma_request_assert(bus, &word_bindings[0]);
    test_dma_transfers(bus, machine, machine, 1u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (core_machine_memory_read(machine, 0x1640u,
            words, sizeof(words[0])) != LIB_STATUS_OK ||
        words[0] != 0xd357u || fixture.next != 0u ||
        !(dma_status & (0x10u << 2u))) {
        failed = 1;
    }

    /* Either controller disable gate prevents a pending bound request from
     * publishing a device or memory transfer until software reenables it. */
    core_machine_dma_reset(bus);
    bytes[0] = 0u;
    fixture.bytes[0] = 0xa1u;
    fixture.next = 0u;
    core_machine_dma_write_channel2(machine, 0x1a00u, 0u, 0u, 0x86u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_dma_request_assert(bus, &binding);
    core_machine_bus_write(machine, 0x0008u, 0x04u);
    test_dma_transfers(bus, machine, machine, 1u);
    core_machine_bus_write(machine, 0x0008u, 0u);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x1a00u,
            bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0xa1u) {
        failed = 1;
    }

    core_machine_dma_reset(bus);
    words[0] = 0u;
    word_fixture.words[0] = 0xb357u;
    word_fixture.next = 0u;
    core_machine_dma_write_secondary_channel(machine, 1u, 0x0af0u, 0u, 0u,
        0x85u);
    core_machine_bus_write(machine, 0x00dcu, 0u);
    core_machine_bus_write(machine, 0x0008u, 0x04u);
    core_machine_dma_request_assert(bus, &word_bindings[0]);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x15e0u,
            words, sizeof(words[0])) != LIB_STATUS_OK ||
        words[0] != 0xb357u) {
        failed = 1;
    }

    core_machine_dma_reset(bus);
    words[0] = 0u;
    word_fixture.words[0] = 0xc357u;
    word_fixture.next = 0u;
    core_machine_dma_write_secondary_channel(machine, 1u, 0x0b00u, 0u, 0u,
        0x85u);
    core_machine_bus_write(machine, 0x00dcu, 0u);
    core_machine_dma_request_assert(bus, &word_bindings[0]);
    core_machine_bus_write(machine, 0x00d0u, 0x04u);
    test_dma_transfers(bus, machine, machine, 1u);
    core_machine_bus_write(machine, 0x00d0u, 0u);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x1600u,
            words, sizeof(words[0])) != LIB_STATUS_OK ||
        words[0] != 0xc357u) {
        failed = 1;
    }

    /* External EOP is accepted only from the active opaque binding. It closes
     * the active service, records TC/masking, and leaves other channels alone. */
    core_machine_dma_reset(bus);
    eop_fixture.transfer.bytes[0] = 0xd1u;
    eop_fixture.transfer.bytes[1] = 0xd2u;
    eop_fixture.transfer.next = 0u;
    eop_fixture.transfer.terminal_count = 0u;
    eop_fixture.terminate_on_read = LIB_TRUE;
    core_machine_dma_write_primary_channel(machine, 3u, 0x1b00u, 2u, 0x87u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_dma_request_assert(bus, &eop_binding);
    test_dma_transfers(bus, machine, machine, 2u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (core_machine_memory_read(machine, 0x1b00u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        bytes[0] != 0xd1u || bytes[1] != 0u ||
        eop_fixture.transfer.terminal_count != 1u ||
        (dma_status & (1u << 3u)) == 0u ||
        (test_dma_blocked_inputs(bus->primary.device) & (1u << 3u)) == 0u) {
        failed = 1;
    }

    core_machine_dma_reset(bus);
    eop_fixture.transfer.bytes[0] = 0xe1u;
    eop_fixture.transfer.next = 0u;
    eop_fixture.transfer.terminal_count = 0u;
    eop_fixture.terminate_on_read = LIB_TRUE;
    core_machine_dma_write_primary_channel(machine, 3u, 0x1b10u, 2u, 0x97u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_dma_request_assert(bus, &eop_binding);
    test_dma_transfers(bus, machine, machine, 1u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (eop_fixture.transfer.terminal_count != 1u ||
        test_dma_register_word(machine, LIB_FALSE, 3u * 2u) != 0x1b10u || test_dma_register_word(machine, LIB_FALSE, 3u * 2u + 1u) != 2u ||
        (test_dma_blocked_inputs(bus->primary.device) & (1u << 3u)) != 0u ||
        (dma_status & (1u << 3u)) != 0u) {
        failed = 1;
    }

    /* A rejected physical route is a preflight failure: no provider, memory,
     * latch, address/count, request, terminal, or mask state may publish. */
    core_machine_dma_reset(bus);
    fixture.bytes[0] = 0xf1u;
    fixture.next = 0u;
    fixture.terminal_count = 0u;
    core_machine_dma_write_channel2(machine, 0x0000u, 0x20u, 0u, 0x86u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 1u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (fixture.next != 0u || fixture.terminal_count != 0u ||
        test_dma_register_word(machine, LIB_FALSE, 2u * 2u) != 0u || test_dma_register_word(machine, LIB_FALSE, 2u * 2u + 1u) != 0u ||
        !(dma_status & (0x10u << 2u)) ||
        x86_dma_get_signals(bus->primary.device).active_channel != 4u ||
        (test_dma_blocked_inputs(bus->primary.device) & (1u << 2u)) != 0u) {
        failed = 1;
    }

    core_machine_dma_reset(bus);
    failure_fixture.writes = 0u;
    core_machine_dma_write_primary_channel(machine, 0u, 0u, 0u, 0x88u);
    core_machine_bus_write(machine, 0x0087u, 0x20u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_dma_request_assert(bus, &failure_binding);
    test_dma_transfers(bus, machine, machine, 1u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (failure_fixture.writes != 0u || test_dma_register_word(machine, LIB_FALSE, 0u * 2u) != 0u ||
        test_dma_register_word(machine, LIB_FALSE, 0u * 2u + 1u) != 0u ||
        !(dma_status & (0x10u << 0u)) ||
        x86_dma_get_signals(bus->primary.device).active_channel != 4u ||
        (test_dma_blocked_inputs(bus->primary.device) & (1u << 0u)) != 0u) {
        failed = 1;
    }

    /* M2M terminates through channel 1's count. Auto-init restores both
     * participating current register pairs and leaves their mask/TC clear. */
    bytes[0] = 0x5cu;
    zeroes[0] = 0u;
    if (core_machine_memory_write(machine, 0x0210u,
            bytes, 1u) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0310u,
            zeroes, 1u) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_reset(bus);
    priority_fixture.terminal_count = 0u;
    core_machine_dma_write_primary_channel(machine, 0u, 0x0210u, 0u, 0x90u);
    core_machine_dma_write_primary_channel(machine, 1u, 0x0310u, 0u, 0x91u);
    core_machine_bus_write(machine, 0x0008u, 0x01u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_bus_write(machine, 0x0009u, 0x04u);
    core_machine_dma_advance_phases(bus, machine, 9u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (core_machine_memory_read(machine, 0x0310u,
            bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0x5cu ||
        test_dma_register_word(machine, LIB_FALSE, 0u * 2u) != 0x0210u || test_dma_register_word(machine, LIB_FALSE, 0u * 2u + 1u) != 0u ||
        test_dma_register_word(machine, LIB_FALSE, 1u * 2u) != 0x0310u || test_dma_register_word(machine, LIB_FALSE, 1u * 2u + 1u) != 0u ||
        (test_dma_blocked_inputs(bus->primary.device) & ((1u << 0u) | (1u << 1u))) != 0u ||
        (dma_status & (1u << 0u)) != 0u ||
        (dma_status & (1u << 1u)) == 0u ||
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
    if (core_machine_memory_write(machine, 0x0220u,
            bytes, sizeof(bytes)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0320u,
            zeroes, sizeof(zeroes)) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    core_machine_dma_reset(bus);
    priority_fixture.terminal_count = 0u;
    core_machine_dma_write_primary_channel(machine, 0u, 0x0220u, 1u, 0x80u);
    core_machine_dma_write_primary_channel(machine, 1u, 0x0320u, 1u, 0x81u);
    core_machine_bus_write(machine, 0x0008u, 0x01u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_bus_write(machine, 0x0009u, 0x04u);
    core_machine_dma_advance_phases(bus, machine, 9u);
    core_machine_dma_request_terminate(bus, &failure_binding);
    core_machine_dma_advance_phases(bus, machine, 3u);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (core_machine_memory_read(machine, 0x0320u,
            zeroes, sizeof(zeroes)) != LIB_STATUS_OK ||
        zeroes[0] != 0x61u || zeroes[1] != 0u ||
        priority_fixture.terminal_count != 1u ||
        (dma_status & (1u << 1u)) == 0u ||
        (test_dma_blocked_inputs(bus->primary.device) & ((1u << 0u) | (1u << 1u))) !=
            ((1u << 0u) | (1u << 1u)) ||
        test_dma_register_word(machine, LIB_FALSE, 0u * 2u) != 0x0221u ||
        test_dma_register_word(machine, LIB_FALSE, 1u * 2u) != 0x0321u) {
        failed = 1;
    }

    /* M2M validates both physical routes before it moves its temporary latch,
     * count, current addresses, or software request. */
    core_machine_dma_reset(bus);
    core_machine_dma_write_primary_channel(machine, 0u, 0u, 0u, 0x80u);
    core_machine_dma_write_primary_channel(machine, 1u, 0x0400u, 0u, 0x81u);
    core_machine_bus_write(machine, 0x0087u, 0x20u);
    core_machine_bus_write(machine, 0x0008u, 0x01u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_bus_write(machine, 0x0009u, 0x04u);
    core_machine_dma_advance_phases(bus, machine, 5u);
    if (test_dma_register_word(machine, LIB_FALSE, 0u * 2u) != 0u || test_dma_register_word(machine, LIB_FALSE, 1u * 2u) != 0x0400u ||
        test_dma_register_word(machine, LIB_FALSE, 1u * 2u + 1u) != 0u || !(x86_dma_get_signals(bus->primary.device).requests & (1u << 0u)) || x86_dma_get_signals(bus->primary.device).active_channel != 4u ||
        test_dma_port_read(machine, 0x0du) != 0u) {
        failed = 1;
    }

    core_machine_dma_reset(bus);
    core_machine_dma_write_primary_channel(machine, 0u, 0x0410u, 0u, 0x80u);
    core_machine_dma_write_primary_channel(machine, 1u, 0u, 0u, 0x81u);
    core_machine_bus_write(machine, 0x0083u, 0x20u);
    core_machine_bus_write(machine, 0x0008u, 0x01u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_bus_write(machine, 0x0009u, 0x04u);
    core_machine_dma_advance_phases(bus, machine, 9u);
    if (test_dma_register_word(machine, LIB_FALSE, 0u * 2u) != 0x0410u || test_dma_register_word(machine, LIB_FALSE, 1u * 2u) != 0u ||
        test_dma_register_word(machine, LIB_FALSE, 1u * 2u + 1u) != 0u || !(x86_dma_get_signals(bus->primary.device).requests & (1u << 0u)) || x86_dma_get_signals(bus->primary.device).active_channel != 4u ||
        test_dma_port_read(machine, 0x0du) != 0u) {
        failed = 1;
    }

    /* Reset clears transient controller state but does not revoke a machine
     * binding; the previously issued opaque request remains usable. */
    core_machine_dma_reset(bus);
    core_machine_dma_request_assert(bus, &binding);
    core_machine_dma_reset(bus);
    dma_status = (lib_u8)test_dma_port_read(machine, 8u);
    if (bus->primary.connect.read_provider[2] != core_machine_dma_fixture_read ||
        bus->primary.connect.device_owner[2] != &fixture ||
        dma_status != 0u || x86_dma_get_signals(bus->primary.device).requests != 0u ||
        x86_dma_get_signals(bus->primary.device).active_channel != 4u ||
        test_dma_port_read(machine, 0x0du) != 0u ||
        test_dma_blocked_inputs(bus->primary.device) != 0x0fu) {
        failed = 1;
    }
    fixture.bytes[0] = 0x6du;
    fixture.next = 0u;
    core_machine_dma_write_channel2(machine, 0x1c00u, 0u, 0u, 0x86u);
    core_machine_bus_write(machine, 0x000eu, 0u);
    core_machine_dma_request_assert(bus, &binding);
    test_dma_transfers(bus, machine, machine, 1u);
    if (core_machine_memory_read(machine, 0x1c00u,
            bytes, 1u) != LIB_STATUS_OK ||
        bytes[0] != 0x6du) {
        failed = 1;
    }

done:
    core_machine_dma_finalize(bus);
    core_machine_destroy(machine);

    if (failed) return 1;
    lib_c_printf("M5:T269:S1:DMA-GRANT:PORT:OK\n");
    lib_c_printf("M5:T269:S4:DMA-MODES:OK\n");
    lib_c_printf("M5:T230:S3:DMA-CHANNEL:OK\n");
    lib_c_printf("M5:T348:S2:DMA-PORT-PAGE:OK\n");
    lib_c_printf("M5:T348:S3:DMA-REQUEST-CASCADE:OK\n");
    lib_c_printf("M5:T348:S4:DMA-TRANSACTION-LIFECYCLE:OK\n");
    return 0;
}
