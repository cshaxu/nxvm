#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/driver.h"
#include "core/machine.h"

static lib_u32 probe_audio_queued(const core_driver *driver)
{
    lib_u32 queued;
    lib_u32 writable;

    assert(driver != LIB_NULL);
    if (driver->audio == LIB_NULL) return 0u;
    assert(lib_audio_stream_query(driver->audio, &queued, &writable) == LIB_STATUS_OK);
    return queued;
}

static void probe(const char *variable, lib_u8 mapper, lib_bool text_output)
{
    const char *path = getenv(variable);
    core_driver *driver = LIB_NULL;
    const char *limit_text = getenv("MYNES_OWNER_SLICES");
    lib_u32 limit = 20000u;
    lib_u32 slice;
    common_machine_frame frame = { 0 };

    if (path == LIB_NULL || path[0] == '\0') return;
    if (limit_text != LIB_NULL && limit_text[0] != '\0') {
        unsigned long value = strtoul(limit_text, LIB_NULL, 10);
        assert(value != 0u && value <= LIB_UINT32_MAX);
        limit = (lib_u32)value;
    }
    assert(core_driver_create(&driver, &(core_driver_options) {
        .text_output = text_output }) == LIB_STATUS_OK);
    assert(core_driver_set_media(driver, path, LIB_STORAGE_MEDIUM_READONLY));
    assert(driver->machine != LIB_NULL && driver->machine->cartridge->mapper == mapper);
    core_driver_set_heartbeat(driver, LIB_TRUE);
    for (slice = 0u; slice < limit && !driver->machine->trap.trap_valid; ++slice)
        assert(core_driver_run(driver));
    assert(core_driver_copy_frame(driver, &frame) == LIB_STATUS_OK);
    assert(frame.window.valid != 0u && frame.window.graphics ==
        (text_output ? 0u : 1u));
    printf("mapper=%u frame=%s slices=%u instructions=%llu frames=%u pcm=%u queued=%u dropped=%llu trap=%d reason=%u pc=%04x address=%04x opcode=%02x\\n", mapper,
        text_output ? "text" : "graphics",
        slice, (unsigned long long)driver->machine->instructions,
        driver->machine->ppu.frame_revision, driver->machine->apu.sample_count,
        probe_audio_queued(driver),
        (unsigned long long)driver->machine->apu.dropped_samples,
        driver->machine->trap.trap_valid, driver->machine->trap.trap_reason,
        driver->machine->pc, driver->machine->trap.trap_address,
        driver->machine->trap.trap_opcode);
    assert(!driver->machine->trap.trap_valid);
    core_driver_set_heartbeat(driver, LIB_FALSE);
    assert(probe_audio_queued(driver) == 0u);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
}

int main(void)
{
    probe("MYNES_OWNER_NROM_ROM", 0u, LIB_FALSE);
    probe("MYNES_OWNER_MMC1_ROM", 1u, LIB_FALSE);
    probe("MYNES_OWNER_UXROM_ROM", 2u, LIB_FALSE);
    probe("MYNES_OWNER_MMC3_ROM", 4u, LIB_FALSE);
    probe("MYNES_OWNER_MMC3_SMB3_ROM", 4u, LIB_FALSE);
    probe("MYNES_OWNER_MMC3_TMNT3_ROM", 4u, LIB_FALSE);
    if (getenv("MYNES_OWNER_TEXT_FRAMES") != LIB_NULL) {
        probe("MYNES_OWNER_NROM_ROM", 0u, LIB_TRUE);
        probe("MYNES_OWNER_MMC1_ROM", 1u, LIB_TRUE);
        probe("MYNES_OWNER_UXROM_ROM", 2u, LIB_TRUE);
        probe("MYNES_OWNER_MMC3_ROM", 4u, LIB_TRUE);
        probe("MYNES_OWNER_MMC3_SMB3_ROM", 4u, LIB_TRUE);
        probe("MYNES_OWNER_MMC3_TMNT3_ROM", 4u, LIB_TRUE);
    }
    return 0;
}
