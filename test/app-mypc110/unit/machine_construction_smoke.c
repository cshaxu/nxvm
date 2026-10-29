#include "app-mypc110/pc110_plan_interface.h"
#include "app-mypc110/pc110_cmos.h"
#include "app-mypc110/pc110_clock.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "app-mypc110/pc110_font_interface.h"
#include "app-mypc110/pc110_display_interface.h"
#include "lib/storage/file_interface.h"
#include "lib/types/file.h"
#include "lib/kvm-console/frame_interface.h"
#include "lib/kvm-window/frame_interface.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "x86-nxvm/chips/f65535/f65535_interface.h"
#include "x86-core/machine/clock_interface.h"
#include "x86-debug/product/protocol_interface.h"

static const lib_u8 mypc110_rtc_calendar[][2] = {
    {0u, 0x45u}, {2u, 0x59u}, {4u, 0x23u}, {6u, 3u},
    {7u, 0x28u}, {8u, 0x10u}, {9u, 0x24u}
};

static lib_bool mypc110_test_port(emulator_machine_driver *driver,
    lib_bool write, lib_u16 port, lib_u8 *value)
{
    const x86_debug_request request = {
        .operation = write ? X86_DEBUG_WRITE_PORT : X86_DEBUG_READ_PORT,
        .port = port, .address = *value, .bytes = 1u};
    x86_debug_response response = {0};
    lib_size response_bytes = 0u;
    const lib_status status = driver->execute_debug(driver->context, &request,
        sizeof(request), &response, sizeof(response), &response_bytes);
    if (status != LIB_STATUS_OK || response_bytes != sizeof(response)) return LIB_FALSE;
    if (!write) *value = (lib_u8)response.value;
    return LIB_TRUE;
}

static lib_bool mypc110_test_memory(emulator_machine_driver *driver,
    lib_bool write, lib_u32 address, lib_u8 *value)
{
    const x86_debug_request request = {
        .operation = write ? X86_DEBUG_WRITE_LINEAR : X86_DEBUG_READ_LINEAR,
        .address = address, .bytes = 1u, .data = {*value}};
    x86_debug_response response = {0};
    lib_size response_bytes = 0u;
    const lib_status status = driver->execute_debug(driver->context, &request,
        sizeof(request), &response, sizeof(response), &response_bytes);
    if (status != LIB_STATUS_OK || response_bytes != sizeof(response)) return LIB_FALSE;
    if (!write) *value = response.data[0u];
    return LIB_TRUE;
}

