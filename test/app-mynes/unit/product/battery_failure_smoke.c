#include <assert.h>

#include "core/driver_interface.h"
#include "common/machine/machine_interface.h"
#include "common/session/session_interface.h"
#include "lib/storage/file_interface.h"
#include "lib/types/file.h"

static lib_status save_status;
static lib_u32 save_calls, media_calls;

static lib_status save_battery(core_driver *driver, const char *path)
{
    assert(driver != LIB_NULL && path != LIB_NULL);
    ++save_calls;
    return save_status;
}

static lib_bool set_media(common_machine *machine, const char *path,
    lib_storage_medium_mode mode)
{
    ++media_calls;
    return common_machine_set_removable_media(machine, path, mode);
}

static lib_bool session_run(common_session *session)
{
    assert(session != LIB_NULL);
    return LIB_TRUE;
}

/* Substitute only existing boundaries, to observe whether a failed save can
 * reach the destructive media operation. No production test API is added. */
#define core_driver_save_battery_ram save_battery
#define common_machine_set_removable_media set_media
#define common_session_run session_run
#include "product/composition.c"
#undef common_session_run
#undef common_machine_set_removable_media
#undef core_driver_save_battery_ram

int main(void)
{
    app_composition composition = {0};
    common_machine_driver driver;
    assert(core_driver_create(&composition.driver,
        &(core_driver_options){0}) == LIB_STATUS_OK);
    assert(core_driver_make_driver(composition.driver, &driver) == LIB_STATUS_OK);
    assert(common_machine_create(&composition.machine, &driver) == LIB_STATUS_OK);
    lib_memory_copy(composition.battery_path, "previous.sav", sizeof("previous.sav"));

    save_status = LIB_STATUS_IO_ERROR;
    assert(!app_composition_set_media(&composition, LIB_NULL));
    assert(save_calls == 1u && media_calls == 0u);
    assert(lib_text_compare((const char *)composition.battery_path, "previous.sav") == 0);
    /* Shutdown uses this same checked status, converting failure to exit 1. */
    assert(app_composition_save_battery(&composition) == LIB_STATUS_IO_ERROR);
    save_status = LIB_STATUS_OK;
    assert(app_composition_set_media(&composition, LIB_NULL));
    assert(media_calls == 1u && composition.battery_path[0] == '\0');
    assert(common_machine_shutdown(composition.machine) == LIB_STATUS_OK);
    assert(common_machine_destroy(composition.machine) == LIB_STATUS_OK);
    assert(core_driver_destroy(composition.driver) == LIB_STATUS_OK);
    {
        lib_u8 rom[16400] = {'N', 'E', 'S', 0x1au, 1u};
        lib_storage_file_writer *writer = LIB_NULL;
        app_startup_config config = {.rom_path = "battery-exit.nes"};
        rom[16] = 0xeau;
        rom[16397] = 0x80u;
        assert(lib_storage_file_writer_open((const char *)config.rom_path,
            LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
        assert(lib_storage_file_writer_write(writer, rom, sizeof(rom)) == LIB_STATUS_OK);
        assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
        save_status = LIB_STATUS_IO_ERROR;
        assert(app_composition_run(&config) == 1);
        save_status = LIB_STATUS_OK;
        assert(app_composition_run(&config) == 0);
        assert(lib_c_remove((const char *)config.rom_path) == 0);
    }
    return 0;
}
