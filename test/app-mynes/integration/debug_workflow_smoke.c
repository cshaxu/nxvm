#include <assert.h>
#include <string.h>

#include "common/machine/machine_interface.h"
#include "core/driver_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/storage/file_interface.h"

#define FIXTURE_PATH "mynes-debug-workflow-fixture.nes"

static void write_u16(lib_u8 *bytes, lib_u16 value)
{
    bytes[0] = (lib_u8)value;
    bytes[1] = (lib_u8)(value >> 8u);
}

static void write_u32(lib_u8 *bytes, lib_u32 value)
{
    bytes[0] = (lib_u8)value;
    bytes[1] = (lib_u8)(value >> 8u);
    bytes[2] = (lib_u8)(value >> 16u);
    bytes[3] = (lib_u8)(value >> 24u);
}

static lib_u16 read_u16(const lib_u8 *bytes)
{
    return (lib_u16)bytes[0] | ((lib_u16)bytes[1] << 8u);
}

static lib_u32 read_u32(const lib_u8 *bytes)
{
    return (lib_u32)bytes[0] | ((lib_u32)bytes[1] << 8u) |
        ((lib_u32)bytes[2] << 16u) | ((lib_u32)bytes[3] << 24u);
}

static void make_fixture(lib_u8 *bytes)
{
    static const lib_u8 program[] = { 0xa9u, 0x2au, 0xeau, 0x4cu, 0x02u, 0x80u };
    memset(bytes, 0, 16u + 16384u);
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    memcpy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0x00u;
    bytes[16u + 0x3ffdu] = 0x80u;
}

static void write_fixture(const lib_u8 *bytes, lib_size size)
{
    lib_storage_file_writer *writer = 0;
    assert(lib_storage_file_writer_open(FIXTURE_PATH, LIB_STORAGE_FILE_WRITER_TRUNCATE,
        &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, bytes, size) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

static void wait_for_paused(common_machine *machine)
{
    lib_u32 attempt;
    for (attempt = 0u; attempt < 1000u; ++attempt) {
        if (common_machine_state_get(machine) == COMMON_MACHINE_PAUSED) return;
        base_sync_sleep_milliseconds(1u);
    }
    assert(0 && "machine did not pause");
}

static lib_size execute(common_machine *machine, const lib_u8 *request,
    lib_size request_size, lib_u8 *response, lib_size response_capacity)
{
    common_machine_debug_lease lease;
    lib_size response_size = 0u;
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease, request,
        request_size, response, response_capacity, &response_size) == LIB_STATUS_OK);
    return response_size;
}

static void header(lib_u8 *request, lib_u16 operation, lib_u32 payload_size)
{
    write_u16(request, 1u);
    write_u16(request + 2u, operation);
    write_u32(request + 4u, payload_size);
}

static void wait_for_instruction_count(common_machine *machine, lib_u64 expected)
{
    lib_u8 request[8] = { 0 };
    lib_u8 response[64];
    lib_u32 attempt;
    header(request, 1u, 0u);
    for (attempt = 0u; attempt < 1000u; ++attempt) {
        if (common_machine_state_get(machine) == COMMON_MACHINE_PAUSED) {
            if (execute(machine, request, sizeof(request), response, sizeof(response)) == 52u &&
                ((lib_u64)read_u32(response + 28u) |
                ((lib_u64)read_u32(response + 32u) << 32u)) == expected)
                return;
        }
        base_sync_sleep_milliseconds(1u);
    }
    assert(0 && "machine did not stop at expected instruction count");
}

