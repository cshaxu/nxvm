#include <assert.h>

#include "machine/driver.h"
#include "core/machine.h"
#include "lib/base/clock_interface.h"
#include "lib/storage/file_interface.h"

#define PACING_CPU_HZ 1789773u
#define PACING_SLICES 128u

static lib_u8 fixture[16u + 16384u];

static void write_fixture(void)
{
    lib_storage_file_writer *writer = LIB_NULL;

    assert(lib_storage_file_writer_open("pacing-fixture.nes",
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, fixture, sizeof(fixture)) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

static void make_fixture(void)
{
    lib_memory_set(fixture, 0, sizeof(fixture));
    fixture[0] = 'N'; fixture[1] = 'E'; fixture[2] = 'S'; fixture[3] = 0x1au;
    fixture[4] = 1u;
    /* $8000: JMP $8000.  The test needs deterministic guest work only. */
    fixture[16u] = 0x4cu; fixture[17u] = 0u; fixture[18u] = 0x80u;
    fixture[16u + 0x3ffcu] = 0u; fixture[16u + 0x3ffdu] = 0x80u;
}

int main(void)
{
    machine_driver *driver = LIB_NULL;
    lib_u64 begin;
    lib_u64 end;
    lib_u64 begin_cycles;
    lib_u64 guest_milliseconds;
    lib_u32 slice;

    make_fixture();
    write_fixture();
    assert(machine_driver_create(&driver, &(machine_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(machine_driver_set_media(driver, "pacing-fixture.nes",
        LIB_STORAGE_MEDIUM_READONLY));
    assert(machine_driver_reset(driver));
    assert(driver->audio_needs_clear == LIB_FALSE);
    machine_driver_set_heartbeat(driver, LIB_TRUE);
    begin_cycles = driver->machine->cycles;
    assert(base_clock_milliseconds(&begin) == LIB_STATUS_OK);
    for (slice = 0u; slice < PACING_SLICES; ++slice) {
        /* A continuous wake stream models active host input/callback traffic.
         * It may shorten response latency, but cannot accelerate the guest. */
        machine_driver_request_wake(driver);
        assert(machine_driver_run(driver));
    }
    assert(base_clock_milliseconds(&end) == LIB_STATUS_OK);
    guest_milliseconds = (driver->machine->cycles - begin_cycles) * 1000u /
        PACING_CPU_HZ;
    assert(end >= begin);
    /* Allow one 5 ms host scheduling quantum while rejecting a spin loop. */
    assert(end - begin + 5u >= guest_milliseconds);
    /* A physical output can be present yet stopped by the test/RDP host. Core
     * preserves order under that backpressure and remains bounded either way. */
    assert(driver->audio_staging_count <= MACHINE_DRIVER_AUDIO_STAGING_CAPACITY);
    machine_driver_set_heartbeat(driver, LIB_FALSE);
    assert(driver->audio_staging_count == 0u);
    /* Resume starts pacing at the current guest position, not the pre-pause
     * origin. Disable is idempotent and discards the adapter's pending PCM. */
    for (slice = 0u; slice < 2u; ++slice) {
        driver->audio_staging_count = 2u;
        driver->audio_staging_read = 1u;
        driver->audio_staging_write = 3u;
        machine_driver_set_heartbeat(driver, LIB_FALSE);
        assert(driver->audio_staging_count == 0u);
        assert(driver->audio_staging_read == 0u && driver->audio_staging_write == 0u);
    }
    driver->pacing_ready = LIB_FALSE;
    driver->pacing_origin_cycles = LIB_UINT64_MAX;
    machine_driver_set_heartbeat(driver, LIB_TRUE);
    assert(driver->pacing_ready);
    assert(driver->pacing_origin_cycles == driver->machine->cycles);
    machine_driver_set_heartbeat(driver, LIB_FALSE);
    assert(machine_driver_destroy(driver) == LIB_STATUS_OK);
    return 0;
}
