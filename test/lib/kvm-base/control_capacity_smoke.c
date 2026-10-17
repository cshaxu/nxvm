#include "lib/types/test.h"
#include "lib/kvm-base/worker_interface.h"
typedef struct leaf_probe {
    lib_u32 failures;
    lib_status last_failure;
} leaf_probe;

static lib_bool leaf_input(void *opaque, const kvm_input_event *event)
{ (void)opaque; (void)event; return LIB_TRUE; }

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
    kvm_component component = {0};
    lib_u8 frame = 0;
    leaf_probe probe = {0};
    lib_u32 index;
    kvm_component_options options = {.input_sink = leaf_input, .failure_sink = leaf_failure};
    kvm_component_control title = {0};
    options.failure_context = &probe;
    lib_test_assert(kvm_component_initialize(&component, &options, leaf_stop, leaf_dispose,
        &frame, sizeof(frame)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&component.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    /* Base transports an opaque ordinary kind without interpreting it. */
    title.kind = 42u;
    title.payload[0] = 'x';
    for (index = 0u; index < KVM_COMPONENT_CONTROL_CAPACITY; ++index)
        lib_test_assert(kvm_component_enqueue_control(&component, &title) ==
            LIB_STATUS_OK);
    lib_test_assert(kvm_component_enqueue_control(&component, &title) ==
        LIB_STATUS_LIMIT_EXCEEDED);
    lib_test_assert(probe.failures == 0u);
    leaf_drain(&component, 42u);

    kvm_component_mailboxes_destroy(&component.mailboxes);
    return 0;
}
