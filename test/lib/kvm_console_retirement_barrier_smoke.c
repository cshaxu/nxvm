#include "lib/console/binding_interface.h"
#include "lib/kvm-console/console_interface.h"

#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#include "lib/kvm-console/console.h"

static LONG fail_wake;
static HANDLE frame_idle;
static base_sync_wait_result retirement_wait(base_sync_event *wake, lib_u32 timeout)
{
    if (frame_idle) assert(ReleaseSemaphore(frame_idle, 1, NULL));
    base_sync_wait_result result = base_sync_event_wait(wake, timeout);
    return InterlockedCompareExchange(&fail_wake, 0, 0) ?
        BASE_SYNC_WAIT_FAULT : result;
}
/* Compile the production worker; only its wait result is controllable. */
#define base_sync_event_wait retirement_wait
#include "lib/kvm-console/win32/component.c"
#undef base_sync_event_wait

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
        base_sync_event_signal(console->base.mailboxes.wake);
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
    static kvm_console_text_frame frame;
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
        const lib_console_output_binding output = { NULL, failing_frame, NULL };
        assert(lib_console_set_output_binding(logical, &output) == LIB_STATUS_OK);
        frame.base.text_columns = 80u; frame.base.text_rows = 25u;
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
static kvm_console_text_frame next_frame;
static lib_status activation_frame(void *opaque, const lib_console_text_frame *frame)
{
    (void)opaque;
    ++frame_writes;
    last_frame_text=(char)frame->text[0];
    if (frame->text[0]=='A') {
        next_frame.base.cells[0].glyph_index='B';
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
    static kvm_console_text_frame copied;
    InterlockedExchange(&fail_wake,0);
    frame_idle=CreateSemaphoreA(NULL,0,32,NULL); assert(frame_idle);
    probe.retired=CreateEventA(NULL,TRUE,FALSE,NULL);
    options.input_sink=retirement_input; options.input_context=&probe;
    options.failure_sink=retirement_failure; options.failure_context=&probe;
    assert(kvm_console_create(&c,&options)==0); idle_frame();
    lib_console *logical=kvm_console_get_console(c);
    assert(lib_console_bind_generation(logical,1)==0);
    const lib_console_output_binding output = { NULL, activation_frame, NULL };
    assert(lib_console_set_output_binding(logical, &output) == LIB_STATUS_OK);
    activated.kind=LIB_CONSOLE_EVENT_ACTIVATED; activated.binding_generation=1;
    assert(lib_console_deliver_event(logical,&activated)==0); idle_frame();
    assert(frame_writes==0); /* Empty activation never invents output. */
    next_frame.base.text_columns=80; next_frame.base.text_rows=25;
    for (lib_u32 i=0;i<256;++i) {
        next_frame.characters.primary[i]=(lib_u16)i;
        next_frame.characters.secondary[i]=(lib_u16)i;
    }
    next_frame.base.cells[0].glyph_index='X'; write_result=LIB_STATUS_OK;
    assert(kvm_console_publish_frame(c,&next_frame)==0); idle_frame();
    assert(frame_writes==1 && !kvm_component_mailboxes_capture_frame(
        &c->base.mailboxes,&generation,&copied,sizeof(copied)));
    {
        static kvm_console_text_frame rejected;
        assert(kvm_console_publish_frame(NULL,&next_frame)==LIB_STATUS_INVALID_ARGUMENT);
        assert(kvm_console_publish_frame(c,NULL)==LIB_STATUS_INVALID_ARGUMENT);
        assert(kvm_console_publish_frame(c,&rejected)==LIB_STATUS_INVALID_ARGUMENT);
        rejected.base.text_columns=81; rejected.base.text_rows=25;
        assert(kvm_console_publish_frame(c,&rejected)==LIB_STATUS_UNSUPPORTED);
        rejected.base.text_columns=80; rejected.base.text_rows=26;
        assert(kvm_console_publish_frame(c,&rejected)==LIB_STATUS_UNSUPPORTED);
        rejected.base.text_rows=25; rejected.characters.secondary[255]=0xd800;
        assert(kvm_console_publish_frame(c,&rejected)==LIB_STATUS_INVALID_ARGUMENT);
        rejected.characters.secondary[255]=0;
        for (unsigned field=0;field<3;++field) {
            lib_u8 *value=field==0 ? &rejected.base.cells[1999].foreground :
                field==1 ? &rejected.base.cells[1999].background : &rejected.base.cells[1999].glyph_bank;
            *value=field==2 ? 2 : 16;
            assert(kvm_console_publish_frame(c,&rejected)==LIB_STATUS_INVALID_ARGUMENT);
            *value=0;
        }
        assert(!kvm_component_mailboxes_capture_frame(&c->base.mailboxes,
            &generation,&copied,sizeof(copied)));
        assert(frame_writes==1 && probe.failures==0);
        assert(WaitForSingleObject(frame_idle,0)==WAIT_TIMEOUT);
        /* The worker is parked after its previous write; no rejected request
         * may signal the existing auto-reset wake event. */
        assert(base_sync_event_wait(c->base.mailboxes.wake,0)==BASE_SYNC_WAIT_TIMED_OUT);
    }
    write_result=LIB_STATUS_OK;
    assert(lib_console_deliver_event(logical,&activated)==0); idle_frame();
    assert(frame_writes==1 && !kvm_component_mailboxes_capture_frame(
        &c->base.mailboxes,&generation,&copied,sizeof(copied)));
    assert(lib_console_deliver_event(logical,&activated)==0); idle_frame();
    assert(frame_writes==1);
    publishing_console=c; next_frame.base.cells[0].glyph_index='A';
    assert(kvm_console_publish_frame(c,&next_frame)==0);
    idle_frame(); idle_frame();
    assert(frame_writes==3 && last_frame_text=='B' && !kvm_component_mailboxes_capture_frame(
        &c->base.mailboxes,&generation,&copied,sizeof(copied)));
    next_frame.base.cells[0].glyph_index='A'; stop_after_publication=1;
    assert(kvm_console_publish_frame(c,&next_frame)==0);
    assert(WaitForSingleObject(probe.retired,5000)==WAIT_OBJECT_0);
    assert(kvm_console_publish_frame(c,&next_frame)==LIB_STATUS_INVALID_STATE);
    assert(kvm_console_destroy(c) == LIB_STATUS_OK);
    assert(probe.event_count==1 && probe.failures==0 && frame_writes==4);
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
    static kvm_console_text_frame frame = { .base = { .text_columns=80, .text_rows=25 } }, copied;
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
    assert(lib_console_deliver_event(logical,&e)==LIB_STATUS_OK);
    assert(c->base.hotkey_matcher.held_count==1);
    e.binding_generation=2;
    assert(lib_console_deliver_event(logical,&e)==0);
    assert(!c->base.hotkey_matcher.held_count && c->base.hotkey_matcher.registry.count==1);
    assert(kvm_component_mailboxes_capture_frame(&c->base.mailboxes,&generation,&copied,sizeof(copied)));
    assert(copied.base.text_columns==80);
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
    e.value.raw_mouse.buttons=FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED;
    assert(lib_console_deliver_event(logical,&e)==0);
    assert(reset_events[3].data.mouse.delta_x==8 && reset_events[3].data.mouse.delta_y==16);
    assert(reset_events[2].data.mouse.buttons==0 && reset_events[3].data.mouse.buttons==
        (KVM_MOUSE_BUTTON_LEFT|KVM_MOUSE_BUTTON_RIGHT));
    assert(reset_events[3].data.mouse.relative && reset_events[3].source_identity==
        reset_events[2].source_identity);
    assert(kvm_console_mouse_buttons(0)==0);
    assert(kvm_console_mouse_buttons(FROM_LEFT_1ST_BUTTON_PRESSED)==KVM_MOUSE_BUTTON_LEFT);
    assert(kvm_console_mouse_buttons(RIGHTMOST_BUTTON_PRESSED)==KVM_MOUSE_BUTTON_RIGHT);
    assert(kvm_console_mouse_buttons(FROM_LEFT_2ND_BUTTON_PRESSED)==0);
    e.kind=LIB_CONSOLE_EVENT_RAW_KEY;
    e.value.raw_key=(lib_console_raw_key){ .key='P', .scan_code=0x19, .pressed=1, .modifiers=3 };
    assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==5);
    assert(reset_events[4].type==KVM_EVENT_HOTKEY); /* Accepted snapshot policy survives. */
    assert(kvm_console_destroy(c) == LIB_STATUS_OK);
    assert(reset_event_count==6 && reset_events[5].type==KVM_EVENT_SOURCE_RETIRED);
}

