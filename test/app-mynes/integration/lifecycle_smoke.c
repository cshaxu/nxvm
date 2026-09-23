#include <assert.h>

#include "common/machine/machine_interface.h"
#include "core/driver_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/storage/file_interface.h"

#define FIXTURE_PATH "mynes-lifecycle-fixture.nes"
#define REPLACEMENT_PATH "mynes-lifecycle-replacement.nes"

static void make_fixture(lib_u8 *bytes)
{
    static const lib_u8 program[] = {
        0x78u, 0xa9u, 0x2au, 0x8du, 0x00u, 0x02u, 0xa2u, 0x00u,
        0xe8u, 0xe0u, 0x0au, 0xd0u, 0xfbu, 0x4cu, 0x0du, 0x80u
    };

    lib_memory_set(bytes, 0, 16u + 16384u);
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    lib_memory_copy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0x00u;
    bytes[16u + 0x3ffdu] = 0x80u;
}

static void write_fixture(const char *path, const lib_u8 *bytes, lib_size size)
{
    lib_storage_file_writer *writer = 0;

    assert(lib_storage_file_writer_open(path, LIB_STORAGE_FILE_WRITER_TRUNCATE,
        &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, bytes, size) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

static void wait_for_state(common_machine *machine, common_machine_state expected)
{
    lib_u32 attempt;
    for (attempt = 0u; attempt < 1000u; ++attempt) {
        if (common_machine_state_get(machine) == expected) return;
        base_sync_sleep_milliseconds(1u);
    }
    assert(0 && "machine did not reach its expected lifecycle state");
}

static void observe(common_machine *machine, lib_u8 *response)
{
    common_machine_debug_lease lease;
    static const lib_u8 request[8] = { 1u, 0u, 1u, 0u, 0u, 0u, 0u, 0u };
    lib_size response_size = 0u;

    base_sync_sleep_milliseconds(5u);
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease, request,
        sizeof(request), response, 64u, &response_size) == LIB_STATUS_OK);
    assert(response_size == 52u);
}

int main(void)
{
    lib_u8 fixture[16u + 16384u];
    lib_u8 replacement[16u + 16384u];
    lib_u8 invalid[16u + 16384u];
    lib_u8 response[64u];
    lib_u8 response_before_rejection[64u];
    core_driver *driver = 0;
    common_machine *machine = 0;
    common_machine_driver common_driver;
    lib_u32 repetition;

    make_fixture(fixture);
    make_fixture(replacement);
    replacement[16u + 0x0010u] = 0xeau;
    replacement[16u + 0x3ffcu] = 0x10u;
    lib_memory_set(invalid, 0, sizeof(invalid));
    write_fixture(FIXTURE_PATH, fixture, sizeof(fixture));
    write_fixture(REPLACEMENT_PATH, replacement, sizeof(replacement));
    write_fixture("mynes-invalid-fixture.nes", invalid, sizeof(invalid));
    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_make_driver(driver, &common_driver) == LIB_STATUS_OK);
    assert(common_machine_create(&machine, &common_driver) == LIB_STATUS_OK);
    assert(common_machine_set_removable_media(machine, FIXTURE_PATH,
        LIB_STORAGE_MEDIUM_READONLY));
    assert(common_machine_reset(machine));
    wait_for_state(machine, COMMON_MACHINE_PAUSED);
    observe(machine, response);
    assert(response[13] == 0u && response[18] == 0u && response[19] == 0x80u);
    for (repetition = 0u; repetition < 3u; ++repetition) {
        assert(common_machine_resume(machine));
        wait_for_state(machine, COMMON_MACHINE_RUNNING);
        assert(common_machine_pause(machine));
        wait_for_state(machine, COMMON_MACHINE_PAUSED);
        observe(machine, response);
        assert(response[46] == 0u);
    }
    observe(machine, response);
    assert(response[13] == 0x0au && response[18] == 0x0du && response[19] == 0x80u);
    lib_memory_copy(response_before_rejection, response, sizeof(response_before_rejection));
    assert(!common_machine_set_removable_media(machine, "mynes-invalid-fixture.nes",
        LIB_STORAGE_MEDIUM_READONLY));
    assert(core_driver_has_cartridge(driver));
    assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
    observe(machine, response);
    assert(lib_memory_compare(response, response_before_rejection, 52u) == 0);
    assert(common_machine_set_removable_media(machine, REPLACEMENT_PATH,
        LIB_STORAGE_MEDIUM_READONLY));
    assert(core_driver_has_cartridge(driver));
    observe(machine, response);
    assert(response[18] == 0x10u && response[19] == 0x80u);
    assert(common_machine_reset(machine));
    wait_for_state(machine, COMMON_MACHINE_PAUSED);
    observe(machine, response);
    assert(response[13] == 0u);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
    base_sync_sleep_milliseconds(5u);
    assert(common_machine_set_removable_media(machine, 0, LIB_STORAGE_MEDIUM_READONLY));
    assert(!core_driver_has_cartridge(driver));
    assert(common_machine_shutdown(machine) == LIB_STATUS_OK);
    assert(common_machine_destroy(machine) == LIB_STATUS_OK);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
    return 0;
}
