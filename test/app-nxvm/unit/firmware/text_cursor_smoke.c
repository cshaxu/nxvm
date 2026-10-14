#include "../../support/profile.h"
#include "lib/types/file.h"

#include "core/board-base/machine_board_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine_private.h"
#include "core/machine/frame_interface.h"
#include "core/x86/debug_interface.h"

extern const vm_machine_assets vm_app_firmware;

static lib_bool default_firmware_text_geometry(const x86_video_snapshot *snapshot)
{
    emulator_machine_frame *frame = lib_allocate(sizeof(*frame));
    lib_bool passed;

    if (frame == LIB_NULL) return LIB_FALSE;
    passed = snapshot->kind == X86_VIDEO_KIND_TEXT && snapshot->columns == 80u &&
        snapshot->rows == 25u && snapshot->text_cell_height == 16u &&
        snapshot->text_glyph_height == 16u && snapshot->cursor_top == 14u &&
        snapshot->cursor_bottom == 15u && snapshot->text_glyphs_present &&
        vm_machine_frame_from_display(snapshot, 1u, frame) == LIB_STATUS_OK &&
        !frame->window.graphics && frame->window.text.base.font_height == 16u &&
        frame->window.text.base.text_columns * 8u == 640u &&
        frame->window.text.base.text_rows * frame->window.text.base.font_height == 400u;
    for (lib_size row = 0u; passed && row < X86_VIDEO_TEXT_GLYPH_ROWS; ++row) {
        const lib_u8 expected = row < 8u ? 0x55u : row < 16u ? 0x33u : 0u;

        passed = snapshot->text_glyphs[(lib_size)'A' * X86_VIDEO_TEXT_GLYPH_ROWS + row] ==
            expected && frame->window.text.font[(lib_size)'A' * KVM_WINDOW_FONT_HEIGHT + row] ==
            expected;
    }
    lib_release(frame);
    return passed;
}

static lib_bool default_firmware_text_bda(core_machine *core)
{
    lib_u8 bda[3];

    return core_machine_memory_read(core, 0x484u, bda, sizeof(bda)) == LIB_STATUS_OK &&
        bda[0] == 24u && bda[1] == 16u && bda[2] == 0u;
}

static lib_bool default_firmware_initializes_text_cursor(vm_machine *machine)
{
    for (lib_u32 instruction = 0u; instruction < 100000u; ++instruction) {
        core_machine_run_result run = {0};
        x86_video_snapshot snapshot;
        lib_status captured;

        if (core_machine_run(machine->core_machine,
                (core_machine_run_budget){1u, 0u}, &run) !=
                LIB_STATUS_OK || run.reason == CORE_MACHINE_STOP_FAULT) {
            return LIB_FALSE;
        }
        captured = core_machine_capture_display_snapshot(machine->board, &snapshot);
        /* A mode setter programs multiple registers before its frame is ready. */
        if (captured == LIB_STATUS_UNSUPPORTED) continue;
        if (captured != LIB_STATUS_OK) return LIB_FALSE;
        if (snapshot.kind == X86_VIDEO_KIND_TEXT &&
            snapshot.text_cell_height == 16u && snapshot.cursor_visible &&
            snapshot.cursor_top == 14u && snapshot.cursor_bottom == 15u) {
            return default_firmware_text_geometry(&snapshot) &&
                default_firmware_text_bda(machine->core_machine);
        }
    }
    return LIB_FALSE;
}

static lib_bool default_firmware_raster_runs(core_machine *core, lib_u8 mode)
{
    const lib_u8 program[] = {0xebu, 0xfeu};
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {[CORE_MACHINE_DEBUG_EIP] = 0x500u}
    };
    lib_bool active = LIB_FALSE;
    lib_bool retrace = LIB_FALSE;
    lib_bool outside_retrace = LIB_FALSE;

    if (core_machine_memory_write(core, 0x500u, program, sizeof(program)) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(core, &entry) != LIB_STATUS_OK) return LIB_FALSE;
    for (lib_u16 segment = 0u; segment < 128u && (!active || !retrace || !outside_retrace); ++segment) {
        core_machine_run_result run = {0};
        lib_u32 status;

        if (core_machine_run(core, (core_machine_run_budget){64u, 0u}, &run) != LIB_STATUS_OK ||
            run.reason == CORE_MACHINE_STOP_FAULT ||
            core_machine_bus_read(core, 0x3dau, &status) != LIB_STATUS_OK) return LIB_FALSE;
        active |= (status & 1u) != 0u;
        retrace |= (status & 8u) != 0u;
        outside_retrace |= retrace && (status & 8u) == 0u;
        if ((status & 9u) == 9u) return LIB_FALSE;
    }
    if (!active || !retrace || !outside_retrace)
        lib_c_fprintf(lib_c_stderr, "FIRMWARE:RASTER:mode=%u active=%u retrace=%u outside=%u\n",
            mode, active, retrace, outside_retrace);
    return active && retrace && outside_retrace;
}

