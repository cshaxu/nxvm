#include "lib/kvm-base/worker_interface.h"

#include <assert.h>
#include <string.h>

typedef struct component_probe {
    unsigned int input_count;
    unsigned int failure_count;
    lib_u64 last_identity;
    lib_status last_failure;
    kvm_event_type last_type;
    char last_hotkey[KVM_HOTKEY_IDENTIFIER_CAPACITY];
    int accept_input;
} component_probe;

static int component_probe_input(void *opaque, const kvm_input_event *event)
{
    component_probe *probe = (component_probe *)opaque;
    if (probe == LIB_NULL || event == LIB_NULL || !probe->accept_input) return 0;
    ++probe->input_count;
    probe->last_identity = event->source_identity;
    probe->last_type = event->type;
    if (event->type == KVM_EVENT_HOTKEY)
        memcpy(probe->last_hotkey, event->data.hotkey.identifier,
            sizeof(probe->last_hotkey));
    return 1;
}

/* Leaf policy belongs after kvm-base attribution and matching. This test probe
 * models frozen Window delivery: regular matcher output succeeds but does not
 * enter the application sink; registered hotkeys still do. */
static int component_probe_hotkeys_only(void *opaque, const kvm_input_event *event)
{
    return event != LIB_NULL && event->type != KVM_EVENT_HOTKEY ? 1 :
        component_probe_input(opaque, event);
}

static void component_probe_failure(void *opaque, lib_u64 source_identity,
    lib_status status)
{
    component_probe *probe = (component_probe *)opaque;
    assert(probe != LIB_NULL);
    ++probe->failure_count;
    probe->last_identity = source_identity;
    probe->last_failure = status;
}

static lib_status component_probe_stop(kvm_component *component, lib_u32 timeout_ms)
{ (void)component; assert(timeout_ms == KVM_COMPONENT_DESTROY_TIMEOUT_MS); return LIB_STATUS_OK; }

static void component_probe_dispose(kvm_component *component)
{ kvm_component_mailboxes_destroy(&component->mailboxes); }

