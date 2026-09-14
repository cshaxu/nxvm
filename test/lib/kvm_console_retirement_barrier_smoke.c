#include "lib/console/binding_interface.h"
#include "lib/kvm-console/console_interface.h"

#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#include "lib/kvm-console/console.h"

static LONG fail_wake;
static HANDLE frame_idle;
static lib_status retirement_wait(
    const kvm_mailbox_wake *wake, lib_u32 timeout, kvm_mailbox_wake_wait_result *out_result)
{
    if (frame_idle) assert(ReleaseSemaphore(frame_idle, 1, NULL));
    lib_status result = kvm_mailbox_wake_wait(wake, timeout, out_result);
    return InterlockedCompareExchange(&fail_wake, 0, 0) ?
        LIB_STATUS_IO_ERROR : result;
}
/* Compile the production worker; only its wait result is controllable. */
#define kvm_mailbox_wake_wait retirement_wait
#include "lib/kvm-console/win32/component.c"
#undef kvm_mailbox_wake_wait

typedef struct retirement_probe {
    HANDLE input_entered;
    HANDLE release_input;
    HANDLE input_done;
    HANDLE retired;
    HANDLE destroyed;
    kvm_input_event events[2];
    LONG event_count;
    LONG failures;
} retirement_probe;

typedef struct delivery_context {
    lib_console *console;
    HANDLE done;
} delivery_context;

typedef struct destroy_context {
    kvm_console *console;
    HANDLE done;
} destroy_context;

static int retirement_input(void *opaque, const kvm_input_event *event)
{
    retirement_probe *probe = (retirement_probe *)opaque;
    LONG index;
    assert(probe != NULL && event != NULL);
    index = InterlockedIncrement(&probe->event_count) - 1;
    assert(index >= 0 && index < 2);
    probe->events[index] = *event;
    if (event->type == KVM_EVENT_KEY) {
        SetEvent(probe->input_entered);
        assert(WaitForSingleObject(probe->release_input, INFINITE) == WAIT_OBJECT_0);
        SetEvent(probe->input_done);
    } else {
        assert(event->type == KVM_EVENT_SOURCE_RETIRED);
        SetEvent(probe->retired);
    }
    return 1;
}

static void retirement_failure(void *opaque, lib_u64 source_identity,
    lib_status status)
{
    retirement_probe *probe = opaque;
    assert(source_identity != 0u && status == LIB_STATUS_IO_ERROR);
    InterlockedIncrement(&probe->failures);
}

static DWORD WINAPI retirement_deliver(void *opaque)
{
    delivery_context *context = (delivery_context *)opaque;
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = 1u;
    event.value.raw_key.key = 'A';
    event.value.raw_key.pressed = LIB_TRUE;
    assert(lib_console_deliver_event(context->console, &event) == LIB_STATUS_OK);
    SetEvent(context->done);
    return 0u;
}

static DWORD WINAPI retirement_destroy(void *opaque)
{
    destroy_context *context = (destroy_context *)opaque;
    assert(kvm_console_destroy(context->console) == LIB_STATUS_OK);
    SetEvent(context->done);
    return 0u;
}