static void invalid_control_failure(void *opaque, lib_u64 identity, lib_status status)
{
    retirement_probe *probe = opaque;
    assert(identity && status == LIB_STATUS_INVALID_ARGUMENT);
    InterlockedIncrement(&probe->failures);
}

static void check_invalid_control(void)
{
    retirement_probe probe = { 0 };
    kvm_console_options options = { .input_sink=retirement_input,
        .input_context=&probe, .failure_sink=invalid_control_failure,
        .failure_context=&probe };
    kvm_component_control control = { .kind=LIB_UINT32_MAX };
    kvm_console *console;
    probe.retired = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(probe.retired && kvm_console_create(&console, &options) == LIB_STATUS_OK);
    assert(kvm_component_enqueue_control(&console->base, &control) == LIB_STATUS_OK);
    assert(WaitForSingleObject(probe.retired, 5000u) == WAIT_OBJECT_0);
    assert(kvm_component_enqueue_control(&console->base, &control) == LIB_STATUS_INVALID_STATE);
    assert(kvm_console_destroy(console) == LIB_STATUS_OK);
    assert(probe.failures == 1 && probe.event_count == 1);
    CloseHandle(probe.retired);
}

static lib_status capture_characters(void *opaque, const lib_console_text_frame *frame)
{
    lib_console_text_frame *captured = opaque;
    *captured = *frame;
    return LIB_STATUS_OK;
}