int main(void)
{
    component_probe probe = { 0 };
    /* Each private mailbox owns a complete kvm_frame.  Keep both test
       components out of the small default 32-bit thread stack. */
    static kvm_component first;
    static kvm_component second;
    static kvm_component third;
    kvm_component_options options = { 0 };
    kvm_input_event event = { 0 };
    kvm_component_control control = { KVM_COMPONENT_CONTROL_SET_WINDOW_FROZEN,
        { 0 } };
    kvm_component_control taken;
    kvm_component_control disable_controls[2] = {
        { KVM_COMPONENT_CONTROL_SET_WINDOW_FROZEN, { 0 } },
        { KVM_COMPONENT_CONTROL_RELEASE_WINDOW_MOUSE, { 0 } }
    };
    atomic_uint_fast64_t identity_next;
    kvm_hotkey_registry hotkeys;
    lib_u64 identity;
    unsigned int index;

    probe.accept_input = 1;
    kvm_hotkey_registry_initialize(&hotkeys);
    assert(kvm_hotkey_registry_register(&hotkeys, 'P',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT,
        "pause-toggle") == LIB_STATUS_OK);
    options.input_context = &probe;
    options.input_sink = component_probe_input;
    options.failure_context = &probe;
    options.failure_sink = component_probe_failure;
    options.hotkeys = hotkeys;
    assert(kvm_component_initialize(&first, &options, component_probe_stop,
        component_probe_dispose) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&first.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    assert(kvm_component_initialize(&second, &options, component_probe_stop,
        component_probe_dispose) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&second.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    assert(kvm_component_initialize(&third, &options, component_probe_stop,
        component_probe_dispose) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&third.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    assert(first.source_identity != 0u);
    assert(second.source_identity != 0u);
    assert(first.source_identity != second.source_identity);

    /* Source identity is a single non-repeating epoch: issuing the final
       representable value permanently exhausts it instead of wrapping. */
    atomic_init(&identity_next, UINT64_MAX - 1u);
    assert(kvm_component_allocate_source_identity(&identity_next, &identity) ==
        LIB_STATUS_OK && identity == UINT64_MAX - 1u);
    assert(kvm_component_allocate_source_identity(&identity_next, &identity) ==
        LIB_STATUS_OK && identity == UINT64_MAX);
    assert(kvm_component_allocate_source_identity(&identity_next, &identity) ==
        LIB_STATUS_LIMIT_EXCEEDED);
    assert(atomic_load_explicit(&identity_next, memory_order_relaxed) == 0u);

    event.type = KVM_EVENT_KEY;
    event.data.key.key = 'A';
    event.data.key.pressed = 1u;
    assert(kvm_component_emit(&first, &event));
    assert(probe.input_count == 1u);
    assert(probe.last_identity == first.source_identity);
    probe.accept_input = 0;
    assert(!kvm_component_emit(&first, &event));
    assert(probe.failure_count == 1u);
    assert(!kvm_component_emit(&first, &event));
    kvm_component_retire(&first, LIB_STATUS_OK);
    assert(probe.failure_count == 1u);
    assert(probe.last_failure == LIB_STATUS_IO_ERROR);
    kvm_component_mailboxes_destroy(&first.mailboxes);
    assert(kvm_component_initialize(&first, &options, component_probe_stop,
        component_probe_dispose) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&first.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);

    /* A frozen Window's delivery policy must not bypass matching. It silently
       consumes ordinary/mismatched records after kvm-base has attributed them,
       while forwarding only the copied matched identifier to the app sink. */
    probe.accept_input = 1;
    probe.input_count = 0u;
    kvm_hotkey_matcher_initialize(&first.hotkey_matcher, &hotkeys);
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.scan_code = 0x1du;
    event.data.key.pressed = 1u;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL;
    assert(kvm_component_emit_to(&first, &event, component_probe_hotkeys_only,
        &probe, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_ALT;
    event.data.key.scan_code = 0x38u;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL |
        KVM_HOTKEY_MODIFIER_ALT;
    assert(kvm_component_emit_to(&first, &event, component_probe_hotkeys_only,
        &probe, LIB_TRUE));
    event.data.key.key = 'P';
    event.data.key.scan_code = 0x19u;
    assert(kvm_component_emit_to(&first, &event, component_probe_hotkeys_only,
        &probe, LIB_TRUE));
    assert(probe.input_count == 1u && probe.last_type == KVM_EVENT_HOTKEY);
    assert(strcmp(probe.last_hotkey, "pause-toggle") == 0);
    assert(probe.last_identity == first.source_identity);
    event.data.key.key = 'X';
    event.data.key.scan_code = 0x2du;
    event.data.key.modifiers = 0u;
    assert(kvm_component_emit_to(&first, &event, component_probe_hotkeys_only,
        &probe, LIB_TRUE));
    assert(probe.input_count == 1u);

    for (index = 0u; index < KVM_COMPONENT_CONTROL_CAPACITY; ++index)
        assert(kvm_component_enqueue_controls(&second, &control, 1u) ==
            LIB_STATUS_OK);
    /* A full ordinary FIFO rejects the next request and retains every
       original record. Rejection is returned; the component remains healthy. */
    assert(kvm_component_enqueue_controls(&second, &control, 1u) ==
        LIB_STATUS_LIMIT_EXCEEDED);
    assert(probe.failure_count == 1u);
    /* STOP has one reserved FIFO slot.  A full normal queue cannot make
       destroy wait forever for a stop record it could not enqueue. */
    assert(kvm_component_request_stop(&second) == LIB_STATUS_OK);
    for (index = 0u; index < KVM_COMPONENT_CONTROL_CAPACITY; ++index) {
        assert(kvm_component_mailboxes_take_control(&second.mailboxes, &taken));
        assert(taken.kind == KVM_COMPONENT_CONTROL_SET_WINDOW_FROZEN);
    }
    assert(kvm_component_mailboxes_take_control(&second.mailboxes, &taken));
    assert(taken.kind == KVM_COMPONENT_CONTROL_STOP);
    assert(!kvm_component_mailboxes_take_control(&second.mailboxes, &taken));

    /* Multi-record control requests are all-or-nothing. This is the exact
       shape used by Window freeze: no frozen flag may be left queued
       without its following release when only one ordinary slot remains. */
    for (index = 0u; index + 1u < KVM_COMPONENT_CONTROL_CAPACITY; ++index)
        assert(kvm_component_enqueue_controls(&third, &control, 1u) ==
            LIB_STATUS_OK);
    disable_controls[0].value.window_frozen = LIB_TRUE;
    assert(kvm_component_enqueue_controls(&third, disable_controls, 2u) ==
        LIB_STATUS_LIMIT_EXCEEDED);
    assert(probe.failure_count == 1u);
    for (index = 0u; index + 1u < KVM_COMPONENT_CONTROL_CAPACITY; ++index) {
        assert(kvm_component_mailboxes_take_control(&third.mailboxes, &taken));
        assert(taken.kind == KVM_COMPONENT_CONTROL_SET_WINDOW_FROZEN);
        assert(taken.value.window_frozen == LIB_FALSE);
    }
    assert(!kvm_component_mailboxes_take_control(&third.mailboxes, &taken));

    assert(kvm_component_destroy(&first) == LIB_STATUS_OK);
    assert(kvm_component_destroy(&second) == LIB_STATUS_OK);
    assert(kvm_component_destroy(&third) == LIB_STATUS_OK);
    return 0;
}
