#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/entry_plan_interface.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "support/rom/session_assets.h"

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
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const core_machine_entry_plan_preload preload = {
        0x0200u, vm_cga_graphics_program, sizeof(vm_cga_graphics_program) };
    const core_machine_entry_plan plan = {
        .state = {.ip = 0x0200u, .sp = 0x7000u, .eflags = 0u},
        .entry_physical = 0x0200u,
        .entry_route = CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM,
        .preloads = &preload,
        .preload_count = 1u};
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_display_snapshot snapshot;
    vm_machine *session = LIB_NULL;
    lib_u32 instruction;
    lib_i32 passed = 0;

    if (vm_test_default_pc_at_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || core_machine_apply_entry_plan(session->core_machine,
            &plan) != LIB_STATUS_OK) goto done;
    for (instruction = 0u; instruction < VM_CGA_GRAPHICS_BOOT_BUDGET;
         ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) !=
                LIB_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) {
            goto done;
        }
        if (core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) != LIB_STATUS_OK ||
            snapshot.kind != CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4) {
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
    printf("M5:T228:S3:CGA:SYSTEM:OK\n");
    return 0;
}
