#include "lib/types/test.h"
#include "lib/kvm-window/window.h"


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

static lib_status leaf_stop(kvm_component *component)
{ (void)component; return LIB_STATUS_OK; }

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
    kvm_component_options options = { 0 };
    leaf_probe window_probe = { 0 };
    lib_u32 index;

    options.input_sink = leaf_input;
    options.failure_sink = leaf_failure;
    options.failure_context = &window_probe;
    lib_test_assert(kvm_component_initialize(&window.base, &options, leaf_stop, leaf_dispose) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&window.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    {
        kvm_component_control taken;
        char oversized[KVM_WINDOW_TITLE_CAPACITY+1u];
        lib_memory_set(oversized,'x',sizeof(oversized));
        oversized[sizeof(oversized)-1u]='\0';
        lib_test_assert(kvm_window_set_title(&window,oversized)==LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(kvm_window_set_title(&window,"\xed\xa0\x80")==LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(!kvm_component_mailboxes_take_control(&window.base.mailboxes,&taken));
        lib_test_assert(kvm_window_set_title(&window,"\xe6\xb5\x8b\xf0\x9f\x8e\xb5")==LIB_STATUS_OK);
        lib_test_assert(kvm_component_mailboxes_take_control(&window.base.mailboxes,&taken));
        lib_test_assert(lib_text_compare((char *)taken.payload,"\xe6\xb5\x8b\xf0\x9f\x8e\xb5")==0);
        oversized[KVM_WINDOW_TITLE_CAPACITY-1u]='\0';
        lib_test_assert(kvm_window_set_title(&window,oversized)==LIB_STATUS_OK);
        lib_test_assert(kvm_component_mailboxes_take_control(&window.base.mailboxes,&taken));
        kvm_window_options invalid_options={0};
        invalid_options.component=options;
        invalid_options.initial_title="\xc0\xaf";
        kvm_window *invalid_window=&window;
        lib_test_assert(kvm_window_create(&invalid_window,&invalid_options)==LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(invalid_window==LIB_NULL && window_probe.failures==0u);
    }
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

    /* These are synthetic leaf storage objects: no native worker was started,
       so capacity cleanup destroys only the manually initialized mailboxes.
       Real STOP/destroy ordering is covered by the retirement barrier test. */
    kvm_component_mailboxes_destroy(&window.base.mailboxes);
    return 0;
}
