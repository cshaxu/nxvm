#include <assert.h>

#include "core/driver_interface.h"
#include "emulator/machine/machine_interface.h"
#include "emulator/product/composition_interface.h"
#include "emulator/session/session_interface.h"
#include "lib/types/file.h"

static lib_status save_status;
static lib_u32 save_calls, media_calls;

static lib_status save_battery(core_driver *driver, const char *path)
{
    assert(driver != LIB_NULL && path != LIB_NULL);
    ++save_calls;
    return save_status;
}

static lib_bool set_media(emulator_machine *machine, const char *path,
    lib_storage_medium_mode mode)
{
    ++media_calls;
    return emulator_machine_set_removable_media(machine, path, mode);
}

/* Substitute only existing boundaries, to observe whether a failed save can
 * reach the destructive media operation. No production test API is added. */
#define core_driver_save_battery_ram save_battery
#define emulator_machine_set_removable_media set_media
#include "product/composition.c"
#undef emulator_machine_set_removable_media
#undef core_driver_save_battery_ram


int main(void)
{
    app_composition composition = {0};
    emulator_machine_driver driver;
    assert(core_driver_create(&composition.driver,
        &(core_driver_options){0}) == LIB_STATUS_OK);
    assert(core_driver_make_driver(composition.driver, &driver) == LIB_STATUS_OK);
    assert(emulator_product_create(&(emulator_product_machine){
        .machine = &composition,
        .driver = driver,
        .bind = app_composition_bind_machine,
        .destroy = app_composition_destroy_machine},
        &composition.product) == LIB_STATUS_OK);
    assert(emulator_product_compose_machine(composition.product) == LIB_STATUS_OK);
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
    assert(emulator_product_destroy(composition.product) == LIB_STATUS_OK);
    composition.product = LIB_NULL;
    return 0;
}
