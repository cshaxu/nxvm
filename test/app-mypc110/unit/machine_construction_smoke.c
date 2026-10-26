#include "app-mypc110/pc110_plan_interface.h"
#include "app-mypc110/pc110_clock.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "app-mypc110/pc110_font_interface.h"
#include "lib/kvm-console/frame_interface.h"
#include "lib/kvm-window/frame_interface.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "x86-core/machine/clock_interface.h"
#include "x86-debug/product/protocol_interface.h"

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
    const x86_debug_request ports[] = {
        {.operation = X86_DEBUG_READ_PORT, .port = 0x03f4u, .bytes = 1u},
        {.operation = X86_DEBUG_WRITE_PORT, .port = 0x03f2u, .address = 0x0cu, .bytes = 1u},
        {.operation = X86_DEBUG_WRITE_PORT, .port = 0x70u, .address = 0x3bu, .bytes = 1u},
        {.operation = X86_DEBUG_WRITE_PORT, .port = 0x71u, .address = 0x55u, .bytes = 1u},
        {.operation = X86_DEBUG_WRITE_PORT, .port = 0x70u, .address = 0x7bu, .bytes = 1u},
        {.operation = X86_DEBUG_WRITE_PORT, .port = 0x71u, .address = 0xaau, .bytes = 1u},
        {.operation = X86_DEBUG_READ_PORT, .port = 0x71u, .bytes = 1u},
        {.operation = X86_DEBUG_WRITE_PORT, .port = 0x70u, .address = 0x3bu, .bytes = 1u},
        {.operation = X86_DEBUG_READ_PORT, .port = 0x71u, .bytes = 1u},
        {.operation = X86_DEBUG_READ_PORT, .port = 0xfc23u, .bytes = 1u},
        {.operation = X86_DEBUG_READ_PORT, .port = 0xf023u, .bytes = 1u},
        {.operation = X86_DEBUG_READ_PORT, .port = 0xc023u, .bytes = 1u},
        {.operation = X86_DEBUG_READ_PORT, .port = 0x23u, .bytes = 1u},
        {.operation = X86_DEBUG_WRITE_PORT, .port = 0x24u, .address = 0xffu, .bytes = 1u},
        {.operation = X86_DEBUG_WRITE_PORT, .port = 0x25u, .address = 0x82u, .bytes = 1u},
        {.operation = X86_DEBUG_READ_PORT, .port = 0x704u, .bytes = 1u}};
    const lib_u32 expected[] = {0x80u, 0u, 0u, 0u, 0u, 0u, 0xaau, 0u, 0x55u,
        0xffu, 0xffu, 0xffu, 0x7fu, 0u, 0u, 0u};
    x86_debug_response response = {0};
    lib_size response_bytes = 0u;
    static kvm_window_frame_storage frame;
    static kvm_console_text_frame console;
    lib_bool produced = LIB_FALSE;
    x86_core_machine_clock_domain pit_clock;
    int failed = 0;

    if (mypc110_machine_construction_create(&config, &assets, &construction) != LIB_STATUS_OK)
        failed = 1;
    else if (construction.topology.planar_parity.refresh_status_source !=
            X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE ||
        construction.topology.planar_parity.memory_bytes != 0u ||
        construction.core_config.kbc_command_response_ticks != MYPC110_KBC_COMMAND_RESPONSE_TICKS ||
        construction.core_config.kbc_command_response_status_polls != 0u ||
        !construction.core_config.system_control_port_present)
        failed = 2;
    else if (!construction.topology.rtc_cmos.extended_registers ||
        !construction.topology.fdc_present || !construction.topology.dma_present ||
        construction.topology.fdc.ready_mask != 0x0fu ||
        construction.topology.fdc_drives.double_sided_mask != 0x0fu ||
        construction.topology.fdc_drives.installed_mask != 0u)
        failed = 9;
    else if (construction.fixed_disk_slot_count != 2u)
        failed = 42;
    else if (x86_core_machine_clock_domain_initialize(&pit_clock,
            &construction.core_config.clock_plan.pit) != LIB_STATUS_OK)
        failed = 7;
    else if (x86_core_machine_clock_domain_advance(&pit_clock,
            320u * construction.topology.planar_parity.refresh_status_toggle_ticks)
            < 0x1614u || pit_clock.delivered_ticks > 0x1868u)
        failed = 8;
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
    for (lib_size index = 0u; !failed && index < sizeof(ports) / sizeof(ports[0]); ++index) {
        if (driver.execute_debug(driver.context, &ports[index], sizeof(ports[index]),
                &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
                response_bytes != sizeof(response) || response.value != expected[index] ||
                construction.floppy_slot_count != 0u || information.floppy_media_inserted)
            failed = 20 + (int)index;
    }
    if (!failed) {
        kvm_input_event key = {.type = KVM_EVENT_KEY,
            .data.key = {.key = BASE_KEY_F12, .pressed = LIB_TRUE}};
        const x86_debug_request refresh = {.operation = X86_DEBUG_READ_PORT,
            .port = 0x704u, .bytes = 1u};
        driver.deliver_input(driver.context, &key);
        key.data.key.pressed = LIB_FALSE;
        driver.deliver_input(driver.context, &key);
        if (driver.execute_debug(driver.context, &refresh, sizeof(refresh),
                &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
            response.value != 1u) failed = 40;
    }
    if (!failed) {
        const x86_debug_request select = {.operation = X86_DEBUG_WRITE_PORT,
            .port = 0x03e0u, .address = 0x18u, .bytes = 1u};
        const x86_debug_request write = {.operation = X86_DEBUG_WRITE_PORT,
            .port = 0x03e1u, .address = 0x5au, .bytes = 1u};
        const x86_debug_request read = {.operation = X86_DEBUG_READ_PORT,
            .port = 0x03e1u, .bytes = 1u};

        if (driver.execute_debug(driver.context, &select, sizeof(select),
                &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
            driver.execute_debug(driver.context, &write, sizeof(write),
                &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
            !driver.reset(driver.context) ||
            driver.execute_debug(driver.context, &select, sizeof(select),
                &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
            driver.execute_debug(driver.context, &read, sizeof(read),
                &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
            response.value != 0u) failed = 41;
    }
    x86_nxvm_machine_destroy(machine);
    return failed;
}
