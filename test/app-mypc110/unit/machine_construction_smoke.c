#include "app-mypc110/pc110_plan_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "app-mypc110/pc110_font_interface.h"
#include "lib/kvm-console/frame_interface.h"
#include "lib/kvm-window/frame_interface.h"
#include "x86-nxvm/machine/machine_interface.h"

int main(void)
{
    static lib_u8 bios[MYPC110_FIRMWARE_BYTES];
    static lib_u8 font[MYPC110_FONT_ROM_BYTES];
    const x86_nxvm_machine_assets assets = {
        .bios = {{bios, sizeof(bios)}, {LIB_NULL, 0u}},
        .font = {font, sizeof(font)}};
    const x86_nxvm_machine_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .floppy_format = X86_NXVM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT,
        .bios_count = 1u};
    x86_nxvm_machine_construction construction = {0};
    x86_nxvm_machine *machine = LIB_NULL;
    x86_nxvm_machine_reset_vector reset = {0};
    x86_nxvm_machine_information information = {0};
    emulator_machine_driver driver = {0};
    static kvm_window_frame_storage frame;
    static kvm_console_text_frame console;
    lib_bool produced = LIB_FALSE;
    int failed = 0;

    if (mypc110_machine_construction_create(&config, &assets, &construction) != LIB_STATUS_OK)
        failed = 1;
    else if (construction.topology.planar_parity.refresh_status_source !=
            X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE ||
        construction.topology.planar_parity.refresh_status_toggle_ticks != 3u ||
        construction.topology.planar_parity.memory_bytes != 0u)
        failed = 2;
    else {
        lib_status status = x86_nxvm_machine_create(&config, &construction, &machine);
        if (status != LIB_STATUS_OK) failed = 10 + (int)status;
    }
    if (!failed && x86_nxvm_machine_get_reset_vector(machine, &reset) != LIB_STATUS_OK)
        failed = 3;
    else if (!failed && (x86_nxvm_machine_get_information(machine, &information) !=
            LIB_STATUS_OK || information.memory_bytes != 20u * 1024u * 1024u ||
            !information.fixed_disk_present))
        failed = 6;
    else if (!failed && (reset.cs != 0xf000u || reset.ip != 0xfff0u))
        failed = 4;
    else if (!failed && (x86_nxvm_machine_describe_emulator_driver(machine, &driver) !=
            LIB_STATUS_OK || driver.copy_frame(driver.context, &frame, &console,
                &produced) != LIB_STATUS_OK || !produced))
        failed = 5;
    x86_nxvm_machine_destroy(machine);
    return failed;
}