static lib_bool default_firmware_initializes_video_modes(vm_machine *machine)
{
    static const struct {
        lib_u8 mode;
        x86_video_kind kind;
        lib_u16 width;
        lib_u16 height;
    } modes[] = {
        {0x0du, X86_VIDEO_KIND_EGA_320X200X16, 320u, 200u},
        {0x0eu, X86_VIDEO_KIND_EGA_640X200X16, 640u, 200u},
        {0x10u, X86_VIDEO_KIND_EGA_640X350X16, 640u, 350u},
        {0x03u, X86_VIDEO_KIND_TEXT, 0u, 0u}
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

        if (modes[index].kind == X86_VIDEO_KIND_TEXT &&
            core_machine_memory_write(core, 0xa0000u + (lib_u32)'A' * X86_VIDEO_TEXT_GLYPH_ROWS,
                &(lib_u8){0u}, 1u) != LIB_STATUS_OK) return LIB_FALSE;
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
        /* REP copies all 4096 font words through the actual CPU path. */
        for (lib_u32 instruction = 0u; instruction < 10000u; ++instruction) {
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
        if (modes[index].kind == X86_VIDEO_KIND_TEXT) {
            const lib_u8 first[] = {'A', 0x1fu, 'B', 0x2eu};
            const lib_u8 second[] = {'C', 0x3du};

            if (core_machine_memory_write(core, 0xb8000u, first, sizeof(first)) != LIB_STATUS_OK ||
                core_machine_memory_write(core, 0xb8000u + 160u, second, sizeof(second)) !=
                    LIB_STATUS_OK ||
                core_machine_memory_write(core, 0xb8000u + 24u * 160u, first, sizeof(first)) !=
                    LIB_STATUS_OK || !default_firmware_text_bda(core) ||
                core_machine_capture_display_snapshot(machine->board, &snapshot) != LIB_STATUS_OK ||
                snapshot.kind != X86_VIDEO_KIND_TEXT || snapshot.columns != 80u ||
                snapshot.rows != 25u || !default_firmware_text_geometry(&snapshot) ||
                snapshot.characters[0] != 'A' ||
                snapshot.characters[1] != 'B' || snapshot.characters[80] != 'C' ||
                snapshot.characters[24u * 80u] != 'A' ||
                snapshot.attributes[0] != 0x1fu || snapshot.attributes[1] != 0x2eu ||
                snapshot.attributes[80] != 0x3du || !snapshot.text_glyphs_present ||
                snapshot.text_glyphs[(lib_size)'A' * X86_VIDEO_TEXT_GLYPH_ROWS] != 0x55u ||
                snapshot.text_glyphs[(lib_size)'A' * X86_VIDEO_TEXT_GLYPH_ROWS + 8u] != 0x33u) {
                lib_c_fprintf(lib_c_stderr,
                    "Text restore: kind %u, %ux%u, cells %02x/%02x/%02x, attributes %02x/%02x/%02x, glyph %02x/%02x\n",
                    (lib_u32)snapshot.kind, snapshot.columns, snapshot.rows,
                    snapshot.characters[0], snapshot.characters[1], snapshot.characters[80],
                    snapshot.attributes[0], snapshot.attributes[1], snapshot.attributes[80],
                    snapshot.text_glyphs[(lib_size)'A' * X86_VIDEO_TEXT_GLYPH_ROWS],
                    snapshot.text_glyphs[(lib_size)'A' * X86_VIDEO_TEXT_GLYPH_ROWS + 8u]);
                return LIB_FALSE;
            }
            if (!default_firmware_raster_runs(core, modes[index].mode)) return LIB_FALSE;
            continue;
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
        if (!default_firmware_raster_runs(core, modes[index].mode)) return LIB_FALSE;
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
        default_firmware_initializes_video_modes(machine);
    vm_machine_destroy(machine);
    return passed ? 0 : 1;
}
