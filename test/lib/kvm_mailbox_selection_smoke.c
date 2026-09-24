#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/kvm-base/mailbox_interface.h"

static lib_u32 mutex_creates, reject_mutex;
static lib_status create_mutex(base_sync_mutex **out)
{
    if (++mutex_creates == reject_mutex) { *out = LIB_NULL; return LIB_STATUS_NO_MEMORY; }
    return base_sync_mutex_create(out);
}
/* Count actual wake requests at the platform boundary, including failures. */
static lib_u32 creates, destroys, signals, notifications;
static lib_status allocation = LIB_STATUS_NO_MEMORY;
static lib_i32 token;
lib_status create_event(base_sync_event_mode mode, base_sync_event **out)
{
    lib_test_assert(mode == BASE_SYNC_EVENT_AUTO_RESET);
    ++creates;
    *out = allocation == LIB_STATUS_OK ? (base_sync_event *)&token : LIB_NULL;
    return allocation;
}
static void destroy_event(base_sync_event *wake)
{ if (wake != LIB_NULL) { lib_test_assert(wake == (base_sync_event *)&token); ++destroys; } }
static lib_status signal_event(base_sync_event *wake)
{ lib_test_assert(wake == (base_sync_event *)&token); ++signals; return LIB_STATUS_OK; }
static lib_status notify(void *context)
{ lib_test_assert(context == &token); ++notifications; return LIB_STATUS_OK; }
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
        lib_test_assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_NO_MEMORY);
        lib_test_assert(mailbox.frame_lock == LIB_NULL && mailbox.control_lock == LIB_NULL);
        lib_test_assert(mailbox.wake == LIB_NULL && creates == 0);
        kvm_component_mailboxes_destroy(&mailbox);
    }
    reject_mutex = 0;
    lib_test_assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_OK);
    lib_test_assert(creates == 0u && mailbox.notify == LIB_NULL && mailbox.wake == LIB_NULL);
    lib_test_assert(kvm_component_mailboxes_notify(&mailbox) == LIB_STATUS_INVALID_STATE);
    kvm_component_mailboxes_destroy(&mailbox);
    lib_test_assert(destroys == 0u);

    /* Window succeeds even when default-wake allocation would fail. */
    lib_test_assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&mailbox, notify, &token) == LIB_STATUS_OK);
    lib_test_assert(creates == 0u && mailbox.wake == LIB_NULL);
    lib_test_assert(kvm_component_mailboxes_select_notify(&mailbox, LIB_NULL, LIB_NULL) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(kvm_component_mailboxes_select_notify(&mailbox, notify, LIB_NULL) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(kvm_component_mailboxes_notify(&mailbox) == LIB_STATUS_OK);
    lib_test_assert(notifications == 1u && creates == 0u && signals == 0u);
    kvm_component_mailboxes_destroy(&mailbox);
    lib_test_assert(destroys == 0u);

    /* Failed Console selection owns nothing and can be safely disposed. */
    lib_test_assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&mailbox, LIB_NULL, LIB_NULL) == LIB_STATUS_NO_MEMORY);
    lib_test_assert(creates == 1u && mailbox.notify == LIB_NULL && mailbox.wake == LIB_NULL);
    kvm_component_mailboxes_destroy(&mailbox);
    lib_test_assert(destroys == 0u);
    allocation = LIB_STATUS_OK;
    lib_test_assert(kvm_component_mailboxes_create(&mailbox, &storage, sizeof(storage)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&mailbox, LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(creates == 2u && mailbox.wake != LIB_NULL);
    lib_test_assert(kvm_component_mailboxes_select_notify(&mailbox, LIB_NULL, LIB_NULL) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(kvm_component_mailboxes_select_notify(&mailbox, notify, &token) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(kvm_component_mailboxes_notify(&mailbox) == LIB_STATUS_OK);
    lib_test_assert(signals == 1u && notifications == 1u && creates == 2u);
    kvm_component_mailboxes_destroy(&mailbox);
    lib_test_assert(destroys == 1u);
    return 0;
}
