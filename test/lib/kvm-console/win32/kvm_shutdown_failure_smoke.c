#include "../../fixtures/console_frame.h"
static kvm_frame_exchange *frames;
#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/kvm-console/console.h"
#include "lib/console/binding_interface.h"
#include "lib/types/win32/sync.h"
#include "../../fixtures/lifetime.h"

static lib_win32_handle asleep;
static base_sync_event *rejected_wake;
static base_sync_event *held_worker;
static lib_bool reject_attach;
static lib_u32 attempts, retired, failures, joins;
static lib_status (*real_signal)(base_sync_event *) = base_sync_event_signal;
static lib_status (*real_set_event_sink)(lib_console *, lib_console_event_sink, void *) =
    lib_console_set_event_sink;
static lib_status reject_signal(base_sync_event *event)
{ if(event==rejected_wake) { ++attempts; return LIB_STATUS_IO_ERROR; } return real_signal(event); }
static lib_status reject_event_sink(lib_console *console,
    lib_console_event_sink sink, void *context)
{
    if (reject_attach && sink != LIB_NULL) return LIB_STATUS_IO_ERROR;
    return real_set_event_sink(console, sink, context);
}
static lib_win32_bool (LIB_WIN32_WINAPI *signal_asleep)(lib_win32_handle) = lib_win32_set_event;
static lib_win32_dword (LIB_WIN32_WINAPI *wait_for_signal)(lib_win32_handle,lib_win32_dword) = lib_win32_wait_for_single_object;
#define base_sync_event_signal reject_signal
#include "lib/kvm-base/mailbox.c"
#undef base_sync_event_signal
static base_sync_wait_result observed_wait_any(base_sync_event *const *events,
    lib_u32 count, const base_sync_task *task, lib_u32 timeout, lib_u32 *index)
{
    if (held_worker != LIB_NULL)
        (void)base_sync_event_wait(held_worker, LIB_UINT32_MAX);
    signal_asleep(asleep);
    return base_sync_wait_any(events, count, task, timeout, index);
}
static lib_win32_dword LIB_WIN32_WINAPI bounded_join(lib_win32_handle h,lib_win32_dword timeout)
{
    lib_test_assert(timeout==(lifetime_fault==1u || lifetime_fault==3u ? 0u : LIB_WIN32_INFINITE));
    ++joins;
    if (lifetime_fault == 2u || lifetime_fault == 4u) return LIB_WIN32_WAIT_FAILED;
    return wait_for_signal(h,timeout);
}
static lib_status terminal_signal(base_sync_event *event)
{
    if (lifetime_fault == 1u || lifetime_fault == 3u) return LIB_STATUS_IO_ERROR;
    return real_signal(event);
}
static lib_win32_bool LIB_WIN32_WINAPI close_worker_handle(lib_win32_handle worker)
{
    lib_test_assert(lib_win32_close_handle(worker));
    return lifetime_fault == 5u ? LIB_WIN32_FALSE : LIB_WIN32_TRUE;
}
#undef lib_win32_close_handle
#define lib_win32_close_handle close_worker_handle
#undef lib_win32_wait_for_single_object
#define lib_win32_wait_for_single_object bounded_join
#define base_sync_wait_any observed_wait_any
#define lib_console_set_event_sink reject_event_sink
#define base_sync_event_signal terminal_signal
#include "lib/kvm-console/win32/component.c"
#undef base_sync_event_signal
#undef base_sync_wait_any
#undef lib_console_set_event_sink

