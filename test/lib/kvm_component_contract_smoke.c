#include "lib/types/test.h"
#include "lib/kvm-base/worker_interface.h"


typedef struct component_probe {
    lib_u32 input_count;
    lib_u32 failure_count;
    lib_u64 last_identity;
    lib_status last_failure;
    kvm_event_type last_type;
    char last_hotkey[KVM_HOTKEY_IDENTIFIER_CAPACITY];
    lib_bool accept_input;
} component_probe;

static lib_bool component_probe_input(void *opaque, const kvm_input_event *event)
{
    component_probe *probe = (component_probe *)opaque;
    if (probe == LIB_NULL || event == LIB_NULL || !probe->accept_input) return LIB_FALSE;
    ++probe->input_count;
    probe->last_identity = event->source_identity;
    probe->last_type = event->type;
    if (event->type == KVM_EVENT_HOTKEY)
        lib_memory_copy(probe->last_hotkey, event->data.hotkey.identifier,
            sizeof(probe->last_hotkey));
    return LIB_TRUE;
}

/* Leaf policy belongs after kvm-base attribution and matching. This test probe
 * models frozen Window delivery: regular matcher output succeeds but does not
 * enter the application sink; registered hotkeys still do. */
static lib_bool component_probe_hotkeys_only(void *opaque, const kvm_input_event *event)
{
    return event != LIB_NULL && event->type != KVM_EVENT_HOTKEY ? LIB_TRUE :
        component_probe_input(opaque, event);
}

static void component_probe_failure(void *opaque, lib_u64 source_identity,
    lib_status status)
{
    component_probe *probe = (component_probe *)opaque;
    lib_test_assert(probe != LIB_NULL);
    ++probe->failure_count;
    probe->last_identity = source_identity;
    probe->last_failure = status;
}

static lib_status component_probe_stop(kvm_component *component, lib_u32 timeout_ms)
{ (void)component; lib_test_assert(timeout_ms == KVM_COMPONENT_DESTROY_TIMEOUT_MS); return LIB_STATUS_OK; }

static void component_probe_dispose(kvm_component *component)
{ kvm_component_mailboxes_destroy(&component->mailboxes); }

