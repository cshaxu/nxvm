#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/kvm-console/console.h"
#include "lib/console/binding_interface.h"
#include "lib/types/win32/sync.h"

static lib_win32_handle asleep;
static lib_u32 attempts, retired, failures, joins;
static lib_win32_bool LIB_WIN32_WINAPI reject_signal(lib_win32_handle h) { (void)h; ++attempts; return LIB_WIN32_FALSE; }
static lib_win32_bool (LIB_WIN32_WINAPI *signal_asleep)(lib_win32_handle) = lib_win32_set_event;
static lib_win32_dword (LIB_WIN32_WINAPI *wait_for_signal)(lib_win32_handle,lib_win32_dword) = lib_win32_wait_for_single_object;
#undef lib_win32_set_event
#define lib_win32_set_event reject_signal
#include "lib/base/win32/sync.c"
static base_sync_wait_result observed_wait(base_sync_event *w, lib_u32 timeout)
{ signal_asleep(asleep); return base_sync_event_wait(w, timeout); }
static lib_win32_dword LIB_WIN32_WINAPI bounded_join(lib_win32_handle h,lib_win32_dword timeout)
{
    lib_test_assert(timeout==KVM_COMPONENT_DESTROY_TIMEOUT_MS);
    ++joins;
    /* Model deadline expiry without sleeping five seconds per failure case. */
    return wait_for_signal(h,0);
}
#undef lib_win32_wait_for_single_object
#define lib_win32_wait_for_single_object bounded_join
#define base_sync_event_wait observed_wait
#include "lib/kvm-console/win32/component.c"
#undef base_sync_event_wait

static lib_bool input(void *p,const kvm_input_event *e)
{ (void)p; lib_test_assert(e->type==KVM_EVENT_SOURCE_RETIRED); ++retired; return LIB_TRUE; }
static void failure(void *p,lib_u64 id,lib_status s)
{ (void)p; lib_test_assert(id && s==LIB_STATUS_IO_ERROR); ++failures; }
int main(void)
{
    lib_test_assert(kvm_console_destroy(LIB_NULL)==LIB_STATUS_OK);
    lib_test_assert(base_sync_event_signal(LIB_NULL)==LIB_STATUS_INVALID_ARGUMENT);
    for(lib_u32 mode=0;mode<4;++mode) {
        kvm_console_options o={0}; kvm_console *c=LIB_NULL;
        asleep=lib_win32_create_event_a(LIB_NULL,LIB_WIN32_TRUE,LIB_WIN32_FALSE,LIB_NULL);
        attempts=retired=failures=joins=0;
        o.input_sink=input; o.failure_sink=failure;
        lib_test_assert(kvm_console_create(&c,&o)==LIB_STATUS_OK);
        lib_test_assert(wait_for_signal(asleep,3000)==LIB_WIN32_WAIT_OBJECT_0);
        void *state=c->worker_state;
        kvm_console_win32_state *worker=state;
        if(mode==0) lib_test_assert(kvm_component_request_stop(&c->base)==LIB_STATUS_OK);
        if(mode==1) {
            kvm_component_control title={.kind=42u};
            lib_test_assert(kvm_component_enqueue_control(&c->base,&title)==LIB_STATUS_OK);
        }
        if(mode==2) {
            static kvm_console_text_frame frame={.base = { .text_columns=80, .text_rows=25 }};
            lib_test_assert(kvm_console_publish_frame(c,&frame)==LIB_STATUS_OK);
        }
        if(mode==3) {
            lib_console_event activated={.kind=LIB_CONSOLE_EVENT_ACTIVATED,.binding_generation=1};
            lib_test_assert(lib_console_bind_generation(c->logical_console,1)==LIB_STATUS_OK);
            lib_test_assert(lib_console_deliver_event(c->logical_console,&activated)==LIB_STATUS_OK);
        }
        lib_test_assert(attempts==2 && c->base.stopping && !retired);
        lib_test_assert(kvm_console_destroy(c)==LIB_STATUS_IO_ERROR);
        lib_test_assert(joins==1 && c->worker_state==state && !retired && failures==1);
        lib_test_assert(wait_for_signal(worker->worker,0)==LIB_WIN32_WAIT_TIMEOUT);
        /* Test-only rescue after proving no premature retirement/free. */
        lib_test_assert(signal_asleep(c->base.mailboxes.wake->handle));
        lib_test_assert(wait_for_signal(worker->worker,3000)==LIB_WIN32_WAIT_OBJECT_0);
        lib_test_assert(retired==1 && failures==1);
        lib_test_assert(kvm_console_destroy(c)==LIB_STATUS_OK && joins==2);
        lib_win32_close_handle(asleep);
    }
    return 0;
}
