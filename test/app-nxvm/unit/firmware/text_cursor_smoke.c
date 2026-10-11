#include "../../support/profile.h"

#include "core/board-base/machine_board_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine_private.h"

extern const vm_machine_assets vm_app_firmware;

static lib_bool default_firmware_initializes_text_cursor(vm_machine *machine)
{
    for (lib_u32 instruction = 0u; instruction < 100000u; ++instruction) {
        core_machine_run_result run = {0};
        x86_video_snapshot snapshot;

        if (core_machine_run(machine->core_machine,
                (core_machine_run_budget){1u, 0u}, &run) !=
                LIB_STATUS_OK || run.reason == CORE_MACHINE_STOP_FAULT ||
            core_machine_capture_display_snapshot(machine->board, &snapshot) !=
                LIB_STATUS_OK) {
            return LIB_FALSE;
        }
        if (snapshot.kind == X86_VIDEO_KIND_TEXT &&
            snapshot.text_cell_height == 8u && snapshot.cursor_visible &&
            snapshot.cursor_top == 6u && snapshot.cursor_bottom == 7u) {
            return LIB_TRUE;
        }
    }
    return LIB_FALSE;
}

lib_i32 main(void)
{
    vm_machine_config config = {.bios_count = 1u};
    vm_machine *machine = LIB_NULL;
    lib_bool passed;

    if (vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_DEFAULT_PC_AT,
            &config, &vm_app_firmware, &machine) != LIB_STATUS_OK ||
        machine == LIB_NULL) {
        vm_machine_destroy(machine);
        return 1;
    }
    passed = default_firmware_initializes_text_cursor(machine);
    vm_machine_destroy(machine);
    return passed ? 0 : 1;
}
