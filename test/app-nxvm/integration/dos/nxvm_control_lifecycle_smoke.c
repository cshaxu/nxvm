#include "lib/types/types_interface.h"
#include <stdio.h>

#include "lib/base/sync_interface.h"
#include "test/x86-nxvm/setup/session_ini.h"
#include "test/x86-nxvm/machine/support/nxvm_presentation_capture.h"

static lib_bool wait_for_floppy_prompt(integration_ini_session *session)
{
    x86_nxvm_test_guest_display_frame frame;
    for (lib_u32 elapsed = 0u; elapsed < 10000u; elapsed += 10u) {
        if (test_nxvm_machine_capture_presentation(session->session, &frame) == LIB_STATUS_OK) {
            for (lib_size index = 0u; index + 3u < 80u * 25u; ++index) {
                if (frame.characters[index] == 'A' && frame.characters[index + 1u] == ':' &&
                    frame.characters[index + 2u] == '\\' && frame.characters[index + 3u] == '>')
                    return LIB_TRUE;
            }
        }
        base_sync_sleep_milliseconds(10u);
    }
    return LIB_FALSE;
}

int main(lib_i32 argc, char **argv)
{
    integration_ini_session ini_session;

    if (argc != 3 || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != LIB_STATUS_OK) return 77;
    if (integration_ini_session_start(&ini_session) != LIB_STATUS_OK ||
        integration_ini_session_wait_for_state(&ini_session,
            EMULATOR_MACHINE_RUNNING, 2000u) == 0 || !wait_for_floppy_prompt(&ini_session)) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    base_sync_sleep_milliseconds(5000u);
    if (integration_ini_session_pause(&ini_session, 2000u) != LIB_STATUS_OK ||
        integration_ini_session_reset(&ini_session, 2000u) != LIB_STATUS_OK) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    {
        x86_nxvm_test_guest_display_frame frame;
        lib_status s = test_nxvm_machine_capture_presentation(ini_session.session, &frame);
        if (s == LIB_STATUS_OK && frame.kind == X86_NXVM_TEST_GUEST_DISPLAY_KIND_TEXT &&
            frame.columns == 1u && frame.rows == 1u && frame.text_cell_height == 1u) {
            integration_ini_session_close(&ini_session);
            return 1;
        }
    }
    if (integration_ini_session_resume(&ini_session, 2000u) != LIB_STATUS_OK ||
        integration_ini_session_wait_for_state(&ini_session,
            EMULATOR_MACHINE_RUNNING, 2000u) == 0 || !wait_for_floppy_prompt(&ini_session)) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    base_sync_sleep_milliseconds(5000u);
    if (!emulator_machine_stop(ini_session.emulator_machine) ||
        integration_ini_session_wait_for_state(&ini_session,
            EMULATOR_MACHINE_STOPPED, 2000u) == 0 ||
        integration_ini_session_start(&ini_session) != LIB_STATUS_OK ||
        !wait_for_floppy_prompt(&ini_session)) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    integration_ini_session_close(&ini_session);
    puts("NXVM:CONTEXT-LIFECYCLE:OK");
    return 0;
}