static void check_retirement(int fault)
{
    retirement_probe probe = { 0 };
    kvm_console_options options = { 0 };
    kvm_console *console = NULL;
    delivery_context delivery = { 0 };
    destroy_context destroy = { 0 };
    HANDLE delivery_thread;
    HANDLE destroy_thread;
    lib_console_event late_event = { 0 };

    probe.input_entered = CreateEventA(NULL, TRUE, FALSE, NULL);
    probe.release_input = CreateEventA(NULL, TRUE, FALSE, NULL);
    probe.input_done = CreateEventA(NULL, TRUE, FALSE, NULL);
    probe.retired = CreateEventA(NULL, TRUE, FALSE, NULL);
    probe.destroyed = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(probe.input_entered && probe.release_input && probe.input_done &&
        probe.retired && probe.destroyed);
    options.input_context = &probe;
    options.input_sink = retirement_input;
    options.failure_context = &probe;
    options.failure_sink = retirement_failure;
    assert(kvm_console_create(&console, &options) == LIB_STATUS_OK);
    delivery.console = lib_console_retain(kvm_console_get_console(console));
    assert(lib_console_bind_generation(delivery.console, 1u) == LIB_STATUS_OK);
    delivery.done = CreateEventA(NULL, TRUE, FALSE, NULL);
    destroy.console = console;
    destroy.done = probe.destroyed;
    assert(delivery.done != NULL);
    delivery_thread = CreateThread(NULL, 0u, retirement_deliver, &delivery, 0u,
        NULL);
    assert(delivery_thread != NULL);
    assert(WaitForSingleObject(probe.input_entered, INFINITE) == WAIT_OBJECT_0);
    if (fault) {
        InterlockedExchange(&fail_wake, 1);
        kvm_mailbox_wake_signal(kvm_component_mailboxes_wake(&console->base.mailboxes));
    }
    destroy_thread = CreateThread(NULL, 0u, retirement_destroy, &destroy, 0u,
        NULL);
    assert(destroy_thread != NULL);
    /* The component cannot retire or free itself while a copied key callback
       is still active. No timer is involved in this proof. */
    assert(WaitForSingleObject(probe.destroyed, 0u) == WAIT_TIMEOUT);
    SetEvent(probe.release_input);
    assert(WaitForSingleObject(delivery.done, INFINITE) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(probe.retired, INFINITE) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(probe.destroyed, INFINITE) == WAIT_OBJECT_0);
    assert(probe.event_count == 2);
    assert(probe.events[0].type == KVM_EVENT_KEY);
    assert(probe.events[1].type == KVM_EVENT_SOURCE_RETIRED);
    assert(probe.failures == fault);
    assert(WaitForSingleObject(delivery_thread, INFINITE) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(destroy_thread, INFINITE) == WAIT_OBJECT_0);
    late_event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    late_event.binding_generation = 1u;
    late_event.value.raw_key.key = 'B';
    late_event.value.raw_key.pressed = LIB_TRUE;
    assert(lib_console_deliver_event(delivery.console, &late_event) == LIB_STATUS_INVALID_STATE);
    assert(probe.event_count == 2 && probe.failures == fault);
    lib_console_release(delivery.console);
    CloseHandle(delivery_thread);
    CloseHandle(destroy_thread);
    CloseHandle(delivery.done);
    CloseHandle(probe.input_entered);
    CloseHandle(probe.release_input);
    CloseHandle(probe.input_done);
    CloseHandle(probe.retired);
    CloseHandle(probe.destroyed);
}

static HANDLE write_called;
static lib_status write_result;
static lib_status failing_frame(void *context, const lib_console_text_frame *frame)
{
    (void)context; (void)frame;
    SetEvent(write_called);
    return write_result;
}
static void check_io_failure(int reader, lib_status output_status)
{
    retirement_probe probe = { 0 };
    kvm_console_options options = { 0 };
    kvm_console *console;
    static kvm_frame frame;
    InterlockedExchange(&fail_wake, 0);
    probe.retired = CreateEventA(NULL, TRUE, FALSE, NULL);
    write_called = CreateEventA(NULL, TRUE, FALSE, NULL);
    options.input_sink = retirement_input; options.input_context = &probe;
    options.failure_sink = retirement_failure; options.failure_context = &probe;
    assert(kvm_console_create(&console, &options) == LIB_STATUS_OK);
    lib_console *logical = kvm_console_get_console(console);
    assert(lib_console_bind_generation(logical, 1u) == LIB_STATUS_OK);
    if (reader) {
        lib_console_event event = { 0 };
        event.kind = LIB_CONSOLE_EVENT_IO_FAILURE; event.binding_generation = 1u;
        assert(lib_console_deliver_event(logical, &event) == LIB_STATUS_OK);
    } else {
        write_result = output_status;
        assert(lib_console_set_text_frame_sink(logical, failing_frame, NULL) == LIB_STATUS_OK);
        frame.valid = 1u; frame.text_columns = 80u; frame.text_rows = 25u;
        assert(kvm_console_publish_frame(console, &frame) == LIB_STATUS_OK);
        assert(WaitForSingleObject(write_called, 5000u) == WAIT_OBJECT_0);
    }
    if (reader || output_status == LIB_STATUS_IO_ERROR)
        assert(WaitForSingleObject(probe.retired, 5000u) == WAIT_OBJECT_0);
    assert(kvm_console_destroy(console) == LIB_STATUS_OK);
    assert(probe.event_count == 1 && probe.events[0].type == KVM_EVENT_SOURCE_RETIRED);
    assert(probe.failures == (reader || output_status == LIB_STATUS_IO_ERROR));
    CloseHandle(probe.retired); CloseHandle(write_called);
}

