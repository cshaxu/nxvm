#include "common/session/control.h"
#include <assert.h>

typedef struct capture {
    unsigned makes, breaks, extended_breaks;
} capture;

static int receive(void *context, const kvm_input_event *event)
{
    capture *c = context;
    assert(event->type == KVM_EVENT_KEY);
    if (event->data.key.pressed) ++c->makes;
    else {
        ++c->breaks;
        if (event->data.key.flags & KVM_KEY_FLAG_EXTENDED) ++c->extended_breaks;
    }
    return 1;
}

static void dispatch(common_session_queue *q, kvm_input_event *event, capture *c)
{
    assert(common_session_dispatch_input(q, event, COMMON_SESSION_MACHINE_RUNNING,
        receive, c));
}

static void check(kvm_key key, lib_u16 scan, int release_first, int extended_first)
{
    common_session_queue *q = NULL;
    kvm_input_event event = { 0 };
    capture c = { 0 };
    assert(common_session_queue_create(&q));
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
    assert(c.breaks == (unsigned)release_first);
    event.source_identity = 1;
    dispatch(q, &event, &c);
    assert(c.makes == 3 && c.breaks == 2 && c.extended_breaks == 1);
    dispatch(q, &event, &c);
    assert(c.breaks == 2); /* Retirement is idempotent. */
    common_session_queue_destroy(q);
}

int main(void)
{
    int release_first, extended_first;
    for (release_first = 0; release_first <= 1; ++release_first)
        for (extended_first = 0; extended_first <= 1; ++extended_first) {
            check(KVM_KEY_CONTROL, 0x1d, release_first, extended_first);
            check(KVM_KEY_ALT, 0x38, release_first, extended_first);
            check(KVM_KEY_CONTROL, 0, release_first, extended_first);
        }
    return 0;
}
