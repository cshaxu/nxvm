#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/kvm-window/window.h"
#include "lib/kvm-console/console.h"


typedef struct leaf_probe {
    lib_u32 failures;
    lib_status last_failure;
} leaf_probe;

static lib_i32 leaf_input(void *opaque, const kvm_input_event *event)
{ (void)opaque; (void)event; return 1; }

static void leaf_failure(void *opaque, lib_u64 identity, lib_status status)
{
    leaf_probe *probe = (leaf_probe *)opaque;
    (void)identity;
    lib_test_assert(probe != LIB_NULL);
    ++probe->failures;
    probe->last_failure = status;
}

static lib_status leaf_stop(kvm_component *component, lib_u32 timeout_ms)
{ (void)component; (void)timeout_ms; return LIB_STATUS_OK; }

static void leaf_dispose(kvm_component *component)
{ kvm_component_mailboxes_destroy(&component->mailboxes); }

static void leaf_drain(kvm_component *component, lib_u32 kind)
{
    kvm_component_control control;
    lib_u32 count = 0u;

    while (kvm_component_mailboxes_take_control(&component->mailboxes, &control)) {
        lib_test_assert(control.kind == kind);
        lib_test_assert(control.payload[0] == 'x');
        ++count;
    }
    lib_test_assert(count == KVM_COMPONENT_CONTROL_CAPACITY);
}

int main(void)
{
    /* Each leaf embeds a complete copied-frame mailbox; keep these out of the
       small default Windows test-thread stack. */
    static kvm_window window;
    static kvm_console console;
    kvm_component_options options = { 0 };
    kvm_component_control title = { KVM_WINDOW_CONTROL_SET_TITLE, { 0 } };
    leaf_probe window_probe = { 0 };
    leaf_probe console_probe = { 0 };
    lib_u32 index;

    options.input_sink = leaf_input;
    options.failure_sink = leaf_failure;
    options.failure_context = &window_probe;
    lib_test_assert(kvm_component_initialize(&window.base, &options, leaf_stop, leaf_dispose,
        &window.pending_frame, sizeof(window.pending_frame)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    for (index = 0u; index < KVM_COMPONENT_CONTROL_CAPACITY; ++index)
        lib_test_assert(kvm_window_set_title(&window, "x") == LIB_STATUS_OK);
    lib_test_assert(kvm_window_set_title(&window, "x") == LIB_STATUS_LIMIT_EXCEEDED);
    lib_test_assert(window_probe.failures == 0u);
    leaf_drain(&window.base, KVM_WINDOW_CONTROL_SET_TITLE);

    /* Freeze uses the final ordinary slot, not a two-record transaction. */
    {
        kvm_component_control taken;
        for (index = 0u; index + 1u < KVM_COMPONENT_CONTROL_CAPACITY; ++index)
            lib_test_assert(kvm_window_set_title(&window, "x") == LIB_STATUS_OK);
        lib_test_assert(kvm_window_freeze(&window) == LIB_STATUS_OK);
        lib_test_assert(kvm_window_freeze(&window) == LIB_STATUS_LIMIT_EXCEEDED);
        for (index = 0u; index + 1u < KVM_COMPONENT_CONTROL_CAPACITY; ++index) {
            lib_test_assert(kvm_component_mailboxes_take_control(&window.base.mailboxes, &taken));
            lib_test_assert(taken.kind == KVM_WINDOW_CONTROL_SET_TITLE);
        }
        lib_test_assert(kvm_component_mailboxes_take_control(&window.base.mailboxes,
            &taken));
        lib_test_assert(taken.kind == KVM_WINDOW_CONTROL_SET_FROZEN);
        lib_test_assert(taken.payload[0] == LIB_TRUE);
        lib_test_assert(!kvm_component_mailboxes_take_control(&window.base.mailboxes,
            &taken));
        lib_test_assert(kvm_window_unfreeze(&window) == LIB_STATUS_OK);
        lib_test_assert(kvm_component_mailboxes_take_control(&window.base.mailboxes,
            &taken));
        lib_test_assert(taken.kind == KVM_WINDOW_CONTROL_SET_FROZEN);
        lib_test_assert(taken.payload[0] == LIB_FALSE);
        lib_test_assert(!kvm_component_mailboxes_take_control(&window.base.mailboxes,
            &taken));
    }

    options.failure_context = &console_probe;
    lib_test_assert(kvm_component_initialize(&console.base, &options, leaf_stop, leaf_dispose,
        &console.pending_frame, sizeof(console.pending_frame)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&console.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    /* Synthetic storage proof, not a Console command: the native Console
       worker separately rejects all ordinary kinds. */
    title.kind = 42u;
    title.payload[0] = 'x';
    for (index = 0u; index < KVM_COMPONENT_CONTROL_CAPACITY; ++index)
        lib_test_assert(kvm_component_enqueue_control(&console.base, &title) ==
            LIB_STATUS_OK);
    lib_test_assert(kvm_component_enqueue_control(&console.base, &title) ==
        LIB_STATUS_LIMIT_EXCEEDED);
    lib_test_assert(console_probe.failures == 0u);
    leaf_drain(&console.base, 42u);

    /* These are synthetic leaf storage objects: no native worker was started,
       so capacity cleanup destroys only the manually initialized mailboxes.
       Real STOP/destroy ordering is covered by the retirement barrier test. */
    kvm_component_mailboxes_destroy(&window.base.mailboxes);
    kvm_component_mailboxes_destroy(&console.base.mailboxes);
    return 0;
}
