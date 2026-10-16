#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86-core/machine/entry_plan_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine_private.h"
#include "../support/rom/session_assets.h"

#define VM_CGA_GRAPHICS_BOOT_BUDGET 500000u

static const lib_u8 vm_cga_graphics_program[] = {
        0xb8u, 0x00u, 0xb8u,       /* mov ax, b800h */
        0x8eu, 0xc0u,              /* mov es, ax */
        0xbau, 0xd8u, 0x03u,       /* mov dx, 3d8h */
        0xb0u, 0x0au,              /* mov al, 0ah */
        0xeeu,                     /* out dx, al */
        0xbau, 0xd9u, 0x03u,       /* mov dx, 3d9h */
        0xb0u, 0x20u,              /* mov al, palette 1 */
        0xeeu,                     /* out dx, al */
        0x31u, 0xffu,              /* xor di, di */
        0xb0u, 0x1bu,              /* mov al, 00,01,10,11 */
        0xaau,                     /* stosb (even row) */
        0xbfu, 0x00u, 0x20u,       /* mov di, 2000h */
        0xb0u, 0xe4u,              /* mov al, 11,10,01,00 */
        0xaau,                     /* stosb (odd row) */
        0xebu, 0xfeu               /* jmp $ */
};

lib_i32 main(void)
{
    const vm_machine_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_8086,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .bios_count = 1u
    };
    const x86_core_machine_entry_plan_preload preload = {
        0x0200u, vm_cga_graphics_program, sizeof(vm_cga_graphics_program) };
    const x86_core_machine_entry_plan plan = {
        .state = {.ip = 0x0200u, .sp = 0x7000u, .eflags = 0u},
        .entry_physical = 0x0200u,
        .entry_route = X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM,
        .preloads = &preload,
        .preload_count = 1u};
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result result;
    x86_video_snapshot snapshot;
    vm_machine *session = LIB_NULL;
    vm_machine_construction construction;
    vm_machine_assets assets;
    lib_u8 rom[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES];
    lib_u32 instruction;
    lib_i32 passed = 0;

    vm_test_default_pc_at_assets(&assets, rom);
    if (vm_test_profile_construction_create(VM_MACHINE_PROFILE_DEFAULT_PC_AT,
            &config, &assets, &construction) != LIB_STATUS_OK) goto done;
    /* This instruction receiver needs a CGA board, not the default EGA card. */
    construction.topology.display.cga_vram_present = LIB_TRUE;
    construction.topology.display.ega_present = LIB_FALSE;
    construction.topology.display.vertical_irq = 0u;
    if (vm_machine_create(&config, &construction, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || x86_core_machine_apply_entry_plan(session->x86_core_machine,
            &plan) != LIB_STATUS_OK) goto done;
    for (instruction = 0u; instruction < VM_CGA_GRAPHICS_BOOT_BUDGET;
         ++instruction) {
        if (x86_core_machine_run(session->x86_core_machine, budget, &result) !=
                LIB_STATUS_OK || result.reason == X86_CORE_MACHINE_STOP_FAULT) {
            goto done;
        }
        if (core_machine_capture_display_snapshot(session->board,
                &snapshot) != LIB_STATUS_OK) goto done;
        if (snapshot.kind != X86_VIDEO_KIND_CGA_320X200X4) {
            continue;
        }
        if (snapshot.pixels[0] == 0u && snapshot.pixels[1] == 1u &&
            snapshot.pixels[2] == 2u && snapshot.pixels[3] == 3u &&
            snapshot.pixels[320u] == 3u && snapshot.pixels[321u] == 2u &&
            snapshot.pixels[322u] == 1u && snapshot.pixels[323u] == 0u &&
            snapshot.palette_rgb[1] == 0x00aaaau) {
            passed = 1;
            break;
        }
    }

done:
    vm_machine_destroy(session);
    if (!passed) return 1;
    printf("CGA:SYSTEM:OK\n");
    return 0;
}
