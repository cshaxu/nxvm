#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session.h"
#include "vm/platform/input.h"
#include "vm/platform/vm_request_transport.h"

#define VM_KBC_AUX_IMAGE_BYTES (1440u * 1024u)
#define VM_KBC_AUX_BOOT_BUDGET 500000u
#define VM_KBC_AUX_COUNT_ADDRESS 0x0500u
#define VM_KBC_AUX_BYTES_ADDRESS 0x0510u

static uint8_t vm_kbc_aux_image[VM_KBC_AUX_IMAGE_BYTES];

static C_INT vm_kbc_aux_write_fixture(C_CHAR path[MAX_PATH])
{
    static const uint8_t boot_code[] = {
        0xfau,                         /* cli */
        0x31u, 0xc0u,                 /* xor ax, ax */
        0x8eu, 0xd8u,                 /* mov ds, ax */
        0xc7u, 0x06u, 0xd0u, 0x01u, 0x80u, 0x7cu,
                                      /* mov word [74h*4], 7c80h */
        0xc7u, 0x06u, 0xd2u, 0x01u, 0x00u, 0x00u,
                                      /* mov word [74h*4+2], 0 */
        0xb0u, 0xfbu,                 /* unmask master IRQ2 */
        0xe6u, 0x21u,
        0xb0u, 0xefu,                 /* unmask slave IRQ4 / IRQ12 */
        0xe6u, 0xa1u,
        0xfbu,                         /* sti */
        0xb0u, 0xd4u,                 /* KBC: next byte is AUX command */
        0xe6u, 0x64u,
        0xb0u, 0xf4u,                 /* AUX: enable data reporting */
        0xe6u, 0x60u,
        0xf4u,                         /* hlt */
        0xebu, 0xfdu                  /* jmp to hlt */
    };
    static const uint8_t irq12_handler[] = {
        0x50u,                         /* push ax */
        0x53u,                         /* push bx */
        0xe4u, 0x60u,                 /* in al, 60h */
        0x8bu, 0x1eu, 0x00u, 0x05u,   /* mov bx, [0500h] */
        0x88u, 0x87u, 0x10u, 0x05u,   /* mov [bx+0510h], al */
        0x43u,                         /* inc bx */
        0x89u, 0x1eu, 0x00u, 0x05u,   /* mov [0500h], bx */
        0xb0u, 0x20u,                 /* non-specific EOI */
        0xe6u, 0xa0u,                 /* slave */
        0xe6u, 0x20u,                 /* master */
        0x5bu,                         /* pop bx */
        0x58u,                         /* pop ax */
        0xcfu                          /* iret */
    };
    STD_FILE *file;
    DWORD length;

    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) return 0;
    STD_MEMSET(vm_kbc_aux_image, 0, sizeof(vm_kbc_aux_image));
    STD_MEMCPY(vm_kbc_aux_image, boot_code, sizeof(boot_code));
    STD_MEMCPY(vm_kbc_aux_image + 0x80u, irq12_handler,
        sizeof(irq12_handler));
    vm_kbc_aux_image[510u] = 0x55u;
    vm_kbc_aux_image[511u] = 0xaau;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return 0;
    if (STD_FWRITE(vm_kbc_aux_image, 1u, sizeof(vm_kbc_aux_image), file) !=
        sizeof(vm_kbc_aux_image)) {
        STD_FCLOSE(file);
        DeleteFileA(path);
        return 0;
    }
    STD_FCLOSE(file);
    return 1;
}

static C_INT vm_kbc_aux_read_count(vm_session *session, uint16_t *out_count)
{
    return core_machine_memory_read(session->core_machine,
        VM_KBC_AUX_COUNT_ADDRESS, out_count, sizeof(*out_count)) == TYPE_STATUS_OK;
}

static C_INT vm_kbc_aux_run_until_count(vm_session *session, uint16_t expected)
{
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    uint32_t instruction;
    uint16_t count = 0u;

    for (instruction = 0u; instruction < VM_KBC_AUX_BOOT_BUDGET;
         ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) !=
                TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) {
            return 0;
        }
        if (vm_kbc_aux_read_count(session, &count) && count >= expected) return 1;
    }
    return 0;
}

C_INT main(C_VOID)
{
    const vm_session_config config = {
        .fdd_image = "",
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    vm_session *session = STD_NULL;
    vm_session_config fixture_config = config;
    C_CHAR path[MAX_PATH] = {0};
    uint8_t bytes[4] = {0};
    uint16_t count = 0u;
    C_INT passed = 0;

    if (!vm_kbc_aux_write_fixture(path)) goto done;
    fixture_config.fdd_image = path;
    if (vm_session_create(&fixture_config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) goto done;

    /* The guest's D4/F4 transaction must reach the AUX device before host input. */
    if (!vm_kbc_aux_run_until_count(session, 1u) ||
        !vm_kbc_aux_read_count(session, &count) || count != 1u ||
        core_machine_memory_read(session->core_machine, VM_KBC_AUX_BYTES_ADDRESS,
            bytes, 1u) != TYPE_STATUS_OK || bytes[0] != 0xfau) goto done;

    vm_platform_mouse_receive_relative_event_for(&session->mouse_transport,
        5, 3, 0x01u);
    if (!vm_kbc_aux_read_count(session, &count) || count != 1u) goto done;
    vm_platform_request_transport_observe_execution_boundary(
        &session->request_transport);
    if (!vm_kbc_aux_run_until_count(session, 4u) ||
        core_machine_memory_read(session->core_machine, VM_KBC_AUX_BYTES_ADDRESS,
            bytes, sizeof(bytes)) != TYPE_STATUS_OK ||
        bytes[0] != 0xfau || bytes[1] != 0x29u || bytes[2] != 0x05u ||
        bytes[3] != 0xfdu) goto done;

    passed = 1;
done:
    vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    if (!passed) return 1;
    STD_PRINTF("M5:T229:S3:AUX:GUEST:OK\n");
    return 0;
}
