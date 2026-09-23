#include <assert.h>
#include <string.h>

#include "core/driver.h"
#include "core/machine.h"
#include "lib/storage/file_interface.h"

#define FIXTURE_PATH "mynes-controller-rom-fixture.nes"

static void write_fixture(void)
{
    lib_u8 bytes[16u + 16384u];
    lib_storage_file_writer *writer = LIB_NULL;
    /* Latch pad one, read A through $4016, store it at $0000, then loop. */
    static const lib_u8 program[] = {
        0xa9u, 1u, 0x8du, 0x16u, 0x40u, 0xa9u, 0u, 0x8du, 0x16u, 0x40u,
        0xadu, 0x16u, 0x40u, 0x8du, 0u, 0u, 0x4cu, 0u, 0x80u
    };

    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    memcpy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(lib_storage_file_writer_open(FIXTURE_PATH, LIB_STORAGE_FILE_WRITER_TRUNCATE,
        &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, bytes, sizeof(bytes)) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

int main(void)
{
    core_driver *driver = LIB_NULL;
    kvm_input_event input = { .type = KVM_EVENT_KEY, .source_identity = 1u };
    lib_u32 index;

    write_fixture();
    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_set_media(driver, FIXTURE_PATH, LIB_STORAGE_MEDIUM_READONLY));
    input.data.key.key = 'K'; input.data.key.pressed = 1u;
    core_driver_deliver_input(driver, &input);
    for (index = 0u; index < 16u; ++index) assert(core_driver_run(driver));
    assert((driver->machine->ram[0] & 1u) == 1u);
    input.data.key.pressed = 0u;
    core_driver_deliver_input(driver, &input);
    for (index = 0u; index < 16u; ++index) assert(core_driver_run(driver));
    assert((driver->machine->ram[0] & 1u) == 0u);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
    return 0;
}
