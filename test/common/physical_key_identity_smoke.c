#include "lib/types/test.h"
#include "lib/types/file.h"
#include "common/session/control.h"

typedef struct capture {
    lib_u32 makes, breaks, extended_breaks;
} capture;

static lib_i32 receive(void *context, const kvm_input_event *event)
{
    capture *c = context;
    lib_test_assert(event->type == KVM_EVENT_KEY);
    if (event->data.key.pressed) ++c->makes;
    else {
        ++c->breaks;
        if (event->data.key.flags & KVM_KEY_FLAG_EXTENDED) ++c->extended_breaks;
    }
    return 1;
}

static void dispatch(common_session_queue *q, kvm_input_event *event, capture *c)
{
    lib_test_assert(common_session_dispatch_input(q, event, COMMON_SESSION_MACHINE_RUNNING,
        receive, c));
}

static void check(kvm_key key, lib_u16 scan, lib_i32 release_first, lib_i32 extended_first)
{
    common_session_queue storage = { 0 }, *q = &storage;
    kvm_input_event event = { 0 };
    capture c = { 0 };
    lib_test_assert(common_session_queue_initialize(q));
    event.type = KVM_EVENT_KEY;
    event.source_identity = 1;
    event.data.key.key = key;
    event.data.key.scan_code = scan;
    event.data.key.pressed = 1;
    dispatch(q, &event, &c);
    dispatch(q, &event, &c); /* Repeat never adds a second held-key entry. */
    event.data.key.flags = KVM_KEY_FLAG_EXTENDED;
    dispatch(q, &event, &c);
    if (release_first) {
        event.data.key.pressed = 0;
        event.data.key.flags = extended_first ? KVM_KEY_FLAG_EXTENDED : 0;
        dispatch(q, &event, &c);
    }
    event.type = KVM_EVENT_SOURCE_RETIRED;
    event.source_identity = 2;
    dispatch(q, &event, &c); /* Another source cannot release either key. */
    lib_test_assert(c.breaks == (lib_u32)release_first);
    event.source_identity = 1;
    dispatch(q, &event, &c);
    lib_test_assert(c.makes == 3 && c.breaks == 2 && c.extended_breaks == 1);
    dispatch(q, &event, &c);
    lib_test_assert(c.breaks == 2); /* Retirement is idempotent. */
    common_session_queue_dispose(q);
}

static void check_sources(common_session_machine_state retirement_state)
{
    common_session_queue storage = { 0 }, *q = &storage;
    kvm_input_event event = { 0 };
    capture c = { 0 };
    lib_test_assert(common_session_queue_initialize(q));
    event.type = KVM_EVENT_KEY;
    event.data.key.key = KVM_KEY_CONTROL;
    event.data.key.scan_code = 0x1d;
    event.data.key.pressed = 1;
    event.source_identity = 1;
    dispatch(q, &event, &c);
    event.source_identity = 2;
    dispatch(q, &event, &c);
    dispatch(q, &event, &c);
    event.type = KVM_EVENT_SOURCE_RETIRED;
    event.source_identity = 1;
    lib_test_assert(common_session_dispatch_input(q, &event, retirement_state, receive, &c));
    lib_test_assert(c.breaks == (retirement_state == COMMON_SESSION_MACHINE_RUNNING ? 1u : 0u));
    lib_u32 before = c.breaks;
    dispatch(q, &event, &c); /* Retired while paused must not leave a delayed break. */
    lib_test_assert(c.breaks == before);
    event.source_identity = 2;
    dispatch(q, &event, &c);
    lib_test_assert(c.makes == 3 && c.breaks == before + 1u);
    common_session_queue_dispose(q);
}

static lib_i32 receive_text(void *context, const kvm_input_event *event)
{
    lib_u32 *calls = context;
    lib_test_assert(event->type == KVM_EVENT_TEXT && event->data.text.scalar == 0x4e2du);
    lib_test_assert(event->source_identity == 17u);
    return ++*calls == 1u;
}

