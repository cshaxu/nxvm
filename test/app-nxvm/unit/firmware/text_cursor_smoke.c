#include "../../support/profile.h"

#include "core/board-base/machine_board_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine_private.h"
#include "core/x86/debug_interface.h"

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
            snapshot.cursor_top == 7u && snapshot.cursor_bottom == 7u) {
            return LIB_TRUE;
        }
    }
    return LIB_FALSE;
}

static lib_bool default_firmware_initializes_ega_graphics(vm_machine *machine)
{
    static const struct {
        lib_u8 mode;
        x86_video_kind kind;
        lib_u16 width;
        lib_u16 height;
    } modes[] = {
        {0x0du, X86_VIDEO_KIND_EGA_320X200X16, 320u, 200u},
        {0x0eu, X86_VIDEO_KIND_EGA_640X200X16, 640u, 200u},
        {0x10u, X86_VIDEO_KIND_EGA_640X350X16, 640u, 350u}
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX),
        .values = {[CORE_MACHINE_DEBUG_ESP] = 0x7000u,
            [CORE_MACHINE_DEBUG_EIP] = 0x500u, [CORE_MACHINE_DEBUG_EFLAGS] = 2u,
            [CORE_MACHINE_DEBUG_EBX] = 0x1357u, [CORE_MACHINE_DEBUG_ECX] = 0x2468u,
            [CORE_MACHINE_DEBUG_EDX] = 0x369cu}
    };
    core_machine *core = machine->core_machine;

    for (lib_size index = 0u; index < sizeof(modes) / sizeof(modes[0]); ++index) {
        const lib_u8 program[] = {0xb8u, modes[index].mode, 0u, 0xcdu, 0x10u, 0xf4u};
        const lib_u8 dirty_registers[] = {8u, 9u, 12u, 13u, 0x17u, 0x18u};
        const lib_u32 expected_registers[] = {
            modes[index].mode, 0x2468u, 0x369cu, 0x1357u, 0x7000u};
        x86_video_snapshot snapshot;
        lib_bool returned = LIB_FALSE;

        for (lib_size dirty = 0u; dirty < sizeof(dirty_registers); ++dirty)
            if (core_machine_bus_write(core, 0x3d4u, dirty_registers[dirty]) != LIB_STATUS_OK ||
                core_machine_bus_write(core, 0x3d5u, 3u) != LIB_STATUS_OK) return LIB_FALSE;
        for (lib_u8 dirty = 0u; dirty < 9u; ++dirty)
            if (core_machine_bus_write(core, 0x3ceu, dirty) != LIB_STATUS_OK ||
                core_machine_bus_write(core, 0x3cfu, 0x0fu) != LIB_STATUS_OK) return LIB_FALSE;
        if (core_machine_bus_read(core, 0x3dau, &(lib_u32){0u}) != LIB_STATUS_OK ||
            core_machine_bus_write(core, 0x3c0u, 0x33u) != LIB_STATUS_OK ||
            core_machine_bus_write(core, 0x3c0u, 3u) != LIB_STATUS_OK ||
            core_machine_memory_write(core, 0x500u, program, sizeof(program)) != LIB_STATUS_OK ||
            core_machine_debug_patch_registers(core, &entry) != LIB_STATUS_OK) return LIB_FALSE;
        for (lib_u32 instruction = 0u; instruction < 1000u; ++instruction) {
            core_machine_run_result run = {0};

            if (core_machine_run(core, (core_machine_run_budget){1u, 0u}, &run) != LIB_STATUS_OK ||
                run.reason == CORE_MACHINE_STOP_FAULT) return LIB_FALSE;
            if (run.linear_pc == 0x505u) {
                returned = LIB_TRUE;
                break;
            }
        }
        if (!returned) return LIB_FALSE;
        for (lib_size reg = 0u; reg < sizeof(expected_registers) / sizeof(expected_registers[0]); ++reg) {
            lib_u32 value;

            if (core_machine_debug_read_register(core, (core_machine_debug_register)reg, &value) !=
                    LIB_STATUS_OK || value != expected_registers[reg]) return LIB_FALSE;
        }
        if (core_machine_memory_write(core, 0xa0000u, &(lib_u8){0x80u}, 1u) !=
                LIB_STATUS_OK || core_machine_memory_write(core,
                0xa0000u + modes[index].width / 8u, &(lib_u8){0x40u}, 1u) != LIB_STATUS_OK ||
            core_machine_capture_display_snapshot(machine->board, &snapshot) != LIB_STATUS_OK ||
            snapshot.kind != modes[index].kind || snapshot.pixel_width != modes[index].width ||
            snapshot.pixel_height != modes[index].height || snapshot.pixels[0] != 15u ||
            snapshot.pixels[modes[index].width + 1u] != 15u ||
            snapshot.palette_rgb[6] != 0xaa5500u ||
            snapshot.palette_rgb[8] != 0x555555u ||
            snapshot.palette_rgb[15] != 0xffffffu) return LIB_FALSE;
    }
    return LIB_TRUE;
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
    passed = default_firmware_initializes_text_cursor(machine) &&
        default_firmware_initializes_ega_graphics(machine);
    vm_machine_destroy(machine);
    return passed ? 0 : 1;
}