static unsigned frame_writes;
static char last_frame_text;
static int stop_after_publication;
static kvm_console *publishing_console;
static kvm_frame next_frame;
static lib_status activation_frame(void *opaque, const lib_console_text_frame *frame)
{
    (void)opaque;
    ++frame_writes;
    last_frame_text=(char)frame->text[0];
    if (frame->text[0]=='A') {
        next_frame.text[0]='B';
        assert(kvm_console_publish_frame(publishing_console,&next_frame)==0);
        if (stop_after_publication) {
            lib_console_event activation={0};
            activation.kind=LIB_CONSOLE_EVENT_ACTIVATED;
            activation.binding_generation=1;
            assert(kvm_component_request_stop(&publishing_console->base)==0);
            assert(lib_console_deliver_event(kvm_console_get_console(publishing_console),
                &activation)==0);
        }
    }
    return write_result;
}
static void idle_frame(void)
{ assert(WaitForSingleObject(frame_idle,5000)==WAIT_OBJECT_0); }
static void check_activation_frame(void)
{
    retirement_probe probe={0};
    kvm_console_options options={0};
    lib_console_event activated={0};
    kvm_console *c;
    lib_u32 generation=0;
    static kvm_frame copied;
    InterlockedExchange(&fail_wake,0);
    frame_idle=CreateSemaphoreA(NULL,0,32,NULL); assert(frame_idle);
    probe.retired=CreateEventA(NULL,TRUE,FALSE,NULL);
    options.input_sink=retirement_input; options.input_context=&probe;
    options.failure_sink=retirement_failure; options.failure_context=&probe;
    assert(kvm_console_create(&c,&options)==0); idle_frame();
    lib_console *logical=kvm_console_get_console(c);
    assert(lib_console_bind_generation(logical,1)==0);
    assert(lib_console_set_text_frame_sink(logical,activation_frame,NULL)==0);
    activated.kind=LIB_CONSOLE_EVENT_ACTIVATED; activated.binding_generation=1;
    assert(lib_console_deliver_event(logical,&activated)==0); idle_frame();
    assert(frame_writes==0); /* Empty activation never invents output. */
    next_frame.valid=1; next_frame.text_columns=80; next_frame.text_rows=25;
    next_frame.text[0]='X'; write_result=LIB_STATUS_NOT_CURRENT;
    assert(kvm_console_publish_frame(c,&next_frame)==0); idle_frame();
    assert(frame_writes==1 && kvm_component_mailboxes_capture_frame(
        &c->base.mailboxes,&generation,&copied));
    write_result=LIB_STATUS_OK;
    assert(lib_console_deliver_event(logical,&activated)==0); idle_frame();
    assert(frame_writes==2 && !kvm_component_mailboxes_capture_frame(
        &c->base.mailboxes,&generation,&copied));
    assert(lib_console_deliver_event(logical,&activated)==0); idle_frame();
    assert(frame_writes==2);
    publishing_console=c; next_frame.text[0]='A';
    assert(kvm_console_publish_frame(c,&next_frame)==0);
    idle_frame(); idle_frame();
    assert(frame_writes==4 && last_frame_text=='B' && !kvm_component_mailboxes_capture_frame(
        &c->base.mailboxes,&generation,&copied));
    assert(copied.text[0]=='X'); /* An empty capture does not change output. */
    next_frame.text[0]='A'; stop_after_publication=1;
    assert(kvm_console_publish_frame(c,&next_frame)==0);
    assert(WaitForSingleObject(probe.retired,5000)==WAIT_OBJECT_0);
    assert(kvm_console_destroy(c) == LIB_STATUS_OK);
    assert(probe.event_count==1 && probe.failures==0 && frame_writes==5);
    CloseHandle(probe.retired); CloseHandle(frame_idle); frame_idle=NULL;
}

