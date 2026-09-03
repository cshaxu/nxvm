#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/machine/fdd.h"
#include "../support/rom/session_assets.h"

#define VM_CGA254_IMAGE_BYTES (1440u * 1024u)
#define VM_CGA254_BOOT_BUDGET 500000u

static type_unsigned_8 vm_cga254_image[VM_CGA254_IMAGE_BYTES];

static C_VOID vm_cga254_boot_fixture(C_VOID)
{
    static const type_unsigned_8 boot_code[] = {
        0x31u, 0xc0u, 0x8eu, 0xd8u,
        0xb8u, 0x06u, 0x00u, 0xcdu, 0x10u,
        0xb8u, 0x00u, 0xb8u, 0x8eu, 0xc0u, 0x31u, 0xffu,
        0xb0u, 0xa0u, 0xaau, 0xbfu, 0x00u, 0x20u, 0xb0u, 0x40u, 0xaau,
        0xb8u, 0x03u, 0x00u, 0xcdu, 0x10u,
        0xebu, 0xfeu
    };
    STD_MEMSET(vm_cga254_image, 0, sizeof(vm_cga254_image));
    STD_MEMCPY(vm_cga254_image, boot_code, sizeof(boot_code));
    vm_cga254_image[510u] = 0x55u;
    vm_cga254_image[511u] = 0xaau;
}

C_INT main(C_VOID)
{
    const vm_session_config config = {
        .floppy_image = { "" },
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_display_snapshot snapshot;
    vm_session *session = STD_NULL;
    type_unsigned_8 mode = 0u;
    type_unsigned_32 instruction;
    C_INT saw_cga = 0;
    C_INT saw_text = 0;

    vm_cga254_boot_fixture();
    {
        vm_session_config fixture_config = config;

        if (vm_test_default_pc_at_session_create(&fixture_config, &session) !=
                TYPE_STATUS_OK || session == STD_NULL ||
            vm_machine_fdd_replace_bytes(&session->fdd, vm_cga254_image,
                sizeof(vm_cga254_image)) != 0) goto done;
    }
    for (instruction = 0u; instruction < VM_CGA254_BOOT_BUDGET; ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT ||
            core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) != TYPE_STATUS_OK) goto done;
        if (snapshot.kind == CORE_MACHINE_DISPLAY_KIND_CGA_640X200X2 &&
            snapshot.pixels[0] == 1u && snapshot.pixels[1] == 0u &&
            snapshot.pixels[2] == 1u && snapshot.pixels[640u] == 0u &&
            snapshot.pixels[641u] == 1u && snapshot.palette_rgb[0] == 0u &&
            snapshot.palette_rgb[1] == 0xffffffu &&
            core_machine_memory_read(session->core_machine, 0x0449u, &mode,
                sizeof(mode)) == TYPE_STATUS_OK && mode == 0x06u) {
            saw_cga = 1;
        }
        if (saw_cga && snapshot.kind == CORE_MACHINE_DISPLAY_KIND_TEXT &&
            core_machine_memory_read(session->core_machine, 0x0449u, &mode,
                sizeof(mode)) == TYPE_STATUS_OK && mode == 0x03u) {
            saw_text = 1;
            break;
        }
    }

done:
    vm_session_destroy(session);
    if (!saw_cga || !saw_text) return 1;
    STD_PRINTF("M5:T254:S3:CGA-640:SYSTEM:OK\n");
    return 0;
}
