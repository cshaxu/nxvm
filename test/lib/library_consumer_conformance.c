#include "lib/host/sync.h"
#include "lib/observability/outcome.h"
#include "lib/session/state.h"
#include "lib/storage/medium.h"
#include "lib/ux/actions.h"
#include "lib/ux/capture.h"
#include "lib/ux/event.h"
#include "lib/ux/frame.h"
#include "lib/ux/mailbox.h"

static int library_consumer_ux(void)
{
    ux_action_registry actions;
    ux_capture capture;
    ux_event text = { 0 };
    ux_frame published = { 0 };
    ux_frame copied;
    ux_mailbox *mailbox = LIB_NULL;
    lib_u32 generation;

    text.type = UX_EVENT_TEXT;
    text.data.text.scalar = 0x1f642u;
    if (text.type != UX_EVENT_TEXT || text.data.text.scalar != 0x1f642u ||
        ux_mailbox_create(&mailbox) != LIB_STATUS_OK) return 0;
    published.valid = LIB_TRUE;
    published.graphics = LIB_TRUE;
    published.text[0u] = 'N';
    published.text_palette[1u] = 0x00112233u;
    published.graphics_width = 320u;
    published.graphics_height = 200u;
    published.graphics_stride = 320u;
    published.graphics_palette[1u] = 0x00445566u;
    published.graphics_pixels[0u] = 1u;
    published.dirty_left = 2;
    published.dirty_top = 3;
    published.dirty_right = 318;
    published.dirty_bottom = 199;
    if (ux_mailbox_publish(mailbox, &published) != LIB_STATUS_OK ||
        ux_mailbox_capture(mailbox, &copied) != LIB_STATUS_OK ||
        copied.text[0u] != 'N' || copied.text_palette[1u] != 0x00112233u ||
        copied.graphics_palette[1u] != 0x00445566u ||
        copied.graphics_pixels[0u] != 1u || copied.dirty_left != 2 ||
        copied.dirty_bottom != 199) {
        ux_mailbox_destroy(mailbox);
        return 0;
    }
    generation = ux_mailbox_generation(mailbox);
    ux_mailbox_wake(mailbox);
    ux_actions_initialize(&actions);
    ux_capture_initialize(&capture);
    ux_capture_activate(&capture);
    if (ux_mailbox_generation(mailbox) != generation ||
        ux_actions_register(&actions, UX_KEY_F1, UX_MODIFIER_CONTROL, 9u) !=
            LIB_STATUS_OK || ux_actions_match(&actions, UX_KEY_F1,
            UX_MODIFIER_CONTROL) != 9u || !ux_capture_is_active(&capture)) {
        ux_mailbox_destroy(mailbox);
        return 0;
    }
    ux_capture_release(&capture);
    ux_mailbox_destroy(mailbox);
    return !ux_capture_is_active(&capture);
}

static int library_consumer_host_and_session(void)
{
    host_sync_event *event = LIB_NULL;
    lib_session_state *state = LIB_NULL;
    int passed = 0;

    if (host_sync_event_create(&event) != LIB_STATUS_OK ||
        lib_session_state_create(&state) != LIB_STATUS_OK ||
        host_sync_event_wait(event, 0u) != HOST_SYNC_WAIT_TIMED_OUT) goto done;
    host_sync_event_signal(event);
    lib_session_state_start(state);
    lib_session_state_request_pause(state);
    lib_session_state_acknowledge_pause(state);
    if (host_sync_event_wait(event, 0u) != HOST_SYNC_WAIT_SIGNALED ||
        lib_session_state_lifecycle(state) != LIB_SESSION_PAUSED) goto done;
    lib_session_state_resume(state);
    lib_session_state_stop(state);
    passed = lib_session_state_lifecycle(state) == LIB_SESSION_STOPPED;
done:
    lib_session_state_destroy(state);
    host_sync_event_destroy(event);
    return passed;
}

static int library_consumer_storage_and_outcome(void)
{
    static const lib_u8 source[] = { 1u, 2u, 3u };
    lib_storage_medium *medium = LIB_NULL;
    lib_observability_outcome *outcome = LIB_NULL;
    lib_observability_outcome_snapshot snapshot;
    int passed = 0;

    if (lib_storage_medium_create_overlay(source, sizeof(source), &medium) !=
            LIB_STATUS_OK ||
        lib_observability_outcome_create(&outcome) != LIB_STATUS_OK) goto done;
    if (lib_storage_medium_write_at(medium, 0u, source, sizeof(source)) !=
            LIB_STATUS_OK ||
        lib_observability_outcome_publish(outcome, LIB_STATUS_IO_ERROR) !=
            LIB_STATUS_IO_ERROR || lib_observability_outcome_capture(outcome,
            &snapshot) != LIB_STATUS_OK || !snapshot.valid ||
        snapshot.status != LIB_STATUS_IO_ERROR) goto done;
    passed = 1;
done:
    lib_storage_medium_discard(&medium);
    lib_observability_outcome_destroy(outcome);
    return passed;
}

int main(void)
{
    if (!library_consumer_ux() || !library_consumer_host_and_session() ||
        !library_consumer_storage_and_outcome()) return 1;
    puts("M5:T523:S5:NEUTRAL-CONSUMER:OK");
    return 0;
}
