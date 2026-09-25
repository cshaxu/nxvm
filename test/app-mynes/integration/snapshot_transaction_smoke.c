#include <assert.h>

#include "common/machine/machine_interface.h"
#include "core/debug_interface.h"
#include "core/driver_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/storage/file_interface.h"

typedef struct snapshot_bytes {
    lib_u8 bytes[262144];
    lib_size size, cursor;
} snapshot_bytes;

static lib_status put(void *opaque, const lib_u8 *bytes, lib_size count)
{
    snapshot_bytes *snapshot = opaque;
    if (count > sizeof(snapshot->bytes) - snapshot->size) return LIB_STATUS_LIMIT_EXCEEDED;
    lib_memory_copy(snapshot->bytes + snapshot->size, bytes, count);
    snapshot->size += count;
    return LIB_STATUS_OK;
}

static lib_status get(void *opaque, lib_u8 *bytes, lib_size count)
{
    snapshot_bytes *snapshot = opaque;
    if (count > snapshot->size - snapshot->cursor) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(bytes, snapshot->bytes + snapshot->cursor, count);
    snapshot->cursor += count;
    return LIB_STATUS_OK;
}

static void wait_state(common_machine *machine, common_machine_state state)
{
    for (lib_u32 i = 0u; i < 1000u; ++i) {
        if (common_machine_state_get(machine) == state) return;
        base_sync_sleep_milliseconds(1u);
    }
    assert(0 && "lifecycle did not complete");
}

static void fixture(void)
{
    /* Continuously sample controller A; change the backdrop once per vblank. */
    static const lib_u8 program[] = {
        0xa9,1, 0x8d,0x16,0x40, 0xa9,0, 0x8d,0x16,0x40,
        0xad,0x16,0x40, 0x29,1, 0x85,0,
        0x2c,0x02,0x20, 0x10,0xea, 0xe6,1,
        0xa9,0x3f, 0x8d,6,0x20, 0xa9,0, 0x8d,6,0x20,
        0xa5,1, 0x29,0x3f, 0x8d,7,0x20, 0x4c,0,0x80
    };
    lib_u8 bytes[16400] = {0};
    lib_storage_file_writer *writer = LIB_NULL;
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1a;
    bytes[4] = 1;
    lib_memory_copy(bytes + 16u, program, sizeof(program));
    bytes[16397] = 0x80;
    assert(lib_storage_file_writer_open("mynes-snapshot.nes",
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, bytes, sizeof(bytes)) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

static void debug(common_machine *machine, const lib_u8 *request,
    lib_size request_size, lib_u8 *response, lib_size expected_size)
{
    common_machine_debug_lease lease;
    lib_size size = 0u;
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease, request,
        request_size, response, expected_size, &size) == LIB_STATUS_OK);
    assert(size == expected_size);
}

static lib_u64 cycles(common_machine *machine)
{
    lib_u8 request[8] = {CORE_DEBUG_VERSION, 0, CORE_DEBUG_OBSERVE};
    lib_u8 response[52];
    lib_u64 value = 0u;
    debug(machine, request, sizeof(request), response, sizeof(response));
    for (lib_u32 i = 0u; i < 8u; ++i) value |= (lib_u64)response[20u + i] << (8u * i);
    return value;
}

static void run_and_check(common_machine *machine, lib_bool pressed)
{
    lib_u64 before = cycles(machine);
    lib_u32 sequence;
    kvm_input_event event = {0};
    lib_u8 request[12] = {CORE_DEBUG_VERSION, 0, CORE_DEBUG_PEEK, 0, 4, 0, 0, 0, 0, 0, 1};
    lib_u8 response[17];
    assert(common_machine_resume(machine));
    wait_state(machine, COMMON_MACHINE_RUNNING);
    event.type = KVM_EVENT_KEY;
    event.source_identity = 1u;
    event.data.key.key = 'K';
    event.data.key.pressed = pressed;
    assert(common_machine_enqueue_input(machine, &event));
    sequence = common_machine_published_frame_sequence(machine);
    for (lib_u32 i = 0u; i < 1000u; ++i) {
        if (common_machine_published_frame_sequence(machine) - sequence >= 2u) break;
        base_sync_sleep_milliseconds(1u);
    }
    assert(common_machine_pause(machine));
    wait_state(machine, COMMON_MACHINE_PAUSED);
    assert(cycles(machine) > before && "Running must actually advance guest cycles");
    assert(common_machine_published_frame_sequence(machine) - sequence >= 2u);
    debug(machine, request, sizeof(request), response, sizeof(response));
    assert(response[16] == (pressed ? 1u : 0u));
}