static lib_bool mypc110_test_route_writes(emulator_machine_driver *driver,
    const lib_u16 (*writes)[2], lib_size count)
{
    for (lib_size i = 0u; i < count; ++i) {
        lib_u8 value = (lib_u8)writes[i][1];
        if (!mypc110_test_port(driver, LIB_TRUE, writes[i][0], &value)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool mypc110_test_product_routes(emulator_machine_driver *driver,
    const x86_nxvm_machine_construction *construction)
{
    static x86_video_snapshot snapshot;
    static const lib_u16 text_setup[][2] = {
        {0x03ceu, 8u}, {0x03cfu, 0xffu}, {0x03ceu, 5u}, {0x03cfu, 0x10u},
        {0x03c4u, 2u}, {0x03c5u, 3u}, {0x03ceu, 6u}, {0x03cfu, 0x0eu},
        {0x03c4u, 4u}, {0x03c5u, 2u}, {0x03d4u, 1u}, {0x03d5u, 0x4fu},
        {0x03d4u, 7u}, {0x03d5u, 0x1fu}, {0x03d4u, 9u}, {0x03d5u, 0x0fu},
        {0x03d4u, 18u}, {0x03d5u, 0x8fu}};
    static const lib_u16 graphics_setup[][2] = {
        {0x03ceu, 6u}, {0x03cfu, 5u}, {0x03c4u, 4u}, {0x03c5u, 4u},
        {0x03d4u, 1u}, {0x03d5u, 79u}, {0x03d4u, 7u}, {0x03d5u, 2u},
        {0x03d4u, 18u}, {0x03d5u, 0xdfu}, {0x03d4u, 19u}, {0x03d5u, 40u},
        {0x03c4u, 2u}, {0x03c5u, 15u}};
    const lib_u32 absent[] = {0x01400000u, 0x000c0000u, 0x000d0000u};
    const lib_u16 ports[] = {0x20u, 0x40u, 0x60u, 0x64u, 0x92u, 0x3dau};
    const lib_u8 pattern[] = {0x55u, 0xaau, 0x55u, 0xaau};
    lib_u8 value = 0u;

    if (!driver->reset(driver->context)) return LIB_FALSE;
    for (lib_size i = 0u; i < sizeof(pattern); ++i) {
        value = pattern[i];
        if (!mypc110_test_memory(driver, LIB_TRUE, 0x013f0000u + (lib_u32)i, &value))
            return LIB_FALSE;
        value = 0u;
        if (!mypc110_test_memory(driver, LIB_FALSE, 0x013f0000u + (lib_u32)i, &value) ||
            value != pattern[i]) return LIB_FALSE;
    }
    for (lib_size i = 0u; i < sizeof(ports) / sizeof(ports[0]); ++i) {
        if (!mypc110_test_port(driver, LIB_FALSE, ports[i], &value) ||
            (ports[i] == 0x92u && value != 0u)) return LIB_FALSE;
    }
    for (lib_size i = 0u; i < sizeof(absent) / sizeof(absent[0]); ++i) {
        value = 0u;
        if (!mypc110_test_memory(driver, LIB_FALSE, absent[i], &value) || value != 0xffu)
            return LIB_FALSE;
        value = 0x44u;
        if (!mypc110_test_memory(driver, LIB_TRUE, absent[i], &value)) return LIB_FALSE;
        value = 0u;
        if (!mypc110_test_memory(driver, LIB_FALSE, absent[i], &value) || value != 0xffu)
            return LIB_FALSE;
    }
    for (lib_u16 port = 0x80u; port <= 0x8fu; ++port) {
        value = 0u;
        if (!mypc110_test_port(driver, LIB_FALSE, port, &value) || value != 0u) return LIB_FALSE;
        value = (lib_u8)(port ^ 0x5au);
        if (!mypc110_test_port(driver, LIB_TRUE, port, &value)) return LIB_FALSE;
        value = 0u;
        if (!mypc110_test_port(driver, LIB_FALSE, port, &value) || value != (lib_u8)(port ^ 0x5au))
            return LIB_FALSE;
    }
    if (!driver->reset(driver->context)) return LIB_FALSE;
    for (lib_u16 port = 0x80u; port <= 0x8fu; ++port) {
        value = 0xffu;
        if (!mypc110_test_port(driver, LIB_FALSE, port, &value) || value != 0u) return LIB_FALSE;
    }
    value = 0u;
    if (!mypc110_test_memory(driver, LIB_FALSE, MYPC110_FONT_PHYSICAL_START, &value) || value != 0x55u)
        return LIB_FALSE;
    value = 0x81u;
    if (!mypc110_test_port(driver, LIB_TRUE, MYPC110_FONT_BANK_PORT, &value)) return LIB_FALSE;
    value = 0u;
    if (!mypc110_test_memory(driver, LIB_FALSE, MYPC110_FONT_PHYSICAL_START, &value) || value != 0xaau)
        return LIB_FALSE;

    value = 0u;
    if (!mypc110_test_port(driver, LIB_TRUE, X86_F65535_EXTENSION_INDEX_PORT, &value) ||
        !mypc110_test_port(driver, LIB_FALSE, X86_F65535_EXTENSION_DATA_PORT, &value) ||
        value != X86_F65535_PC110_CHIP_VERSION ||
        !mypc110_test_route_writes(driver, text_setup, sizeof(text_setup) / sizeof(text_setup[0])))
        return LIB_FALSE;
    value = 0x5au;
    if (!mypc110_test_memory(driver, LIB_TRUE, 0x000b8000u, &value) ||
        construction->display_snapshot_provider(construction->display_snapshot_context, &snapshot) !=
            LIB_STATUS_OK || snapshot.columns != 80u || snapshot.rows != 25u ||
        snapshot.characters[0] != 0x5au ||
        !mypc110_test_route_writes(driver, graphics_setup, sizeof(graphics_setup) / sizeof(graphics_setup[0])))
        return LIB_FALSE;
    value = 0x5au;
    return mypc110_test_memory(driver, LIB_TRUE, 0x000a0000u, &value) &&
        construction->display_snapshot_provider(construction->display_snapshot_context, &snapshot) ==
            LIB_STATUS_OK && snapshot.kind == X86_VIDEO_KIND_VGA_PLANAR16 &&
        snapshot.pixel_width == 640u && snapshot.pixel_height == 480u && snapshot.pixels[1] == 15u;
}

static lib_status mypc110_write_zero_image(const char *path, lib_size bytes)
{
    static const lib_u8 zeroes[4096u];
    lib_storage_file_writer *writer = LIB_NULL;
    lib_status status = lib_storage_file_writer_open(path,
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer);

    while (status == LIB_STATUS_OK && bytes != 0u) {
        const lib_size chunk = bytes < sizeof(zeroes) ? bytes : sizeof(zeroes);
        status = lib_storage_file_writer_write(writer, zeroes, chunk);
        bytes -= chunk;
    }
    if (writer != LIB_NULL) {
        const lib_status closed = lib_storage_file_writer_close(writer);
        if (status == LIB_STATUS_OK) status = closed;
    }
    return status;
}

static int mypc110_check_master_geometry(const x86_nxvm_machine_assets *assets)
{
    static const char exact_path[] = "mypc110-primary-geometry.img";
    static const char short_path[] = "mypc110-primary-geometry-short.img";
    const x86_nxvm_machine_config exact = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .bios_count = 1u,
        .fixed_disk_image = {exact_path, LIB_NULL},
        .fixed_disk_mode = {LIB_STORAGE_MEDIUM_OVERLAY, LIB_STORAGE_MEDIUM_OVERLAY}};
    const x86_nxvm_machine_config short_image = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .bios_count = 1u,
        .fixed_disk_image = {short_path, LIB_NULL},
        .fixed_disk_mode = {LIB_STORAGE_MEDIUM_OVERLAY, LIB_STORAGE_MEDIUM_OVERLAY}};
    const lib_size exact_bytes = 123u * 2u * 32u * 512u;
    x86_nxvm_machine_construction construction = {0};
    x86_nxvm_machine *machine = LIB_NULL;
    x86_nxvm_machine_information information = {0};
    int failed = 0;

    if (mypc110_write_zero_image(exact_path, exact_bytes) != LIB_STATUS_OK ||
        mypc110_write_zero_image(short_path, 512u) != LIB_STATUS_OK) failed = 1;
    if (!failed && mypc110_machine_construction_create(&exact, assets,
            &construction) != LIB_STATUS_OK) failed = 2;
    if (!failed && x86_nxvm_machine_create(&exact, &construction, &machine) !=
            LIB_STATUS_OK) failed = 3;
    if (!failed && (x86_nxvm_machine_get_information(machine, &information) !=
            LIB_STATUS_OK || information.fixed_disk_cylinders != 123u)) failed = 4;
    x86_nxvm_machine_destroy(machine);
    machine = LIB_NULL;
    if (!failed && mypc110_machine_construction_create(&short_image, assets,
            &construction) != LIB_STATUS_OK) failed = 5;
    if (!failed && x86_nxvm_machine_create(&short_image, &construction, &machine) !=
            LIB_STATUS_INVALID_ARGUMENT) failed = 6;
    x86_nxvm_machine_destroy(machine);
    if (lib_c_remove(exact_path) != 0 && !failed) failed = 7;
    if (lib_c_remove(short_path) != 0 && !failed) failed = 7;
    return failed;
}

int main(int argc, char **argv)
{
    static lib_u8 bios[MYPC110_FIRMWARE_BYTES];
    static lib_u8 font[MYPC110_FONT_ROM_BYTES];
    static lib_u8 cmos[X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
    static lib_u8 captured[X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
    static lib_u8 loaded[X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
    lib_u8 rtc_snapshot[X86_NXVM_MACHINE_RTC_SEED_BYTES] = {0};
    const lib_bool save_for_restart = argc == 3 &&
        lib_text_compare(argv[1], "--save-cmos") == 0;
    const lib_bool check_restart = argc == 3 &&
        lib_text_compare(argv[1], "--check-cmos") == 0;
    const lib_bool check_bios = argc == 3 &&
        lib_text_compare(argv[1], "--check-bios-cmos") == 0;
    const char *cmos_path = (save_for_restart || check_restart || check_bios) ? argv[2] :
        "mypc110-test-configuration.cmos";
    lib_bool cmos_present = LIB_FALSE;
    const x86_nxvm_machine_assets assets = {
        .bios = {{bios, sizeof(bios)}, {LIB_NULL, 0u}},
        .cmos_seed = {cmos, sizeof(cmos)},
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
        {.operation = X86_DEBUG_READ_PORT, .port = 0x704u, .bytes = 1u},
        {.operation = X86_DEBUG_READ_PORT, .port = 0x15e1u, .bytes = 1u},
        {.operation = X86_DEBUG_READ_PORT, .port = 0x15e2u, .bytes = 1u}};
    const lib_u32 expected[] = {0x80u, 0u, 0u, 0u, 0u, 0u, 0xaau, 0u, 0x55u,
        0xffu, 0xffu, 0xffu, 0x7fu, 0u, 0u, 0u, 0xffu, 0xc0u};
    x86_debug_response response = {0};
    lib_size response_bytes = 0u;
    static kvm_window_frame_storage frame;
    static kvm_console_text_frame console;
    lib_bool produced = LIB_FALSE;
    x86_core_machine_clock_domain pit_clock;
    int failed = 0;

    if (argc != 1 && !save_for_restart && !check_restart && !check_bios) return 1;
    if (check_bios) {
        lib_u16 sum = 0u;
        if (mypc110_cmos_load(cmos_path, loaded, &cmos_present) != LIB_STATUS_OK ||
            !cmos_present) return 1;
        for (lib_size index = 0x10u; index < 0x2eu; ++index)
            sum = (lib_u16)(sum + loaded[index]);
        lib_c_printf("PC110:CMOS-RESTART:sum=%04x stored=%02x%02x\n",
            sum, loaded[0x2eu], loaded[0x2fu]);
        return sum == (lib_u16)(((lib_u16)loaded[0x2eu] << 8u) | loaded[0x2fu]) ? 0 : 1;
    }
    if (check_restart) {
        x86_nxvm_machine_assets recovered = assets;
        if (mypc110_cmos_load(cmos_path, loaded, &cmos_present) != LIB_STATUS_OK ||
            !cmos_present || loaded[0x0eu] != 0xe1u || loaded[0x3bu] != 0x55u ||
            loaded[0x7bu] != 0xaau || loaded[0x7fu] != 0x7eu || loaded[0x0du] != 0x80u)
            return 1;
        recovered.cmos_seed.data = loaded;
        if (mypc110_machine_construction_create(&config, &recovered, &construction) != LIB_STATUS_OK)
            return 1;
        lib_memory_copy(construction.rtc_seed, loaded, sizeof(construction.rtc_seed));
        construction.rtc_seed_present = LIB_TRUE;
        if (x86_nxvm_machine_create(&config, &construction, &machine) != LIB_STATUS_OK ||
            x86_nxvm_machine_describe_emulator_driver(machine, &driver) != LIB_STATUS_OK) {
            x86_nxvm_machine_destroy(machine);
            return 1;
        }
        for (lib_size i = 0u; i < sizeof(mypc110_rtc_calendar) /
                sizeof(mypc110_rtc_calendar[0]); ++i) {
            lib_u8 index = mypc110_rtc_calendar[i][0], value = 0u;
            if (!mypc110_test_port(&driver, LIB_TRUE, 0x70u, &index) ||
                !mypc110_test_port(&driver, LIB_FALSE, 0x71u, &value) ||
                value != mypc110_rtc_calendar[i][1]) {
                x86_nxvm_machine_destroy(machine);
                return 1;
            }
        }
        x86_nxvm_machine_destroy(machine);
        return 0;
    }
    font[0] = 0x55u;
    font[MYPC110_FONT_BANK_BYTES] = 0xaau;
    cmos[0x0eu] = 0xe1u;
    cmos[0x3fu] = 0xf3u;
    cmos[0x40u] = 0x44u;
    cmos[0x7fu] = 0x7eu;
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
        !construction.cmos_seed_present ||
        construction.cmos_seed_bytes != X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES ||
        !construction.topology.fdc_present || !construction.topology.dma_present ||
        construction.topology.fdc.ready_mask != 0x0fu ||
        construction.topology.fdc_drives.double_sided_mask != 0x0fu ||
        construction.topology.fdc_drives.installed_mask != 0u)
        failed = 9;
    else if (construction.fixed_disk_slot_count != 2u || !construction.fixed_geometry ||
        construction.cylinders != 123u || construction.heads != 2u ||
        construction.sectors != 32u)
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
    if (!failed) {
        const x86_debug_request cmos_reads[] = {
            {.operation = X86_DEBUG_WRITE_PORT, .port = 0x70u, .address = 0x0eu, .bytes = 1u},
            {.operation = X86_DEBUG_READ_PORT, .port = 0x71u, .bytes = 1u},
            {.operation = X86_DEBUG_WRITE_PORT, .port = 0x70u, .address = 0x3fu, .bytes = 1u},
            {.operation = X86_DEBUG_READ_PORT, .port = 0x71u, .bytes = 1u},
            {.operation = X86_DEBUG_WRITE_PORT, .port = 0x70u, .address = 0x40u, .bytes = 1u},
            {.operation = X86_DEBUG_READ_PORT, .port = 0x71u, .bytes = 1u},
            {.operation = X86_DEBUG_WRITE_PORT, .port = 0x70u, .address = 0x7fu, .bytes = 1u},
            {.operation = X86_DEBUG_READ_PORT, .port = 0x71u, .bytes = 1u}};
        const lib_u32 expected_cmos[] = {0u, 0xe1u, 0u, 0xf3u, 0u, 0x44u, 0u, 0x7eu};

        for (lib_size index = 0u; index < sizeof(cmos_reads) / sizeof(cmos_reads[0]); ++index) {
            if (driver.execute_debug(driver.context, &cmos_reads[index], sizeof(cmos_reads[index]),
                    &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
                response_bytes != sizeof(response) || response.value != expected_cmos[index]) {
                failed = 50 + (int)index;
                break;
            }
        }
    }
    for (lib_size index = 0u; !failed && index < sizeof(ports) / sizeof(ports[0]); ++index) {
        if (driver.execute_debug(driver.context, &ports[index], sizeof(ports[index]),
                &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
                response_bytes != sizeof(response) || response.value != expected[index] ||
                construction.floppy_slot_count != 0u || information.floppy_media_inserted)
            failed = 20 + (int)index;
    }
    if (!failed) {
        const x86_debug_request enable = {.operation = X86_DEBUG_WRITE_PORT,
            .port = 0x15e2u, .address = 0x38u, .bytes = 1u};
        const x86_debug_request read = {.operation = X86_DEBUG_READ_PORT,
            .port = 0x15e0u, .bytes = 1u};
        x86_debug_request ack = {.operation = X86_DEBUG_WRITE_PORT,
            .port = 0x15e2u, .bytes = 1u};
        kvm_input_event mouse = {.type = KVM_EVENT_MOUSE,
            .data.mouse = {.relative = LIB_TRUE, .delta_x = 3, .delta_y = -4,
                .buttons = BASE_MOUSE_BUTTON_LEFT}};
        const lib_u8 expected_inking[] = {0x91u, 3u, 60u, 0x90u, 3u, 60u};
        if (driver.execute_debug(driver.context, &enable, sizeof(enable), &response,
                sizeof(response), &response_bytes) != LIB_STATUS_OK) failed = 70;
        driver.deliver_input(driver.context, &mouse);
        mouse.data.mouse.delta_x = mouse.data.mouse.delta_y = 0;
        mouse.data.mouse.buttons = 0u;
        driver.deliver_input(driver.context, &mouse);
        for (lib_size i = 0u; !failed && i < sizeof(expected_inking); ++i) {
            if (driver.execute_debug(driver.context, &read, sizeof(read), &response,
                    sizeof(response), &response_bytes) != LIB_STATUS_OK ||
                response.value != expected_inking[i]) { failed = 71; break; }
            ack.address = 0x39u;
            if (driver.execute_debug(driver.context, &ack, sizeof(ack), &response,
                    sizeof(response), &response_bytes) != LIB_STATUS_OK) { failed = 72; break; }
            ack.address = 0x38u;
            if (driver.execute_debug(driver.context, &ack, sizeof(ack), &response,
                    sizeof(response), &response_bytes) != LIB_STATUS_OK) failed = 73;
        }
    }
    if (!failed) {
        for (lib_size i = 0u; i < sizeof(mypc110_rtc_calendar) /
                sizeof(mypc110_rtc_calendar[0]); ++i) {
            lib_u8 index = mypc110_rtc_calendar[i][0];
            lib_u8 value = mypc110_rtc_calendar[i][1];
            if (!mypc110_test_port(&driver, LIB_TRUE, 0x70u, &index) ||
                !mypc110_test_port(&driver, LIB_TRUE, 0x71u, &value)) failed = 75;
        }
    }
    if (!failed && (x86_nxvm_machine_capture_cmos(machine, captured, sizeof(captured) - 1u) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_machine_capture_cmos(machine, captured, sizeof(captured)) != LIB_STATUS_OK ||
        captured[0x0du] != 0u || captured[0x0eu] != 0xe1u ||
        captured[0x3bu] != 0x55u || captured[0x7bu] != 0xaau ||
        captured[0x7fu] != 0x7eu)) failed = 60;
    if (!failed && x86_nxvm_machine_capture_rtc(machine, rtc_snapshot,
            sizeof(rtc_snapshot)) != LIB_STATUS_OK) failed = 76;
    if (!failed && (mypc110_cmos_load(cmos_path, loaded, &cmos_present) != LIB_STATUS_OK ||
        cmos_present || mypc110_cmos_save(cmos_path, machine) != LIB_STATUS_OK ||
        mypc110_cmos_load(cmos_path, loaded, &cmos_present) != LIB_STATUS_OK ||
        !cmos_present || loaded[0x0du] != 0x80u || loaded[9u] != 0x24u ||
        lib_memory_compare(captured + 0x0eu, loaded + 0x0eu,
            sizeof(captured) - 0x0eu) != 0))
        failed = 62;
    if (!failed && !save_for_restart) {
        lib_storage_file_writer *writer = LIB_NULL;
        lib_status status = lib_storage_file_writer_open(cmos_path,
            LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer);
        if (status == LIB_STATUS_OK) status = lib_storage_file_writer_write(writer, captured, 1u);
        if (writer != LIB_NULL && lib_storage_file_writer_close(writer) != LIB_STATUS_OK)
            status = LIB_STATUS_IO_ERROR;
        if (status != LIB_STATUS_OK ||
            mypc110_cmos_load(cmos_path, loaded, &cmos_present) != LIB_STATUS_INVALID_ARGUMENT ||
            mypc110_cmos_load("missing-pc110-test-dir/state.cmos", loaded, &cmos_present) !=
                LIB_STATUS_IO_ERROR ||
            mypc110_cmos_save("missing-pc110-test-dir/state.cmos", machine) !=
                LIB_STATUS_IO_ERROR ||
            x86_nxvm_machine_capture_cmos(machine, loaded, sizeof(loaded)) !=
                LIB_STATUS_OK) failed = 64;
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
        lib_u8 rtc_index = 0x09u;
        lib_u8 rtc_year = 0x24u;

        if (!mypc110_test_port(&driver, LIB_TRUE, 0x70u, &rtc_index) ||
            !mypc110_test_port(&driver, LIB_TRUE, 0x71u, &rtc_year) ||
            driver.execute_debug(driver.context, &select, sizeof(select),
                &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
            driver.execute_debug(driver.context, &write, sizeof(write),
                &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
            !driver.reset(driver.context) ||
            driver.execute_debug(driver.context, &select, sizeof(select),
                &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
            driver.execute_debug(driver.context, &read, sizeof(read),
                &response, sizeof(response), &response_bytes) != LIB_STATUS_OK ||
            response.value != 0u ||
            !mypc110_test_port(&driver, LIB_TRUE, 0x70u, &rtc_index) ||
            !mypc110_test_port(&driver, LIB_FALSE, 0x71u, &rtc_year) ||
            rtc_year != 0x24u) failed = 41;
    }
    if (!failed && !mypc110_test_product_routes(&driver, &construction)) failed = 74;
    x86_nxvm_machine_destroy(machine);
    machine = LIB_NULL;
    if (!failed) {
        x86_nxvm_machine_assets recovered = assets;
        lib_u8 recaptured[sizeof(captured)];

        recovered.cmos_seed.data = captured;
        if (mypc110_machine_construction_create(&config, &recovered, &construction) !=
                LIB_STATUS_OK) failed = 61;
        if (!failed) {
            lib_memory_copy(construction.rtc_seed, rtc_snapshot, sizeof(rtc_snapshot));
            construction.rtc_seed_present = LIB_TRUE;
        }
        if (!failed && (x86_nxvm_machine_create(&config, &construction, &machine) != LIB_STATUS_OK ||
            x86_nxvm_machine_capture_cmos(machine, recaptured, sizeof(recaptured)) !=
                LIB_STATUS_OK ||
            lib_memory_compare(captured, recaptured, sizeof(captured)) != 0)) failed = 61;
        if (!failed && (x86_nxvm_machine_capture_rtc(machine, rtc_snapshot,
                sizeof(rtc_snapshot)) != LIB_STATUS_OK || rtc_snapshot[9u] != 0x24u))
            failed = 77;
        x86_nxvm_machine_destroy(machine);
    }
    if (!save_for_restart && lib_c_remove(cmos_path) != 0 && !failed) failed = 63;
    if (!failed) failed = mypc110_check_master_geometry(&assets);
    return failed;
}