static void check_character_banks(void)
{
    kvm_console console = { 0 };
    kvm_console_text_frame frame = { .base = { .text_columns=2, .text_rows=1 } };
    lib_console_text_frame captured = { 0 };
    lib_console_output_binding binding = { NULL, capture_characters, &captured };
    assert(lib_console_create(&console.logical_console) == LIB_STATUS_OK);
    assert(lib_console_set_output_binding(console.logical_console, &binding) == LIB_STATUS_OK);
    frame.base.cells[0].glyph_index = frame.base.cells[1].glyph_index = 65;
    frame.base.cells[1].foreground = 8;
    frame.characters.primary[65] = 0x263a;
    frame.characters.secondary[65] = 0x2665;
    assert(kvm_console_publish_text_frame(&console, &frame) == LIB_STATUS_OK);
    assert(captured.text[0] == 0x263a && captured.text[1] == 0x263a);
    frame.base.cells[1].glyph_bank = 1;
    assert(kvm_console_publish_text_frame(&console, &frame) == LIB_STATUS_OK);
    assert(captured.text[0] == 0x263a && captured.text[1] == 0x2665);
    frame.characters.secondary[65] = 0x03a9;
    assert(kvm_console_publish_text_frame(&console, &frame) == LIB_STATUS_OK);
    assert(captured.text[1] == 0x03a9);
    for (unsigned enabled=0;enabled<2;++enabled) {
        for (unsigned attribute=0;attribute<256;++attribute) {
            frame.base.cells[1].foreground=attribute & 15u;
            frame.base.cells[1].background=attribute >> 4;
            frame.base.cells[1].glyph_bank=enabled && (attribute & 8u);
            assert(kvm_console_publish_text_frame(&console,&frame)==LIB_STATUS_OK);
            assert(captured.text[1]==(enabled && (attribute & 8u) ? 0x03a9 : 0x263a));
            assert(captured.foreground[1]==(attribute & 15u));
            assert(captured.background[1]==(attribute >> 4));
        }
    }
    frame.base.cells[1].foreground=1; frame.base.cells[1].glyph_bank=1;
    assert(kvm_console_publish_text_frame(&console,&frame)==LIB_STATUS_OK);
    assert(captured.text[1]==0x03a9 && captured.foreground[1]==1);
    frame.base.text_rows = 2;
    frame.base.cells[KVM_TEXT_COLUMNS] = (kvm_text_cell){ 66, 0, 3, 4 };
    frame.base.cells[KVM_TEXT_COLUMNS + 1u] = (kvm_text_cell){ 67, 1, 5, 6 };
    frame.characters.primary[66] = 0x2500;
    frame.characters.secondary[67] = 0x2588;
    assert(kvm_console_publish_text_frame(&console, &frame) == LIB_STATUS_OK);
    assert(captured.text[80] == 0x2500 && captured.foreground[80] == 3 && captured.background[80] == 4);
    assert(captured.text[81] == 0x2588 && captured.foreground[81] == 5 && captured.background[81] == 6);
    /* KVM scanlines are normalized before the independent Console boundary. */
    const struct { unsigned height, top, bottom, visible, out_bottom; } cases[] = {
        {0,14,15,1,15}, {16,20,21,0,15}, {16,14,31,1,15},
        {16,4,7,1,7}, {8,7,255,1,7}, {16,9,8,1,8}
    };
    for (unsigned i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        frame.base.font_height = cases[i].height;
        frame.base.cursor_top = cases[i].top;
        frame.base.cursor_bottom = cases[i].bottom;
        frame.base.cursor_visible = frame.base.cursor_phase = 1;
        assert(kvm_console_publish_text_frame(&console, &frame) == LIB_STATUS_OK);
        assert(captured.font_height == (cases[i].height ? cases[i].height : 16));
        assert(captured.cursor_visible == cases[i].visible);
        assert(captured.cursor_top == cases[i].top && captured.cursor_bottom == cases[i].out_bottom);
        assert(captured.cursor_phase == 1);
    }
    lib_console_release(console.logical_console);
}

int main(void)
{
    check_retirement(0);
    check_retirement(1);
    check_io_failure(1, LIB_STATUS_OK);
    check_io_failure(0, LIB_STATUS_IO_ERROR);
    check_io_failure(0, LIB_STATUS_OK);
    check_activation_frame();
    check_input_reset();
    check_invalid_control();
    check_character_banks();
    return 0;
}
#else
int main(void) { return 0; }
#endif
