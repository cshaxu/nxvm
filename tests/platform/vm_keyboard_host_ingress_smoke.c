#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"
#include "vm/composition/session/session.h"
#include "vm/platform/win32/win32.h"
#include "vm/profile/default_profile/firmware/qdkeyb.h"

static C_INT vm_keyboard_host_ingress_read_word(const vm_session *session,
    uint16_t offset, uint16_t *out_value)
{
    core_machine_memory_read_real_from(
        core_machine_executor_memory_borrow(session->core_machine), 0u, offset,
        out_value, sizeof(*out_value));
    return 1;
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    uint16_t head;
    uint16_t tail;
    uint16_t key;

    if (vm_session_create(STD_NULL, &session) != NTVDM64_STATUS_OK ||
        !vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD, &head) ||
        !vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL, &tail) || head != tail) {
        vm_session_destroy(session);
        return 1;
    }
    vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
        0x1eu, 'A');
    if (!vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL, &tail) || tail != 0x0420u ||
        !vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START, &key) || key != 0x1e61u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    STD_PRINTF("M5:T151:S1:KEYBOARD-HOST-INGRESS:OK\n");
    return 0;
}
