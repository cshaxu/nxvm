#include "type.h"

#include "vm/composition/session/session_interface.h"
#include "tests/support/vm_session_fixture.h"
#include "vm/platform/vm_request_transport.h"
#include "vm/platform/win32/win32.h"

#define VM_HOST_CANCELLATION_F9_SCAN_CODE 0x43u
#define VM_HOST_CANCELLATION_F9_VIRTUAL_KEY 0x78u

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    vm_platform_request request;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) goto fail;
    vm_platform_win32_keyboard_make_key_for(vm_session_fixture_platform_run_context(session),
        vm_session_fixture_platform_run_handle(session), VM_HOST_CANCELLATION_F9_SCAN_CODE,
        VM_HOST_CANCELLATION_F9_VIRTUAL_KEY, 1);
    if (!vm_platform_run_handle_take_stop_report(vm_session_fixture_platform_run_handle(session)) ||
        vm_platform_run_handle_take_stop_report(vm_session_fixture_platform_run_handle(session)) ||
        vm_platform_request_transport_dequeue_ingress(vm_session_fixture_request_transport(session),
            &request) != TYPE_STATUS_OK ||
        request.kind != VM_PLATFORM_REQUEST_KEY_EVENT ||
        request.data.key_event.scan_code != VM_HOST_CANCELLATION_F9_SCAN_CODE ||
        request.data.key_event.virtual_key !=
            VM_HOST_CANCELLATION_F9_VIRTUAL_KEY) goto fail;
    vm_session_destroy(session);
    STD_PRINTF("M5:T201:S3:HOST-CANCELLATION:OK\n");
    return 0;

fail:
    vm_session_destroy(session);
    return 1;
}