int main(void)
{
    lib_u8 fixture[16u + 16384u];
    lib_u8 request[128] = { 0 };
    lib_u8 response[1536];
    core_driver *driver = 0;
    common_machine *machine = 0;
    common_machine_driver common_driver;
    common_machine_debug_lease lease;
    lib_size response_size = 0u;

    make_fixture(fixture);
    write_fixture(fixture, sizeof(fixture));
    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_make_driver(driver, &common_driver) == LIB_STATUS_OK);
    assert(common_machine_create(&machine, &common_driver) == LIB_STATUS_OK);
    assert(common_machine_set_removable_media(machine, FIXTURE_PATH,
        LIB_STORAGE_MEDIUM_READONLY));
    assert(common_machine_reset(machine));
    wait_for_paused(machine);

    header(request, 1u, 0u);
    assert(execute(machine, request, 8u, response, sizeof(response)) == 52u);
    assert(read_u16(response) == 1u && read_u16(response + 2u) == 1u);
    assert(read_u16(response + 18u) == 0x8000u);

    header(request, 3u, 6u);
    write_u16(request + 8u, 0x0010u); write_u16(request + 10u, 2u);
    request[12] = 0x55u; request[13] = 0xaau;
    assert(execute(machine, request, 14u, response, sizeof(response)) == 12u);

    header(request, 2u, 4u);
    write_u16(request + 8u, 0x0010u); write_u16(request + 10u, 2u);
    assert(execute(machine, request, 12u, response, sizeof(response)) == 18u);
    assert(read_u16(response + 12u) == 2u && response[16] == 0x55u && response[17] == 0xaau);

    header(request, 4u, 4u); write_u32(request + 8u, 1u);
    assert(execute(machine, request, 12u, response, sizeof(response)) == 60u);
    assert(response[8] == 1u && response[12] == 1u && response[20] == 0x2au);

    header(request, 5u, 4u);
    write_u16(request + 8u, 0x8002u); write_u16(request + 10u, 1u);
    assert(execute(machine, request, 12u, response, sizeof(response)) == 12u);
    header(request, 6u, 0u);
    assert(execute(machine, request, 8u, response, sizeof(response)) == 18u);
    assert(read_u16(response + 12u) == 1u && read_u16(response + 16u) == 0x8002u);

    assert(common_machine_resume(machine));
    wait_for_instruction_count(machine, 1u);
    header(request, 1u, 0u);
    (void)execute(machine, request, 8u, response, sizeof(response));
    assert(read_u16(response + 18u) == 0x8002u);
    assert(common_machine_resume(machine));
    wait_for_instruction_count(machine, 3u);
    (void)execute(machine, request, 8u, response, sizeof(response));
    assert(read_u16(response + 18u) == 0x8002u);

    header(request, 7u, 0u);
    assert(execute(machine, request, 8u, response, sizeof(response)) == 52u);
    assert(read_u16(response + 18u) == 0x8000u);
    header(request, 2u, 4u);
    write_u16(request + 8u, 0x0010u); write_u16(request + 10u, 2u);
    (void)execute(machine, request, 12u, response, sizeof(response));
    assert(response[16] == 0x55u && response[17] == 0xaau);

    header(request, 3u, 5u);
    write_u16(request + 8u, 0x8000u); write_u16(request + 10u, 1u); request[12] = 0u;
    memset(response, 0x5au, sizeof(response));
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease, request, 13u,
        response, sizeof(response), &response_size) == LIB_STATUS_INVALID_ARGUMENT);
    assert(response_size == 0u && response[0] == 0x5au);

    header(request, 8u, 8u);
    memset(request + 8u, 0, 8u);
    request[8u] = 1u;
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease, request, 16u,
        response, sizeof(response), &response_size) == LIB_STATUS_OK);
    assert(response_size == 12u && read_u16(response + 2u) == 8u);
    request[8u] = 2u;
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease, request, 16u,
        response, sizeof(response), &response_size) == LIB_STATUS_INVALID_ARGUMENT);

    header(request, 9u, 12u);
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease, request, 20u,
        response, sizeof(response), &response_size) == LIB_STATUS_UNSUPPORTED);
    memset(response, 0x5au, sizeof(response));
    header(request, 1u, 0u); request[0] = 2u;
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease, request, 8u,
        response, sizeof(response), &response_size) == LIB_STATUS_UNSUPPORTED);
    assert(response_size == 0u && response[0] == 0x5au);
    header(request, 1u, 1u); request[8] = 0u;
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease, request, 9u,
        response, sizeof(response), &response_size) == LIB_STATUS_INVALID_ARGUMENT);
    header(request, 1u, 0u);
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease, request, 8u,
        response, 51u, &response_size) == LIB_STATUS_INVALID_ARGUMENT);

    {
        lib_u16 address;
        for (address = 0x8100u; address < 0x810fu; ++address) {
            header(request, 5u, 4u);
            write_u16(request + 8u, address); write_u16(request + 10u, 1u);
            (void)execute(machine, request, 12u, response, sizeof(response));
        }
        header(request, 5u, 4u);
        write_u16(request + 8u, 0x8110u); write_u16(request + 10u, 1u);
        assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
        assert(common_machine_debug_execute_with_lease(machine, &lease, request, 12u,
            response, sizeof(response), &response_size) == LIB_STATUS_LIMIT_EXCEEDED);
    }
    header(request, 1u, 0u);
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_reset(machine));
    wait_for_paused(machine);
    assert(common_machine_debug_execute_with_lease(machine, &lease, request, 8u,
        response, sizeof(response), &response_size) == LIB_STATUS_INVALID_STATE);

    assert(common_machine_shutdown(machine) == LIB_STATUS_OK);
    assert(common_machine_destroy(machine) == LIB_STATUS_OK);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
    return 0;
}
