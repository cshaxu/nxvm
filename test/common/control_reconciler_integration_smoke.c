#include "lib/types/test.h"
#include "lib/types/file.h"
#include "common/session/control.h"

/* These stubs keep the test at the application control boundary while making
 * the paused guest-injection barrier observable. */
static lib_u32 delivered_guest_input;
static lib_i32 deliver_input(void *context, const kvm_input_event *event)
{ (void)context; (void)event; ++delivered_guest_input; return 1; }

static void take(common_session_queue *queue, common_session_event *event)
{ lib_test_assert(common_session_queue_take(queue, event, 0u)); }

int main(void)
{
    common_session_queue storage = { 0 }, *queue = &storage;
    common_session_event event;
    lib_console_line start = { 5u, "start" };
    lib_console_line pause = { 5u, "pause" };
    kvm_input_event raw_key = { 0 };
    kvm_input_event raw_mouse = { 0 };
    kvm_input_event retired = { 0 };
    kvm_input_event hotkey = { 0 };

    lib_test_assert(common_session_queue_initialize(queue));
    raw_key.type = KVM_EVENT_KEY;
    raw_key.source_identity = 41u;
    raw_key.data.key.key = 'A';
    raw_key.data.key.pressed = 1u;

    /* Application events admitted after each completed monitor -> raw ->
       monitor handoff remain one FIFO. Native records left in the host input
       buffer before either activation are deliberately flushed by the broker;
       this test does not claim to preserve those pre-cutover records. */
    lib_test_assert(common_session_queue_push_monitor_line(queue, &start, 0));
    lib_test_assert(common_session_queue_push_broker_completed(queue, 1, 7u));
    lib_test_assert(common_session_queue_push_kvm_for_run(queue, &raw_key, 7u));
    lib_test_assert(common_session_queue_push_broker_completed(queue, 0, 7u));
    lib_test_assert(common_session_queue_push_monitor_line(queue, &pause, 0));
    take(queue, &event); lib_test_assert(event.kind == COMMON_SESSION_EVENT_MONITOR_LINE);
    take(queue, &event); lib_test_assert(event.kind == COMMON_SESSION_EVENT_BROKER_COMPLETED &&
        event.value.broker_vm_console_current);
    take(queue, &event); lib_test_assert(event.kind == COMMON_SESSION_EVENT_KVM_INPUT &&
        common_session_accept_kvm_event(&event, 7u, COMMON_SESSION_MACHINE_RUNNING));
    take(queue, &event); lib_test_assert(event.kind == COMMON_SESSION_EVENT_BROKER_COMPLETED &&
        !event.value.broker_vm_console_current);
    take(queue, &event); lib_test_assert(event.kind == COMMON_SESSION_EVENT_MONITOR_LINE);

    /* Window -> Console input belongs to the activated run. A raw event from
       the former Console can never be delivered after Window owns a new run. */
    lib_test_assert(common_session_queue_push_kvm_for_run(queue, &raw_key, 6u));
    take(queue, &event);
    lib_test_assert(!common_session_accept_kvm_event(&event, 7u, COMMON_SESSION_MACHINE_RUNNING));

    /* Retirement is ledger cleanup, never guest input.  It remains admitted
       after a run changes so the unique source cannot leave held keys behind;
       common_session_dispatch_input still emits no guest release while stopped. */
    retired.type = KVM_EVENT_SOURCE_RETIRED;
    retired.source_identity = raw_key.source_identity;
    lib_test_assert(common_session_queue_push_kvm_for_run(queue, &retired, 7u));
    take(queue, &event); lib_test_assert(common_session_accept_kvm_event(&event, 7u,
        COMMON_SESSION_MACHINE_RUNNING));
    lib_test_assert(common_session_accept_kvm_event(&event, 8u, COMMON_SESSION_MACHINE_STOPPED));
    lib_test_assert(common_session_dispatch_input(queue, &event.value.kvm,
        COMMON_SESSION_MACHINE_STOPPED, deliver_input, LIB_NULL));

    /* A frozen Window must not affect a paused VM with any late ordinary
       input. Releases remain admitted only for the app's pressed-key ledger;
       they must not cross the separate VM ingress boundary. */
    lib_test_assert(common_session_queue_push_kvm_for_run(queue, &raw_key, 7u));
    take(queue, &event);
    lib_test_assert(!common_session_accept_kvm_event(&event, 7u, COMMON_SESSION_MACHINE_PAUSED));
    raw_key.data.key.pressed = 0u;
    lib_test_assert(common_session_queue_push_kvm_for_run(queue, &raw_key, 7u));
    take(queue, &event); lib_test_assert(common_session_accept_kvm_event(&event, 7u,
        COMMON_SESSION_MACHINE_PAUSED));
    lib_test_assert(common_session_dispatch_input(queue, &event.value.kvm,
        COMMON_SESSION_MACHINE_PAUSED, deliver_input, LIB_NULL));
    raw_mouse.type = KVM_EVENT_MOUSE;
    raw_mouse.data.mouse.buttons = 0u;
    lib_test_assert(common_session_dispatch_input(queue, &raw_mouse,
        COMMON_SESSION_MACHINE_PAUSED, deliver_input, LIB_NULL));
    lib_test_assert(delivered_guest_input == 0u);

    /* Paused still admits a current-run registered hotkey for product
       handling. A stale run remains rejected. Guest-producing hotkeys are
       consumed at the control/guest boundary while paused. */
    hotkey.type = KVM_EVENT_HOTKEY;
    lib_memory_copy(hotkey.data.hotkey.identifier, "pause-toggle",
        sizeof("pause-toggle"));
    lib_test_assert(common_session_queue_push_kvm_for_run(queue, &hotkey, 7u));
    take(queue, &event);
    lib_test_assert(common_session_accept_kvm_event(&event, 7u, COMMON_SESSION_MACHINE_PAUSED));
    lib_test_assert(common_session_queue_push_kvm_for_run(queue, &hotkey, 6u));
    take(queue, &event);
    lib_test_assert(!common_session_accept_kvm_event(&event, 7u, COMMON_SESSION_MACHINE_PAUSED));
    lib_memory_copy(hotkey.data.hotkey.identifier, "send-ctrl-alt-del",
        sizeof("send-ctrl-alt-del"));
    lib_test_assert(common_session_dispatch_input(queue, &hotkey,
        COMMON_SESSION_MACHINE_PAUSED, deliver_input, LIB_NULL));
    lib_memory_copy(hotkey.data.hotkey.identifier, "send-alt-enter",
        sizeof("send-alt-enter"));
    lib_test_assert(common_session_dispatch_input(queue, &hotkey,
        COMMON_SESSION_MACHINE_PAUSED, deliver_input, LIB_NULL));
    lib_test_assert(delivered_guest_input == 0u);

    /* A KVM component's capacity failure is a product control fact, not a
       swallowed leaf-local status. The monitor consumer reports this kind. */
    lib_test_assert(common_session_queue_push_kvm_delivery_failed(queue, 41u,
        LIB_STATUS_LIMIT_EXCEEDED, 7u));
    take(queue, &event);
    lib_test_assert(event.kind == COMMON_SESSION_EVENT_KVM_DELIVERY_FAILED);
    lib_test_assert(event.value.delivery_failure.source_identity == 41u);
    lib_test_assert(event.value.delivery_failure.status == LIB_STATUS_LIMIT_EXCEEDED);

    /* UI owns native monitor delivery now; the app queue still records its
       copied terminal failure as one control fact. */
    lib_test_assert(common_session_queue_push_console_failed(queue));
    take(queue, &event);
    lib_test_assert(event.kind == COMMON_SESSION_EVENT_CONSOLE_FAILED);
    lib_test_assert(!common_session_queue_take(queue, &event, 0u));
    common_session_queue_dispose(queue);
    return 0;
}
