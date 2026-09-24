#include "lib/kvm-base/mailbox_interface.h"
#include <assert.h>

static lib_u32 mutex_creates, reject_mutex;
static lib_status create_mutex(base_sync_mutex **out)
{
    if (++mutex_creates == reject_mutex) { *out = NULL; return LIB_STATUS_NO_MEMORY; }
    return base_sync_mutex_create(out);
}
/* Count actual wake requests at the platform boundary, including failures. */
static lib_u32 creates, destroys, signals, notifications;
static lib_status allocation = LIB_STATUS_NO_MEMORY;
static lib_i32 token;
lib_status create_event(base_sync_event_mode mode, base_sync_event **out)
{
    assert(mode == BASE_SYNC_EVENT_AUTO_RESET);
    ++creates;
    *out = allocation == LIB_STATUS_OK ? (base_sync_event *)&token : NULL;
    return allocation;
}
static void destroy_event(base_sync_event *wake)
{ if (wake != NULL) { assert(wake == (base_sync_event *)&token); ++destroys; } }
static lib_status signal_event(base_sync_event *wake)
{ assert(wake == (base_sync_event *)&token); ++signals; return LIB_STATUS_OK; }
static lib_status notify(void *context)
{ assert(context == &token); ++notifications; return LIB_STATUS_OK; }
#define base_sync_event_create create_event
#define base_sync_event_destroy destroy_event
#define base_sync_event_signal signal_event
#define base_sync_mutex_create create_mutex
#include "lib/kvm-base/mailbox.c"
#undef base_sync_mutex_create
#undef base_sync_event_create
#undef base_sync_event_destroy
#undef base_sync_event_signal

int main(void)
{
    lib_u32 storage;
    static kvm_component_mailboxes mailbox;
    for (reject_mutex = 1; reject_mutex <= 2; ++reject_mutex) {
        mutex_creates = 0;
        assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_NO_MEMORY);
        assert(mailbox.frame_lock == NULL && mailbox.control_lock == NULL);
        assert(mailbox.wake == NULL && creates == 0);
        kvm_component_mailboxes_destroy(&mailbox);
    }
    reject_mutex = 0;
    assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_OK);
    assert(creates == 0u && mailbox.notify == NULL && mailbox.wake == NULL);
    assert(kvm_component_mailboxes_notify(&mailbox) == LIB_STATUS_INVALID_STATE);
    kvm_component_mailboxes_destroy(&mailbox);
    assert(destroys == 0u);

    /* Window succeeds even when default-wake allocation would fail. */
    assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&mailbox, notify, &token) == LIB_STATUS_OK);
    assert(creates == 0u && mailbox.wake == NULL);
    assert(kvm_component_mailboxes_select_notify(&mailbox, NULL, NULL) == LIB_STATUS_INVALID_STATE);
    assert(kvm_component_mailboxes_select_notify(&mailbox, notify, NULL) == LIB_STATUS_INVALID_STATE);
    assert(kvm_component_mailboxes_notify(&mailbox) == LIB_STATUS_OK);
    assert(notifications == 1u && creates == 0u && signals == 0u);
    kvm_component_mailboxes_destroy(&mailbox);
    assert(destroys == 0u);

    /* Failed Console selection owns nothing and can be safely disposed. */
    assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&mailbox, NULL, NULL) == LIB_STATUS_NO_MEMORY);
    assert(creates == 1u && mailbox.notify == NULL && mailbox.wake == NULL);
    kvm_component_mailboxes_destroy(&mailbox);
    assert(destroys == 0u);
    allocation = LIB_STATUS_OK;
    assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_OK);
    assert(kvm_component_mailboxes_select_notify(&mailbox, NULL, NULL) == LIB_STATUS_OK);
    assert(creates == 2u && mailbox.wake != NULL);
    assert(kvm_component_mailboxes_select_notify(&mailbox, NULL, NULL) == LIB_STATUS_INVALID_STATE);
    assert(kvm_component_mailboxes_select_notify(&mailbox, notify, &token) == LIB_STATUS_INVALID_STATE);
    assert(kvm_component_mailboxes_notify(&mailbox) == LIB_STATUS_OK);
    assert(signals == 1u && notifications == 1u && creates == 2u);
    kvm_component_mailboxes_destroy(&mailbox);
    assert(destroys == 1u);
    return 0;
}
