#include "type.h"

#include <windows.h>

#include "core/machine/dma.h"
#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_FDC_BOUNDARY_IMAGE_BYTES (1440u * 1024u)
#define VM_FDC_BOUNDARY_RESULT_ADDRESS 0x0500u
#define VM_FDC_BOUNDARY_RAM_SIZE_ADDRESS 0x0413u
#define VM_FDC_BOUNDARY_RUN_LIMIT 400000u

static type_unsigned_8 vm_fdc_boundary_image[VM_FDC_BOUNDARY_IMAGE_BYTES];

static C_INT vm_fdc_boundary_write_fixture(C_CHAR path[MAX_PATH])
{
    static const type_unsigned_8 boot_code[] = {
        0xfau,                         /* cli */
        0x31u, 0xc0u,                 /* xor ax, ax */
        0x8eu, 0xd8u,                 /* mov ds, ax */
        0x8eu, 0xc0u,                 /* mov es, ax */
        0x8eu, 0xd0u,                 /* mov ss, ax */
        0xbcu, 0x00u, 0x7cu,         /* mov sp, 7c00h */
        0xb4u, 0x02u,                 /* mov ah, 2 */
        0xb0u, 0x02u,                 /* mov al, 2 */
        0xb5u, 0x00u,                 /* mov ch, 0 */
        0xb1u, 0x02u,                 /* mov cl, 2 */
        0xb6u, 0x00u,                 /* mov dh, 0 */
        0xb2u, 0x00u,                 /* mov dl, 0 */
        0xbbu, 0xf0u, 0xffu,         /* mov bx, fff0h */
        0xcdu, 0x40u,                 /* int 40h */
        0x72u, 0x29u,                 /* jc failure */
        0x80u, 0x3eu, 0xf0u, 0xffu, 0x10u,
                                      /* cmp byte [fff0h], 10h */
        0x75u, 0x22u,                 /* jne failure */
        0xb8u, 0x00u, 0x10u,         /* mov ax, 1000h */
        0x8eu, 0xc0u,                 /* mov es, ax */
        0x26u, 0x80u, 0x3eu, 0x00u, 0x00u, 0x20u,
                                      /* cmp byte es:[0], 20h */
        0x75u, 0x15u,                 /* jne failure */
        0xb8u, 0x1fu, 0x10u,         /* mov ax, 101fh */
        0x8eu, 0xc0u,                 /* mov es, ax */
        0x26u, 0x80u, 0x3eu, 0x00u, 0x00u, 0x80u,
                                      /* cmp byte es:[0], 80h */
        0x75u, 0x08u,                 /* jne failure */
        0xc6u, 0x06u, 0x00u, 0x05u, 0xa5u,
                                      /* mov byte [0500h], a5h */
        0xf4u,                         /* hlt */
        0xebu, 0xfdu,                 /* jmp hlt */
        0xc6u, 0x06u, 0x00u, 0x05u, 0x5au,
                                      /* failure: mov byte [0500h], 5ah */
        0xf4u,                         /* hlt */
        0xebu, 0xfdu                  /* jmp hlt */
    };
    STD_FILE *file;
    DWORD length;
    type_unsigned_16 signature = 0xaa55u;
    type_native_unsigned index;

    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) return 0;
    STD_MEMSET(vm_fdc_boundary_image, 0u, sizeof(vm_fdc_boundary_image));
    STD_MEMCPY(vm_fdc_boundary_image, boot_code, sizeof(boot_code));
    STD_MEMCPY(vm_fdc_boundary_image + 510u, &signature, sizeof(signature));
    for (index = 0u; index < 512u; ++index) {
        vm_fdc_boundary_image[512u + index] = (type_unsigned_8)(0x10u + index);
        vm_fdc_boundary_image[1024u + index] = (type_unsigned_8)(0x80u + index);
    }
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return 0;
    if (STD_FWRITE(vm_fdc_boundary_image, 1u, sizeof(vm_fdc_boundary_image), file) !=
        sizeof(vm_fdc_boundary_image)) {
        STD_FCLOSE(file);
        DeleteFileA(path);
        return 0;
    }
    STD_FCLOSE(file);
    return 1;
}

C_INT main(C_VOID)
{
    const vm_session_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const vm_session_config model_339_config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339
    };
    const core_machine_run_budget budget = { 64u, 0u };
    vm_session_config fixture_config = config;
    core_machine_run_result result;
    vm_session *session = STD_NULL;
    C_CHAR path[MAX_PATH] = { 0 };
    type_unsigned_8 result_byte = 0u;
    type_unsigned_16 conventional_kb = 0u;
    type_unsigned_32 instruction;
    C_INT stage = 0;
    C_INT passed = 0;

    if (!vm_fdc_boundary_write_fixture(path)) goto done;
    stage = 1;
    fixture_config.fdd_image = path;
    if (vm_session_create(&fixture_config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) goto done;
    for (instruction = 0u; instruction < VM_FDC_BOUNDARY_RUN_LIMIT;
        instruction += budget.instructions) {
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT || core_machine_memory_read(
                session->core_machine, VM_FDC_BOUNDARY_RESULT_ADDRESS, &result_byte,
                sizeof(result_byte)) != TYPE_STATUS_OK) {
            goto done;
        }
        if (result_byte != 0u) break;
    }
    passed = result_byte == 0xa5u && core_machine_memory_read(session->core_machine,
        VM_FDC_BOUNDARY_RAM_SIZE_ADDRESS, &conventional_kb,
        sizeof(conventional_kb)) == TYPE_STATUS_OK && conventional_kb == 0x027fu &&
        session->core_machine->shared_dma_primary.data.page[2u] == 0x09u;
    stage = 2;
    vm_session_destroy(session);
    session = STD_NULL;
    if (!passed) goto done;
    fixture_config = model_339_config;
    fixture_config.fdd_image = path;
    result_byte = 0u;
    conventional_kb = 0u;
    if (vm_session_create(&fixture_config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) goto done;
    stage = 3;
    for (instruction = 0u; instruction < VM_FDC_BOUNDARY_RUN_LIMIT;
        instruction += budget.instructions) {
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT || core_machine_memory_read(
                session->core_machine, VM_FDC_BOUNDARY_RESULT_ADDRESS, &result_byte,
                sizeof(result_byte)) != TYPE_STATUS_OK) {
            goto done;
        }
        if (result_byte != 0u) break;
    }
    passed = result_byte == 0xa5u && core_machine_memory_read(session->core_machine,
        VM_FDC_BOUNDARY_RAM_SIZE_ADDRESS, &conventional_kb,
        sizeof(conventional_kb)) == TYPE_STATUS_OK && conventional_kb == 0x0200u &&
        session->core_machine->shared_dma_primary.data.page[2u] == 0x07u;
done:
    vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    if (!passed) {
        STD_PRINTF("M5:T380:S2:FDC-DMA-BOUNDARY:FAIL:%d:%02x:%04x\n", stage,
            result_byte, conventional_kb);
        return 1;
    }
    STD_PRINTF("M5:T348:S4:FDC-DMA-BOUNDARY:OK\n");
    STD_PRINTF("M5:T380:S2:MODEL339-512K-FDC-START:OK\n");
    return 0;
}