static kvm_input_event reset_events[16];
static unsigned reset_event_count;
static int reset_input(void *opaque, const kvm_input_event *event)
{
    (void)opaque;
    assert(reset_event_count < 16);
    reset_events[reset_event_count++] = *event;
    return 1;
}
static void reset_failure(void *opaque, lib_u64 id, lib_status status)
{ (void)opaque; (void)id; (void)status; assert(0); }
static void check_input_reset(void)
{
    kvm_console *c;
    kvm_console_options options = { .input_sink=reset_input, .failure_sink=reset_failure };
    lib_console_event e = { .kind=LIB_CONSOLE_EVENT_RAW_KEY, .binding_generation=1 };
    static kvm_frame frame = { .valid=1, .text_columns=80, .text_rows=25 }, copied;
    lib_u32 generation;
    assert(kvm_hotkey_registry_register(&options.hotkeys,'P',3,"CAP")==0);
    assert(kvm_console_create(&c,&options)==0);
    lib_console *logical=kvm_console_get_console(c);
    assert(lib_console_bind_generation(logical,1)==0);
    e.value.raw_key=(lib_console_raw_key){ .key=VK_CONTROL, .scan_code=0x1d, .pressed=1, .modifiers=1 };
    assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==0);
    e.value.raw_key=(lib_console_raw_key){ .unicode=0xd83d, .pressed=1 };
    assert(lib_console_deliver_event(logical,&e)==0);
    e.kind=LIB_CONSOLE_EVENT_RAW_MOUSE;
    e.value.raw_mouse=(lib_console_raw_mouse){ .delta_x=1, .delta_y=1 };
    assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==1);
    assert(kvm_console_publish_frame(c,&frame)==0); /* No output sink: pending. */
    assert(lib_console_bind_generation(logical,2)==0);
    e.kind=LIB_CONSOLE_EVENT_INPUT_RESET;
    assert(lib_console_deliver_event(logical,&e)==LIB_STATUS_NOT_CURRENT);
    assert(c->base.hotkey_matcher.held_count==1);
    e.binding_generation=2;
    assert(lib_console_deliver_event(logical,&e)==0);
    assert(!c->base.hotkey_matcher.held_count && c->base.hotkey_matcher.registry.count==1);
    assert(kvm_component_mailboxes_capture_frame(&c->base.mailboxes,&generation,&copied));
    assert(copied.text_columns==80);
    e.kind=LIB_CONSOLE_EVENT_RAW_KEY;
    e.value.raw_key=(lib_console_raw_key){ .unicode=0xde00, .pressed=1 };
    assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==1);
    e.value.raw_key=(lib_console_raw_key){ .key='A', .scan_code=0x1e, .pressed=1 };
    assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==2);
    assert(reset_events[1].type==KVM_EVENT_KEY && reset_events[1].data.key.key=='A');
    e.kind=LIB_CONSOLE_EVENT_RAW_MOUSE;
    e.value.raw_mouse=(lib_console_raw_mouse){ .delta_x=61, .delta_y=21 };
    assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==3);
    assert(reset_events[2].data.mouse.delta_x==0 && reset_events[2].data.mouse.delta_y==0);
    ++e.value.raw_mouse.delta_x; ++e.value.raw_mouse.delta_y;
    assert(lib_console_deliver_event(logical,&e)==0);
    assert(reset_events[3].data.mouse.delta_x==8 && reset_events[3].data.mouse.delta_y==16);
    e.kind=LIB_CONSOLE_EVENT_RAW_KEY;
    e.value.raw_key=(lib_console_raw_key){ .key='P', .scan_code=0x19, .pressed=1, .modifiers=3 };
    assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==5);
    assert(reset_events[4].type==KVM_EVENT_HOTKEY); /* Accepted snapshot policy survives. */
    assert(kvm_console_destroy(c) == LIB_STATUS_OK);
    assert(reset_event_count==6 && reset_events[5].type==KVM_EVENT_SOURCE_RETIRED);
}

int main(void)
{
    check_retirement(0);
    check_retirement(1);
    check_io_failure(1, LIB_STATUS_OK);
    check_io_failure(0, LIB_STATUS_IO_ERROR);
    check_io_failure(0, LIB_STATUS_NOT_CURRENT);
    check_activation_frame();
    check_input_reset();
    return 0;
}
#else
int main(void) { return 0; }
#endif
