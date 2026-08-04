#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/debug_interface.h"
#include "vm/composition/session/session.h"
#include "vm/platform/vm_request_transport.h"
#include "vm/platform/win32/win32.h"
#include "vm/profile/default_profile/firmware/qdkeyb.h"

static C_INT vm_keyboard_host_ingress_read_word(const vm_session *session,
    uint16_t offset, uint16_t *out_value)
{
    return core_machine_debug_read_memory(session->core_machine, offset,
        out_value, sizeof(*out_value)) == TYPE_STATUS_OK;
}

static C_INT vm_keyboard_host_ingress_read_byte(const vm_session *session,
    uint16_t offset, uint8_t *out_value)
{
    return core_machine_debug_read_memory(session->core_machine, offset,
        out_value, sizeof(*out_value)) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    uint16_t head;
    uint16_t tail;
    uint8_t flag0;
    uint8_t flag1;
    uint8_t initial_flag0;
    uint8_t initial_flag1;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        !vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD, &head) ||
        !vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL, &tail) ||
        !vm_keyboard_host_ingress_read_byte(session,
            QDKEYB_VBIOS_ADDR_KEYB_FLAG0, &initial_flag0) ||
        !vm_keyboard_host_ingress_read_byte(session,
            QDKEYB_VBIOS_ADDR_KEYB_FLAG1, &initial_flag1) || head != tail) goto fail;
    if (vm_platform_run_context_submit_keyboard_state(
            &session->platform_run_context,
            CORE_MACHINE_KEYBOARD_ASYNC_LEFT_SHIFT |
                CORE_MACHINE_KEYBOARD_ASYNC_CONTROL,
            CORE_MACHINE_KEYBOARD_TOGGLE_CAPS_LOCK) != TYPE_STATUS_OK ||
        !vm_keyboard_host_ingress_read_byte(session,
            QDKEYB_VBIOS_ADDR_KEYB_FLAG0, &flag0) ||
        !vm_keyboard_host_ingress_read_byte(session,
            QDKEYB_VBIOS_ADDR_KEYB_FLAG1, &flag1) ||
        flag0 != initial_flag0 || flag1 != initial_flag1 ||
        vm_platform_request_transport_execution_boundary_count(
            &session->request_transport) != 0u) goto fail;
    vm_platform_request_transport_observe_execution_boundary(
        &session->request_transport);
    if (!vm_keyboard_host_ingress_read_byte(session,
            QDKEYB_VBIOS_ADDR_KEYB_FLAG0, &flag0) ||
        !vm_keyboard_host_ingress_read_byte(session,
            QDKEYB_VBIOS_ADDR_KEYB_FLAG1, &flag1) ||
        flag0 != (QDKEYB_FLAG0_D_LSHIFT | QDKEYB_FLAG0_D_CTRL |
            QDKEYB_FLAG0_A_CAPLCK) || flag1 != 0u ||
        vm_platform_request_transport_execution_boundary_count(
            &session->request_transport) != 1u) goto fail;
    vm_platform_win32_keyboard_make_key_for(&session->platform_run_context,
        0x1eu, 'A');
    if (!vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL, &tail) || tail != head ||
        vm_platform_request_transport_execution_boundary_count(
            &session->request_transport) != 1u) goto fail;
    vm_platform_request_transport_observe_execution_boundary(
        &session->request_transport);
    if (!vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL, &tail) || tail != head ||
        vm_platform_request_transport_execution_boundary_count(
            &session->request_transport) != 2u) goto fail;
    vm_session_destroy(session);
    STD_PRINTF("M5:T200:S1:KEYBOARD-STATE-INGRESS:OK\n");
    return 0;

fail:
    vm_session_destroy(session);
    return 1;
}
