#include "lib/kvm-window/window.h"
#include "lib/kvm-console/console.h"

#include <assert.h>

typedef struct leaf_probe {
    unsigned int failures;
    lib_status last_failure;
} leaf_probe;

static int leaf_input(void *opaque, const kvm_input_event *event)
{ (void)opaque; (void)event; return 1; }

static void leaf_failure(void *opaque, lib_u64 identity, lib_status status)
{
    leaf_probe *probe = (leaf_probe *)opaque;
    (void)identity;
    assert(probe != LIB_NULL);
    ++probe->failures;
    probe->last_failure = status;
}

static lib_status leaf_stop(kvm_component *component, lib_u32 timeout_ms)
{ (void)component; (void)timeout_ms; return LIB_STATUS_OK; }

static void leaf_dispose(kvm_component *component)
{ kvm_component_mailboxes_destroy(&component->mailboxes); }

static void leaf_drain(kvm_component *component)
{
    kvm_component_control control;
    unsigned int count = 0u;

    while (kvm_component_mailboxes_take_control(&component->mailboxes, &control)) {
        assert(control.kind == KVM_COMPONENT_CONTROL_SET_WINDOW_TITLE);
        assert(control.value.title[0] == 'x');
        ++count;
    }
    assert(count == KVM_COMPONENT_CONTROL_CAPACITY);
}

int main(void)
{
    /* Each leaf embeds a complete copied-frame mailbox; keep these out of the
       small default Windows test-thread stack. */
    static kvm_window window;
    static kvm_console console;
    kvm_component_options options = { 0 };
    kvm_component_control title = { KVM_COMPONENT_CONTROL_SET_WINDOW_TITLE, { 0 } };
    leaf_probe window_probe = { 0 };
    leaf_probe console_probe = { 0 };
    unsigned int index;

    options.input_sink = leaf_input;
    options.failure_sink = leaf_failure;
    options.failure_context = &window_probe;
    assert(kvm_component_initialize(&window.base, &options, leaf_stop,
        leaf_dispose) == LIB_STATUS_OK);
    for (index = 0u; index < KVM_COMPONENT_CONTROL_CAPACITY; ++index)
        assert(kvm_window_set_title(&window, "x") == LIB_STATUS_OK);
    assert(kvm_window_set_title(&window, "x") == LIB_STATUS_LIMIT_EXCEEDED);
    assert(window_probe.failures == 0u);
    leaf_drain(&window.base);

    /* Window freeze is one atomic ordered control batch: first it makes future
       capture impossible, then it releases current capture. Unfreeze is one
       independent permission change and never implies a capture request. */
    {
        kvm_component_control taken;
        assert(kvm_window_freeze(&window) == LIB_STATUS_OK);
        assert(kvm_component_mailboxes_take_control(&window.base.mailboxes,
            &taken));
        assert(taken.kind == KVM_COMPONENT_CONTROL_SET_WINDOW_FROZEN);
        assert(taken.value.window_frozen == LIB_TRUE);
        assert(kvm_component_mailboxes_take_control(&window.base.mailboxes,
            &taken));
        assert(taken.kind == KVM_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE);
        assert(!kvm_component_mailboxes_take_control(&window.base.mailboxes,
            &taken));
        assert(kvm_window_unfreeze(&window) == LIB_STATUS_OK);
        assert(kvm_component_mailboxes_take_control(&window.base.mailboxes,
            &taken));
        assert(taken.kind == KVM_COMPONENT_CONTROL_SET_WINDOW_FROZEN);
        assert(taken.value.window_frozen == LIB_FALSE);
        assert(!kvm_component_mailboxes_take_control(&window.base.mailboxes,
            &taken));
    }

    options.failure_context = &console_probe;
    assert(kvm_component_initialize(&console.base, &options, leaf_stop,
        leaf_dispose) == LIB_STATUS_OK);
    title.value.title[0] = 'x';
    for (index = 0u; index < KVM_COMPONENT_CONTROL_CAPACITY; ++index)
        assert(kvm_component_enqueue_controls(&console.base, &title, 1u) ==
            LIB_STATUS_OK);
    assert(kvm_component_enqueue_controls(&console.base, &title, 1u) ==
        LIB_STATUS_LIMIT_EXCEEDED);
    assert(console_probe.failures == 0u);
    leaf_drain(&console.base);

    /* These are synthetic leaf storage objects: no native worker was started,
       so capacity cleanup destroys only the manually initialized mailboxes.
       Real STOP/destroy ordering is covered by the retirement barrier test. */
    kvm_component_mailboxes_destroy(&window.base.mailboxes);
    kvm_component_mailboxes_destroy(&console.base.mailboxes);
    return 0;
}
