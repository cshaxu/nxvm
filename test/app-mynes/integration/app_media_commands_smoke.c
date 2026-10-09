#include <assert.h>

#include "product/command.h"
#include "emulator/machine/machine_interface.h"
#include "emulator/session/control_state.h"
#include "core/driver_interface.h"
#include "lib/storage/file_interface.h"

#define FIXTURE_PATH "mynes-app-media-fixture.nes"

static void write_fixture(void)
{
    lib_u8 bytes[16u + 16384u];
    lib_storage_file_writer *writer = LIB_NULL;

    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    bytes[16u] = 0xeau;
    bytes[16u + 0x3ffcu] = 0u; bytes[16u + 0x3ffdu] = 0x80u;
    assert(lib_storage_file_writer_open(FIXTURE_PATH, LIB_STORAGE_FILE_WRITER_TRUNCATE,
        &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, bytes, sizeof(bytes)) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

int main(void)
{
    core_driver *driver = LIB_NULL;
    emulator_machine *machine = LIB_NULL;
    emulator_machine_driver emulator_driver;
    app_command_context command;
    emulator_session_command_result result;

    write_fixture();
    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_make_driver(driver, &emulator_driver) == LIB_STATUS_OK);
    assert(emulator_machine_create(&machine, &emulator_driver) == LIB_STATUS_OK);
    app_command_initialize(&command, machine, LIB_FALSE,
        EMULATOR_SESSION_DISPLAY_WINDOW);

    /* Common begins monitoring at INIT and publishes the actual stopped
     * state asynchronously. App opens cartridge management after that
     * authoritative completion, rather than fabricating a lifecycle state. */
    {
        emulator_session_state session_state;
        emulator_session_state_initialize(&session_state,
            EMULATOR_SESSION_DISPLAY_CONSOLE, LIB_TRUE);
        assert(session_state.monitor_actual == EMULATOR_SESSION_MACHINE_INIT);
        app_command_note_runtime(&command, session_state.monitor_actual,
            EMULATOR_SESSION_MACHINE_STOPPED, &result);
        session_state.monitor_actual = EMULATOR_SESSION_MACHINE_STOPPED;
        app_command_submit_line(&command, session_state.monitor_actual,
            "start", &result);
        assert(lib_text_compare(result.text,
            "Insert a cartridge before starting.\r\n") == 0);
        app_command_submit_line(&command, session_state.monitor_actual,
            "rom insert mynes-app-media-fixture.nes", &result);
        assert(result.request == EMULATOR_SESSION_REQUEST_RESET);
        assert(command.cartridge_present);
        app_command_note_runtime(&command, session_state.monitor_actual,
            EMULATOR_SESSION_MACHINE_STOPPED, &result);
        assert(emulator_machine_set_removable_media(machine, LIB_NULL,
            LIB_STORAGE_MEDIUM_READONLY));
        app_command_initialize(&command, machine, LIB_FALSE,
            EMULATOR_SESSION_DISPLAY_WINDOW);
    }

    app_command_submit_line(&command, EMULATOR_SESSION_MACHINE_STOPPED,
        "rom insert mynes-app-media-fixture.nes", &result);
    assert(result.request == EMULATOR_SESSION_REQUEST_RESET && !result.arm_prompt &&
        core_driver_has_cartridge(driver));
    app_command_note_runtime(&command, EMULATOR_SESSION_MACHINE_STOPPED,
        EMULATOR_SESSION_MACHINE_RESET_COMPLETED, &result);
    app_command_submit_line(&command, EMULATOR_SESSION_MACHINE_STOPPED,
        "rom insert mynes-missing-media-fixture.nes", &result);
    assert(lib_text_compare(result.text, "Cartridge insertion failed.\r\n") == 0 &&
        result.request == EMULATOR_SESSION_REQUEST_NONE && core_driver_has_cartridge(driver));
    app_command_submit_line(&command, EMULATOR_SESSION_MACHINE_RUNNING,
        "rom eject", &result);
    assert(lib_text_compare(result.text, "Stop or pause before ejecting the cartridge.\r\n") == 0 &&
        core_driver_has_cartridge(driver));
    app_command_submit_line(&command, EMULATOR_SESSION_MACHINE_PAUSED, "rom eject", &result);
    assert(result.request == EMULATOR_SESSION_REQUEST_STOP && !result.arm_prompt &&
        !core_driver_has_cartridge(driver));
    assert(emulator_machine_shutdown(machine) == LIB_STATUS_OK);
    assert(emulator_machine_destroy(machine) == LIB_STATUS_OK);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
    return 0;
}