static lib_bool input(void *p,const kvm_input_event *e)
{ (void)p; lib_test_assert(e->type==KVM_EVENT_SOURCE_RETIRED); ++retired; return LIB_TRUE; }
static void failure(void *p,lib_u64 id,lib_status s)
{ (void)p; lib_test_assert(id && s==LIB_STATUS_IO_ERROR); ++failures; }
int main(int argc, char **argv)
{
    if (argc == 2) {
        lifetime_fault = (lib_u32)(argv[1][0] - '0');
        lib_test_assert(lib_test_at_exit(unexpected_cleanup) == 0);
        lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &held_worker) == LIB_STATUS_OK);
        reject_attach = lifetime_fault <= 2u;
        kvm_console_options o = { .input_sink = input, .failure_sink = failure };
        kvm_console *c = LIB_NULL;
        (void)kvm_console_create(&c, &o);
        (void)kvm_console_destroy(c);
        return 90; /* The parent rejects an ordinary return as well as a crash. */
    }
    lib_test_assert(kvm_frame_exchange_create(&frames, sizeof(kvm_console_text_frame), kvm_console_frame_retire) == LIB_STATUS_OK);
    lib_test_assert(kvm_console_destroy(LIB_NULL)==LIB_STATUS_OK);

    for(lib_u32 mode=0;mode<4;++mode) {
        kvm_console_options o={0}; kvm_console *c=LIB_NULL;
        asleep=lib_win32_create_event_a(LIB_NULL,LIB_WIN32_TRUE,LIB_WIN32_FALSE,LIB_NULL);
        attempts=retired=failures=joins=0;
        o.input_sink=input; o.failure_sink=failure;
        lib_test_assert(kvm_console_create(&c,&o)==LIB_STATUS_OK);
        rejected_wake=c->base.mailboxes.wake;
        lib_test_assert(wait_for_signal(asleep,3000)==LIB_WIN32_WAIT_OBJECT_0);
        if(mode==0) lib_test_assert(kvm_component_request_stop(&c->base)==LIB_STATUS_OK);
        if(mode==1) {
            kvm_component_control title={.kind=42u};
            lib_test_assert(kvm_component_enqueue_control(&c->base,&title)==LIB_STATUS_OK);
        }
        if(mode==2) {
            kvm_console_text_frame frame={0};
            lib_test_assert(kvm_console_frame_prepare(&frame, 80u, 25u, 80u) == LIB_STATUS_OK);
            lib_test_assert(test_console_frame(c, frames,&frame)==LIB_STATUS_OK);
            kvm_console_frame_release(&frame);
        }
        if(mode==3) {
            lib_console_event activated={.kind=LIB_CONSOLE_EVENT_ACTIVATED,.binding_generation=1};
            lib_test_assert(lib_console_bind_generation(c->logical_console,1)==LIB_STATUS_OK);
            lib_test_assert(lib_console_deliver_event(c->logical_console,&activated)==LIB_STATUS_OK);
        }
        lib_test_assert(attempts==2 && c->base.stopping && !retired);
        /* The dedicated terminal wake is independent of failed ordinary
         * notification, so destroy waits for real retirement and frees once. */
        lib_test_assert(kvm_console_destroy(c)==LIB_STATUS_OK);
        lib_test_assert(joins==1 && retired==1 && failures==1);
        rejected_wake=LIB_NULL;
        lib_win32_close_handle(asleep);
    }
    {
        kvm_console_options o = { .input_sink = input, .failure_sink = failure };
        kvm_console *c = LIB_NULL;
        asleep = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
        lib_test_assert(asleep != LIB_NULL);
        rejected_wake = LIB_NULL;
        reject_attach = LIB_TRUE;
        attempts = retired = failures = joins = 0u;
        lib_test_assert(kvm_console_create(&c, &o) == LIB_STATUS_IO_ERROR && c == LIB_NULL);
        lib_test_assert(joins == 1u && retired == 1u && failures == 0u);
        reject_attach = LIB_FALSE;
        lib_win32_close_handle(asleep);
    }
    {
        kvm_console_options o = { .input_sink = input, .failure_sink = failure };
        kvm_console *c = LIB_NULL;
        lifetime_fault = 5u;
        asleep = LIB_NULL;
        lib_test_assert(kvm_console_create(&c, &o) == LIB_STATUS_OK);
        lib_test_assert(kvm_console_destroy(c) == LIB_STATUS_IO_ERROR);
        lifetime_fault = 0u;
    }
    {
        kvm_console_options o = { .input_sink = input, .failure_sink = failure };
        kvm_console *c = LIB_NULL;
        lib_test_assert(kvm_console_create(&c, &o) == LIB_STATUS_OK);
        lib_test_assert(kvm_component_request_stop(&c->base) == LIB_STATUS_OK);
        lib_test_assert(wait_for_signal(((kvm_console_win32_state *)c->worker_state)->worker,
            3000u) == LIB_WIN32_WAIT_OBJECT_0);
        lifetime_fault = 1u; /* Failed wake after known exit is safe, not fatal. */
        lib_test_assert(kvm_console_destroy(c) == LIB_STATUS_IO_ERROR);
        lifetime_fault = 0u;
    }
    kvm_frame_exchange_destroy(frames);
    return 0;
}
