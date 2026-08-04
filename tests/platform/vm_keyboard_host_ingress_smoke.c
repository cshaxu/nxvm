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

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    uint16_t head;
    uint16_t tail;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        !vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD, &head) ||
        !vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL, &tail) || head != tail) goto fail;
    vm_platform_win32_keyboard_make_key_for(&session->platform_run_context,
        0x1eu, 'A');
    if (!vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL, &tail) || tail != head ||
        vm_platform_request_transport_execution_boundary_count(
            &session->request_transport) != 0u) goto fail;
    vm_platform_request_transport_observe_execution_boundary(
        &session->request_transport);
    if (!vm_keyboard_host_ingress_read_word(session,
            QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL, &tail) || tail != head ||
        vm_platform_request_transport_execution_boundary_count(
            &session->request_transport) != 1u) goto fail;
    vm_session_destroy(session);
    STD_PRINTF("M5:T192:S3:KBC-HOST-INGRESS:OK\n");
    return 0;

fail:
    vm_session_destroy(session);
    return 1;
}