int main(void)
{
    fixture();
    for (lib_u32 text = 0u; text < 2u; ++text) {
        core_driver *driver = LIB_NULL;
        common_machine *machine = LIB_NULL;
        common_machine_driver binding;
        snapshot_bytes snapshot = {0};
        lib_u64 saved;
        assert(core_driver_create(&driver, &(core_driver_options){.text_output = text != 0u}) == LIB_STATUS_OK);
        assert(core_driver_make_driver(driver, &binding) == LIB_STATUS_OK);
        assert(common_machine_create(&machine, &binding) == LIB_STATUS_OK);
        assert(common_machine_set_removable_media(machine, "mynes-snapshot.nes", LIB_STORAGE_MEDIUM_READONLY));
        assert(common_machine_reset(machine));
        wait_state(machine, COMMON_MACHINE_PAUSED);
        run_and_check(machine, LIB_TRUE);
        assert(common_machine_read_state(machine, &(common_machine_state_writer){put, &snapshot}) == LIB_STATUS_OK);
        wait_state(machine, COMMON_MACHINE_PAUSED);
        run_and_check(machine, LIB_FALSE);
        saved = cycles(machine);
        snapshot.size = 0u;
        assert(common_machine_read_state(machine, &(common_machine_state_writer){put, &snapshot}) == LIB_STATUS_OK);
        wait_state(machine, COMMON_MACHINE_PAUSED);
        /* Restore the same completed frame: the old publication cache must
         * not suppress the first image of the new run generation. */
        {
            lib_u32 sequence = common_machine_published_frame_sequence(machine);
            assert(common_machine_stop(machine));
            wait_state(machine, COMMON_MACHINE_STOPPED);
            snapshot.cursor = 0u;
            assert(common_machine_write_state(machine, &(common_machine_state_reader){get, &snapshot}) == LIB_STATUS_OK);
            wait_state(machine, COMMON_MACHINE_PAUSED);
            assert(cycles(machine) == saved);
            assert(common_machine_published_frame_sequence(machine) != sequence);
        }
        run_and_check(machine, LIB_FALSE);
        for (lib_u32 repeat = 0u; repeat < 2u; ++repeat) {
            assert(common_machine_stop(machine));
            wait_state(machine, COMMON_MACHINE_STOPPED);
            snapshot.cursor = 0u;
            assert(common_machine_write_state(machine, &(common_machine_state_reader){get, &snapshot}) == LIB_STATUS_OK);
            wait_state(machine, COMMON_MACHINE_PAUSED);
            assert(cycles(machine) == saved);
            run_and_check(machine, repeat == 0u);
        }
        assert(common_machine_stop(machine));
        wait_state(machine, COMMON_MACHINE_STOPPED);
        snapshot.bytes[0] = 'X'; snapshot.cursor = 0u;
        assert(common_machine_write_state(machine, &(common_machine_state_reader){get, &snapshot}) != LIB_STATUS_OK);
        assert(common_machine_state_get(machine) == COMMON_MACHINE_STOPPED);
        assert(common_machine_reset(machine));
        wait_state(machine, COMMON_MACHINE_PAUSED);
        run_and_check(machine, LIB_FALSE);
        assert(common_machine_shutdown(machine) == LIB_STATUS_OK);
        assert(common_machine_destroy(machine) == LIB_STATUS_OK);
        assert(core_driver_destroy(driver) == LIB_STATUS_OK);
    }
    return 0;
}
