#include "lib/kvm-console/console.h"
#include "lib/console/binding_interface.h"
#include "lib/types/win32/sync.h"
#include <assert.h>

static HANDLE asleep;
static unsigned attempts, retired, failures, joins;
static BOOL WINAPI reject_signal(HANDLE h) { (void)h; ++attempts; return FALSE; }
#undef lib_win32_set_event
#define lib_win32_set_event reject_signal
#include "lib/base/win32/sync.c"
static base_sync_wait_result observed_wait(base_sync_event *w, lib_u32 timeout)
{ SetEvent(asleep); return base_sync_event_wait(w, timeout); }
static DWORD WINAPI bounded_join(HANDLE h,DWORD timeout)
{
    assert(timeout==KVM_COMPONENT_DESTROY_TIMEOUT_MS);
    ++joins;
    /* Model deadline expiry without sleeping five seconds per failure case. */
    return WaitForSingleObject(h,0);
}
#undef lib_win32_wait_for_single_object
#define lib_win32_wait_for_single_object bounded_join
#define base_sync_event_wait observed_wait
#include "lib/kvm-console/win32/component.c"
#undef base_sync_event_wait

static int input(void *p,const kvm_input_event *e)
{ (void)p; assert(e->type==KVM_EVENT_SOURCE_RETIRED); ++retired; return 1; }
static void failure(void *p,lib_u64 id,lib_status s)
{ (void)p; assert(id && s==LIB_STATUS_IO_ERROR); ++failures; }
int main(void)
{
    assert(kvm_console_destroy(NULL)==LIB_STATUS_OK);
    assert(base_sync_event_signal(NULL)==LIB_STATUS_INVALID_ARGUMENT);
    for(unsigned mode=0;mode<4;++mode) {
        kvm_console_options o={0}; kvm_console *c=NULL;
        asleep=CreateEventA(NULL,TRUE,FALSE,NULL);
        attempts=retired=failures=joins=0;
        o.input_sink=input; o.failure_sink=failure;
        assert(kvm_console_create(&c,&o)==LIB_STATUS_OK);
        assert(WaitForSingleObject(asleep,3000)==WAIT_OBJECT_0);
        void *state=c->worker_state;
        kvm_console_win32_state *worker=state;
        if(mode==0) assert(kvm_component_request_stop(&c->base)==LIB_STATUS_OK);
        if(mode==1) {
            kvm_component_control title={.kind=KVM_COMPONENT_CONTROL_SET_WINDOW_TITLE};
            assert(kvm_component_enqueue_control(&c->base,&title)==LIB_STATUS_OK);
        }
        if(mode==2) {
            static kvm_frame frame={.valid=1,.text_columns=80,.text_rows=25};
            assert(kvm_console_publish_frame(c,&frame)==LIB_STATUS_OK);
        }
        if(mode==3) {
            lib_console_event activated={.kind=LIB_CONSOLE_EVENT_ACTIVATED,.binding_generation=1};
            assert(lib_console_bind_generation(c->logical_console,1)==LIB_STATUS_OK);
            assert(lib_console_deliver_event(c->logical_console,&activated)==LIB_STATUS_OK);
        }
        assert(attempts==2 && c->base.stopping && !retired);
        assert(kvm_console_destroy(c)==LIB_STATUS_IO_ERROR);
        assert(joins==1 && c->worker_state==state && !retired && failures==1);
        assert(WaitForSingleObject(worker->worker,0)==WAIT_TIMEOUT);
        /* Test-only rescue after proving no premature retirement/free. */
        assert(SetEvent(c->base.mailboxes.wake->handle));
        assert(WaitForSingleObject(worker->worker,3000)==WAIT_OBJECT_0);
        assert(retired==1 && failures==1);
        assert(kvm_console_destroy(c)==LIB_STATUS_OK && joins==2);
        CloseHandle(asleep);
    }
    return 0;
}