static void check_text(void)
{
    common_session_queue q = { 0 };
    kvm_input_event event = { 0 };
    lib_u32 calls = 0u;
    event.type = KVM_EVENT_TEXT;
    event.source_identity = 17u;
    event.data.text.scalar = 0x4e2du;
    lib_test_assert(common_session_dispatch_input(&q, &event, COMMON_SESSION_MACHINE_PAUSED, receive_text, &calls));
    lib_test_assert(common_session_dispatch_input(&q, &event, COMMON_SESSION_MACHINE_STOPPED, receive_text, &calls));
    lib_test_assert(calls == 0u);
    lib_test_assert(common_session_dispatch_input(&q, &event, COMMON_SESSION_MACHINE_RUNNING, receive_text, &calls));
    lib_test_assert(!common_session_dispatch_input(&q, &event, COMMON_SESSION_MACHINE_RUNNING, receive_text, &calls));
    lib_test_assert(calls == 2u && q.pressed_count == 0u);
}

static void check_capacity(void)
{
    common_session_queue q = { 0 };
    kvm_input_event event = { 0 };
    capture c = { 0 };
    const lib_u32 capacity = COMMON_SESSION_EVENT_PRESSED_CAPACITY;
    lib_test_assert(common_session_queue_initialize(&q));
    event.type = KVM_EVENT_KEY;
    event.data.key.key = KVM_KEY_CONTROL;
    event.data.key.scan_code = 0x1d;
    event.data.key.pressed = 1u;
    /* Same physical key on distinct sources must occupy distinct entries. */
    for (lib_u32 i = 1u; i <= capacity; ++i) {
        event.source_identity = i;
        dispatch(&q, &event, &c);
        lib_test_assert(q.pressed_count == i && c.makes == i);
    }
    event.source_identity = 1u;
    dispatch(&q, &event, &c); /* Repeat at capacity still succeeds. */
    lib_test_assert(q.pressed_count == capacity && c.makes == capacity + 1u);
    /* Neither a new physical key nor a new source may bypass the ledger. */
    for (lib_u32 i = 0u; i < 2u; ++i) {
        event.source_identity = i == 0u ? 1u : capacity + 1u;
        event.data.key.flags = i == 0u ? KVM_KEY_FLAG_EXTENDED : 0u;
        lib_test_assert(!common_session_dispatch_input(&q, &event,
            COMMON_SESSION_MACHINE_RUNNING, receive, &c));
        lib_test_assert(q.pressed_count == capacity && c.makes == capacity + 1u);
    }
    event.type = KVM_EVENT_SOURCE_RETIRED;
    dispatch(&q, &event, &c); /* Rejected source has nothing to release. */
    lib_test_assert(q.pressed_count == capacity && c.breaks == 0u);
    event.type = KVM_EVENT_KEY;
    event.source_identity = 1u;
    event.data.key.pressed = 0u;
    dispatch(&q, &event, &c);
    lib_test_assert(q.pressed_count == capacity - 1u && c.breaks == 1u);
    event.source_identity = capacity + 1u;
    event.data.key.pressed = 1u;
    dispatch(&q, &event, &c); /* Freed slot admits the formerly rejected key. */
    lib_test_assert(q.pressed_count == capacity && c.makes == capacity + 2u);
    event.type = KVM_EVENT_SOURCE_RETIRED;
    for (lib_u32 i = 1u; i <= capacity + 1u; ++i) {
        event.source_identity = i;
        dispatch(&q, &event, &c);
    }
    lib_test_assert(q.pressed_count == 0u && c.breaks == capacity + 1u);
    lib_test_assert(c.extended_breaks == 0u); /* Rejected extended key was never stored. */
    for (lib_u32 i = 1u; i <= capacity + 1u; ++i) {
        event.source_identity = i;
        dispatch(&q, &event, &c);
    }
    lib_test_assert(c.breaks == capacity + 1u); /* No duplicate retirement releases. */
    common_session_queue_dispose(&q);
}

int main(void)
{
    lib_i32 release_first, extended_first;
    for (release_first = 0; release_first <= 1; ++release_first)
        for (extended_first = 0; extended_first <= 1; ++extended_first) {
            check(KVM_KEY_CONTROL, 0x1d, release_first, extended_first);
            check(KVM_KEY_ALT, 0x38, release_first, extended_first);
            check(KVM_KEY_CONTROL, 0, release_first, extended_first);
        }
    check_sources(COMMON_SESSION_MACHINE_RUNNING);
    check_sources(COMMON_SESSION_MACHINE_PAUSED);
    check_text();
    check_capacity();
    return 0;
}