int main(void)
{
    component_probe probe = { 0 };
    lib_u8 storage[3];
    static kvm_component first;
    static kvm_component second;
    static kvm_component third;
    kvm_component_options options = { 0 };
    kvm_input_event event = { 0 };
    kvm_component_control control = { 0xabcdef01u,
        { 0 } };
    kvm_component_control taken;
    lib_atomic_u64 identity_next;
    kvm_hotkey_registry hotkeys;
    lib_u64 identity;
    lib_u32 index;

    probe.accept_input = LIB_TRUE;
    kvm_hotkey_registry_initialize(&hotkeys);
    lib_test_assert(kvm_hotkey_registry_register(&hotkeys, 'P',
        KVM_HOTKEY_MODIFIER_CONTROL | KVM_HOTKEY_MODIFIER_ALT,
        "pause-toggle") == LIB_STATUS_OK);
    options.input_context = &probe;
    options.input_sink = component_probe_input;
    options.failure_context = &probe;
    options.failure_sink = component_probe_failure;
    options.hotkeys = hotkeys;
    lib_test_assert(kvm_component_initialize(&first, &options, component_probe_stop,
        component_probe_dispose, &storage[0], 1u) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&first.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_initialize(&second, &options, component_probe_stop,
        component_probe_dispose, &storage[1], 1u) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&second.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_initialize(&third, &options, component_probe_stop,
        component_probe_dispose, &storage[2], 1u) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&third.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(first.source_identity != 0u);
    lib_test_assert(second.source_identity != 0u);
    lib_test_assert(first.source_identity != second.source_identity);

    /* The transport cannot restrict consumer opcodes or interpret payloads.
       Copy every byte (including embedded zeroes) before the caller mutates it. */
    control.kind = LIB_UINT32_MAX;
    for (index = 0u; index < sizeof(control.payload); ++index)
        control.payload[index] = (lib_u8)index;
    lib_test_assert(kvm_component_enqueue_control(&second, &control) == LIB_STATUS_OK);
    lib_memory_set(control.payload, 0xff, sizeof(control.payload));
    lib_test_assert(kvm_component_mailboxes_take_control(&second.mailboxes, &taken));
    lib_test_assert(taken.kind == LIB_UINT32_MAX);
    for (index = 0u; index < sizeof(taken.payload); ++index)
        lib_test_assert(taken.payload[index] == (lib_u8)index);
    control = (kvm_component_control){ 0xabcdef01u, { 0 } };

    /* Source identity is a single non-repeating epoch: issuing the final
       representable value permanently exhausts it instead of wrapping. */
    identity_next = LIB_UINT64_MAX - 1u;
    lib_test_assert(kvm_component_allocate_source_identity(&identity_next, &identity) ==
        LIB_STATUS_OK && identity == LIB_UINT64_MAX - 1u);
    lib_test_assert(kvm_component_allocate_source_identity(&identity_next, &identity) ==
        LIB_STATUS_OK && identity == LIB_UINT64_MAX);
    lib_test_assert(kvm_component_allocate_source_identity(&identity_next, &identity) ==
        LIB_STATUS_LIMIT_EXCEEDED);
    lib_test_assert(lib_atomic_u64_load_explicit(&identity_next,
        LIB_MEMORY_ORDER_RELAXED) == 0u);

    event.type = KVM_EVENT_KEY;
    event.data.key.key = 'A';
    event.data.key.pressed = LIB_TRUE;
    lib_test_assert(kvm_component_emit(&first, &event));
    lib_test_assert(probe.input_count == 1u);
    lib_test_assert(probe.last_identity == first.source_identity);
    probe.accept_input = LIB_FALSE;
    lib_test_assert(!kvm_component_emit(&first, &event));
    lib_test_assert(probe.failure_count == 1u);
    lib_test_assert(!kvm_component_emit(&first, &event));
    kvm_component_retire(&first, LIB_STATUS_OK);
    lib_test_assert(probe.failure_count == 1u);
    lib_test_assert(probe.last_failure == LIB_STATUS_IO_ERROR);
    kvm_component_mailboxes_destroy(&first.mailboxes);
    lib_test_assert(kvm_component_initialize(&first, &options, component_probe_stop,
        component_probe_dispose, &storage[0], 1u) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&first.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);

    /* A frozen Window's delivery policy must not bypass matching. It silently
       consumes ordinary/mismatched records after kvm-base has attributed them,
       while forwarding only the copied matched identifier to the app sink. */
    probe.accept_input = LIB_TRUE;
    probe.input_count = 0u;
    kvm_hotkey_matcher_initialize(&first.hotkey_matcher, &hotkeys);
    event.data.key.key = KVM_HOTKEY_KEY_CONTROL;
    event.data.key.scan_code = 0x1du;
    event.data.key.pressed = LIB_TRUE;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL;
    lib_test_assert(kvm_component_emit_to(&first, &event, component_probe_hotkeys_only,
        &probe, LIB_TRUE));
    event.data.key.key = KVM_HOTKEY_KEY_ALT;
    event.data.key.scan_code = 0x38u;
    event.data.key.modifiers = KVM_HOTKEY_MODIFIER_CONTROL |
        KVM_HOTKEY_MODIFIER_ALT;
    lib_test_assert(kvm_component_emit_to(&first, &event, component_probe_hotkeys_only,
        &probe, LIB_TRUE));
    event.data.key.key = 'P';
    event.data.key.scan_code = 0x19u;
    lib_test_assert(kvm_component_emit_to(&first, &event, component_probe_hotkeys_only,
        &probe, LIB_TRUE));
    lib_test_assert(probe.input_count == 1u && probe.last_type == KVM_EVENT_HOTKEY);
    lib_test_assert(lib_text_compare(probe.last_hotkey, "pause-toggle") == 0);
    lib_test_assert(probe.last_identity == first.source_identity);
    event.data.key.key = 'X';
    event.data.key.scan_code = 0x2du;
    event.data.key.modifiers = 0u;
    lib_test_assert(kvm_component_emit_to(&first, &event, component_probe_hotkeys_only,
        &probe, LIB_TRUE));
    lib_test_assert(probe.input_count == 1u);

    for (index = 0u; index < KVM_COMPONENT_CONTROL_CAPACITY; ++index)
        lib_test_assert(kvm_component_enqueue_control(&second, &control) ==
            LIB_STATUS_OK);
    /* A full ordinary FIFO rejects the next request and retains every
       original record. Rejection is returned; the component remains healthy. */
    lib_test_assert(kvm_component_enqueue_control(&second, &control) ==
        LIB_STATUS_LIMIT_EXCEEDED);
    lib_test_assert(probe.failure_count == 1u);
    /* STOP has one reserved FIFO slot.  A full normal queue cannot make
       destroy wait forever for a stop record it could not enqueue. */
    lib_test_assert(kvm_component_request_stop(&second) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_request_stop(&second) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_enqueue_control(&second, &control) == LIB_STATUS_INVALID_STATE);
    for (index = 0u; index < KVM_COMPONENT_CONTROL_CAPACITY; ++index) {
        lib_test_assert(kvm_component_mailboxes_take_control(&second.mailboxes, &taken));
        lib_test_assert(taken.kind == 0xabcdef01u);
    }
    lib_test_assert(kvm_component_mailboxes_take_control(&second.mailboxes, &taken));
    lib_test_assert(taken.kind == KVM_COMPONENT_CONTROL_STOP);
    lib_test_assert(!kvm_component_mailboxes_take_control(&second.mailboxes, &taken));

    /* The last ordinary slot accepts exactly one record; rejection is inert. */
    for (index = 0u; index + 1u < KVM_COMPONENT_CONTROL_CAPACITY; ++index)
        lib_test_assert(kvm_component_enqueue_control(&third, &control) ==
            LIB_STATUS_OK);
    control.payload[0] = LIB_TRUE;
    lib_test_assert(kvm_component_enqueue_control(&third, &control) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_enqueue_control(&third, &control) ==
        LIB_STATUS_LIMIT_EXCEEDED);
    lib_test_assert(probe.failure_count == 1u);
    for (index = 0u; index + 1u < KVM_COMPONENT_CONTROL_CAPACITY; ++index) {
        lib_test_assert(kvm_component_mailboxes_take_control(&third.mailboxes, &taken));
        lib_test_assert(taken.kind == 0xabcdef01u);
        lib_test_assert(taken.payload[0] == LIB_FALSE);
    }
    lib_test_assert(kvm_component_mailboxes_take_control(&third.mailboxes, &taken));
    lib_test_assert(taken.payload[0] == LIB_TRUE);
    lib_test_assert(!kvm_component_mailboxes_take_control(&third.mailboxes, &taken));

    lib_test_assert(kvm_component_destroy(&first) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_destroy(&second) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_destroy(&third) == LIB_STATUS_OK);
    return 0;
}
