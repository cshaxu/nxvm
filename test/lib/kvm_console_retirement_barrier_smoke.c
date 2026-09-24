#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/console/binding_interface.h"
#include "lib/kvm-console/console_interface.h"


#ifdef _WIN32
#include "lib/types/win32/test.h"
#include "lib/kvm-console/console.h"

static lib_win32_long fail_wake;
static lib_win32_handle frame_idle;
static base_sync_wait_result retirement_wait(base_sync_event *wake, lib_u32 timeout)
{
    if (frame_idle) lib_test_assert(ReleaseSemaphore(frame_idle, 1, LIB_NULL));
    base_sync_wait_result result = base_sync_event_wait(wake, timeout);
    return lib_win32_interlocked_compare_exchange(&fail_wake, 0, 0) ?
        BASE_SYNC_WAIT_FAULT : result;
}
/* Compile the production worker; only its wait result is controllable. */
#define base_sync_event_wait retirement_wait
#include "lib/kvm-console/win32/component.c"
#undef base_sync_event_wait

typedef struct retirement_probe {
    lib_win32_handle input_entered;
    lib_win32_handle release_input;
    lib_win32_handle input_done;
    lib_win32_handle retired;
    lib_win32_handle destroyed;
    kvm_input_event events[2];
    lib_win32_long event_count;
    lib_win32_long failures;
} retirement_probe;

typedef struct delivery_context {
    lib_console *console;
    lib_win32_handle done;
} delivery_context;

typedef struct destroy_context {
    kvm_console *console;
    lib_win32_handle done;
} destroy_context;

static lib_i32 retirement_input(void *opaque, const kvm_input_event *event)
{
    retirement_probe *probe = (retirement_probe *)opaque;
    lib_win32_long index;
    lib_test_assert(probe != LIB_NULL && event != LIB_NULL);
    index = lib_win32_interlocked_increment(&probe->event_count) - 1;
    lib_test_assert(index >= 0 && index < 2);
    probe->events[index] = *event;
    if (event->type == KVM_EVENT_KEY) {
        lib_win32_set_event(probe->input_entered);
        lib_test_assert(lib_win32_wait_for_single_object(probe->release_input, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
        lib_win32_set_event(probe->input_done);
    } else {
        lib_test_assert(event->type == KVM_EVENT_SOURCE_RETIRED);
        lib_win32_set_event(probe->retired);
    }
    return 1;
}

static void retirement_failure(void *opaque, lib_u64 source_identity,
    lib_status status)
{
    retirement_probe *probe = opaque;
    lib_test_assert(source_identity != 0u && status == LIB_STATUS_IO_ERROR);
    lib_win32_interlocked_increment(&probe->failures);
}

static lib_win32_dword LIB_WIN32_WINAPI retirement_deliver(void *opaque)
{
    delivery_context *context = (delivery_context *)opaque;
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = 1u;
    event.value.raw_key.key = 'A';
    event.value.raw_key.pressed = LIB_TRUE;
    lib_test_assert(lib_console_deliver_event(context->console, &event) == LIB_STATUS_OK);
    lib_win32_set_event(context->done);
    return 0u;
}

static lib_win32_dword LIB_WIN32_WINAPI retirement_destroy(void *opaque)
{
    destroy_context *context = (destroy_context *)opaque;
    lib_test_assert(kvm_console_destroy(context->console) == LIB_STATUS_OK);
    lib_win32_set_event(context->done);
    return 0u;
}

static void check_retirement(lib_i32 fault)
{
    retirement_probe probe = { 0 };
    kvm_console_options options = { 0 };
    kvm_console *console = LIB_NULL;
    delivery_context delivery = { 0 };
    destroy_context destroy = { 0 };
    lib_win32_handle delivery_thread;
    lib_win32_handle destroy_thread;
    lib_console_event late_event = { 0 };

    probe.input_entered = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    probe.release_input = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    probe.input_done = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    probe.retired = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    probe.destroyed = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    lib_test_assert(probe.input_entered && probe.release_input && probe.input_done &&
        probe.retired && probe.destroyed);
    options.input_context = &probe;
    options.input_sink = retirement_input;
    options.failure_context = &probe;
    options.failure_sink = retirement_failure;
    lib_test_assert(kvm_console_create(&console, &options) == LIB_STATUS_OK);
    delivery.console = lib_console_retain(kvm_console_get_console(console));
    lib_test_assert(lib_console_bind_generation(delivery.console, 1u) == LIB_STATUS_OK);
    delivery.done = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    destroy.console = console;
    destroy.done = probe.destroyed;
    lib_test_assert(delivery.done != LIB_NULL);
    delivery_thread = lib_win32_create_thread(LIB_NULL, 0u, retirement_deliver, &delivery, 0u,
        LIB_NULL);
    lib_test_assert(delivery_thread != LIB_NULL);
    lib_test_assert(lib_win32_wait_for_single_object(probe.input_entered, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    if (fault) {
        lib_win32_interlocked_exchange(&fail_wake, 1);
        base_sync_event_signal(console->base.mailboxes.wake);
    }
    destroy_thread = lib_win32_create_thread(LIB_NULL, 0u, retirement_destroy, &destroy, 0u,
        LIB_NULL);
    lib_test_assert(destroy_thread != LIB_NULL);
    /* The component cannot retire or free itself while a copied key callback
       is still active. No timer is involved in this proof. */
    lib_test_assert(lib_win32_wait_for_single_object(probe.destroyed, 0u) == LIB_WIN32_WAIT_TIMEOUT);
    lib_win32_set_event(probe.release_input);
    lib_test_assert(lib_win32_wait_for_single_object(delivery.done, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_wait_for_single_object(probe.retired, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_wait_for_single_object(probe.destroyed, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(probe.event_count == 2);
    lib_test_assert(probe.events[0].type == KVM_EVENT_KEY);
    lib_test_assert(probe.events[1].type == KVM_EVENT_SOURCE_RETIRED);
    lib_test_assert(probe.failures == fault);
    lib_test_assert(lib_win32_wait_for_single_object(delivery_thread, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_wait_for_single_object(destroy_thread, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    late_event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    late_event.binding_generation = 1u;
    late_event.value.raw_key.key = 'B';
    late_event.value.raw_key.pressed = LIB_TRUE;
    lib_test_assert(lib_console_deliver_event(delivery.console, &late_event) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(probe.event_count == 2 && probe.failures == fault);
    lib_console_release(delivery.console);
    lib_win32_close_handle(delivery_thread);
    lib_win32_close_handle(destroy_thread);
    lib_win32_close_handle(delivery.done);
    lib_win32_close_handle(probe.input_entered);
    lib_win32_close_handle(probe.release_input);
    lib_win32_close_handle(probe.input_done);
    lib_win32_close_handle(probe.retired);
    lib_win32_close_handle(probe.destroyed);
}

static lib_win32_handle write_called;
static lib_status write_result;
static lib_status failing_frame(void *context, const lib_console_text_frame *frame)
{
    (void)context; (void)frame;
    lib_win32_set_event(write_called);
    return write_result;
}
static void check_io_failure(lib_i32 reader, lib_status output_status)
{
    retirement_probe probe = { 0 };
    kvm_console_options options = { 0 };
    kvm_console *console;
    static kvm_console_text_frame frame;
    lib_win32_interlocked_exchange(&fail_wake, 0);
    probe.retired = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    write_called = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    options.input_sink = retirement_input; options.input_context = &probe;
    options.failure_sink = retirement_failure; options.failure_context = &probe;
    lib_test_assert(kvm_console_create(&console, &options) == LIB_STATUS_OK);
    lib_console *logical = kvm_console_get_console(console);
    lib_test_assert(lib_console_bind_generation(logical, 1u) == LIB_STATUS_OK);
    if (reader) {
        lib_console_event event = { 0 };
        event.kind = LIB_CONSOLE_EVENT_IO_FAILURE; event.binding_generation = 1u;
        lib_test_assert(lib_console_deliver_event(logical, &event) == LIB_STATUS_OK);
    } else {
        write_result = output_status;
        const lib_console_output_binding output = { LIB_NULL, failing_frame, LIB_NULL };
        lib_test_assert(lib_console_set_output_binding(logical, &output) == LIB_STATUS_OK);
        frame.base.text_columns = 80u; frame.base.text_rows = 25u;
        lib_test_assert(kvm_console_publish_frame(console, &frame) == LIB_STATUS_OK);
        lib_test_assert(lib_win32_wait_for_single_object(write_called, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    }
    if (reader || output_status == LIB_STATUS_IO_ERROR)
        lib_test_assert(lib_win32_wait_for_single_object(probe.retired, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(kvm_console_destroy(console) == LIB_STATUS_OK);
    lib_test_assert(probe.event_count == 1 && probe.events[0].type == KVM_EVENT_SOURCE_RETIRED);
    lib_test_assert(probe.failures == (reader || output_status == LIB_STATUS_IO_ERROR));
    lib_win32_close_handle(probe.retired); lib_win32_close_handle(write_called);
}

static lib_u32 frame_writes;
static char last_frame_text;
static lib_i32 stop_after_publication;
static kvm_console *publishing_console;
static kvm_console_text_frame next_frame;
static lib_status activation_frame(void *opaque, const lib_console_text_frame *frame)
{
    (void)opaque;
    ++frame_writes;
    last_frame_text=(char)frame->text[0];
    if (frame->text[0]=='A') {
        next_frame.base.cells[0].glyph_index='B';
        lib_test_assert(kvm_console_publish_frame(publishing_console,&next_frame)==0);
        if (stop_after_publication) {
            lib_console_event activation={0};
            activation.kind=LIB_CONSOLE_EVENT_ACTIVATED;
            activation.binding_generation=1;
            lib_test_assert(kvm_component_request_stop(&publishing_console->base)==0);
            lib_test_assert(lib_console_deliver_event(kvm_console_get_console(publishing_console),
                &activation)==0);
        }
    }
    return write_result;
}
static void idle_frame(void)
{ lib_test_assert(lib_win32_wait_for_single_object(frame_idle,5000)==LIB_WIN32_WAIT_OBJECT_0); }
static void check_activation_frame(void)
{
    retirement_probe probe={0};
    kvm_console_options options={0};
    lib_console_event activated={0};
    kvm_console *c;
    lib_u32 generation=0;
    static kvm_console_text_frame copied;
    lib_win32_interlocked_exchange(&fail_wake,0);
    frame_idle=lib_win32_create_semaphore_a(LIB_NULL,0,32,LIB_NULL); lib_test_assert(frame_idle);
    probe.retired=lib_win32_create_event_a(LIB_NULL,LIB_WIN32_TRUE,LIB_WIN32_FALSE,LIB_NULL);
    options.input_sink=retirement_input; options.input_context=&probe;
    options.failure_sink=retirement_failure; options.failure_context=&probe;
    lib_test_assert(kvm_console_create(&c,&options)==0); idle_frame();
    lib_console *logical=kvm_console_get_console(c);
    lib_test_assert(lib_console_bind_generation(logical,1)==0);
    const lib_console_output_binding output = { LIB_NULL, activation_frame, LIB_NULL };
    lib_test_assert(lib_console_set_output_binding(logical, &output) == LIB_STATUS_OK);
    activated.kind=LIB_CONSOLE_EVENT_ACTIVATED; activated.binding_generation=1;
    lib_test_assert(lib_console_deliver_event(logical,&activated)==0); idle_frame();
    lib_test_assert(frame_writes==0); /* Empty activation never invents output. */
    next_frame.base.text_columns=80; next_frame.base.text_rows=25;
    for (lib_u32 i=0;i<256;++i) {
        next_frame.characters.primary[i]=(lib_u16)i;
        next_frame.characters.secondary[i]=(lib_u16)i;
    }
    next_frame.base.cells[0].glyph_index='X'; write_result=LIB_STATUS_OK;
    lib_test_assert(kvm_console_publish_frame(c,&next_frame)==0); idle_frame();
    lib_test_assert(frame_writes==1 && !kvm_component_mailboxes_capture_frame(
        &c->base.mailboxes,&generation,&copied,sizeof(copied)));
    {
        static kvm_console_text_frame rejected;
        lib_test_assert(kvm_console_publish_frame(LIB_NULL,&next_frame)==LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(kvm_console_publish_frame(c,LIB_NULL)==LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(kvm_console_publish_frame(c,&rejected)==LIB_STATUS_INVALID_ARGUMENT);
        rejected.base.text_columns=81; rejected.base.text_rows=25;
        lib_test_assert(kvm_console_publish_frame(c,&rejected)==LIB_STATUS_UNSUPPORTED);
        rejected.base.text_columns=80; rejected.base.text_rows=26;
        lib_test_assert(kvm_console_publish_frame(c,&rejected)==LIB_STATUS_UNSUPPORTED);
        rejected.base.text_rows=25; rejected.characters.secondary[255]=0xd800;
        lib_test_assert(kvm_console_publish_frame(c,&rejected)==LIB_STATUS_INVALID_ARGUMENT);
        rejected.characters.secondary[255]=0;
        for (lib_u32 field=0;field<3;++field) {
            lib_u8 *value=field==0 ? &rejected.base.cells[1999].foreground :
                field==1 ? &rejected.base.cells[1999].background : &rejected.base.cells[1999].glyph_bank;
            *value=field==2 ? 2 : 16;
            lib_test_assert(kvm_console_publish_frame(c,&rejected)==LIB_STATUS_INVALID_ARGUMENT);
            *value=0;
        }
        lib_test_assert(!kvm_component_mailboxes_capture_frame(&c->base.mailboxes,
            &generation,&copied,sizeof(copied)));
        lib_test_assert(frame_writes==1 && probe.failures==0);
        lib_test_assert(lib_win32_wait_for_single_object(frame_idle,0)==LIB_WIN32_WAIT_TIMEOUT);
        /* The worker is parked after its previous write; no rejected request
         * may signal the existing auto-reset wake event. */
        lib_test_assert(base_sync_event_wait(c->base.mailboxes.wake,0)==BASE_SYNC_WAIT_TIMED_OUT);
    }
    write_result=LIB_STATUS_OK;
    lib_test_assert(lib_console_deliver_event(logical,&activated)==0); idle_frame();
    lib_test_assert(frame_writes==1 && !kvm_component_mailboxes_capture_frame(
        &c->base.mailboxes,&generation,&copied,sizeof(copied)));
    lib_test_assert(lib_console_deliver_event(logical,&activated)==0); idle_frame();
    lib_test_assert(frame_writes==1);
    publishing_console=c; next_frame.base.cells[0].glyph_index='A';
    lib_test_assert(kvm_console_publish_frame(c,&next_frame)==0);
    idle_frame(); idle_frame();
    lib_test_assert(frame_writes==3 && last_frame_text=='B' && !kvm_component_mailboxes_capture_frame(
        &c->base.mailboxes,&generation,&copied,sizeof(copied)));
    next_frame.base.cells[0].glyph_index='A'; stop_after_publication=1;
    lib_test_assert(kvm_console_publish_frame(c,&next_frame)==0);
    lib_test_assert(lib_win32_wait_for_single_object(probe.retired,5000)==LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(kvm_console_publish_frame(c,&next_frame)==LIB_STATUS_INVALID_STATE);
    lib_test_assert(kvm_console_destroy(c) == LIB_STATUS_OK);
    lib_test_assert(probe.event_count==1 && probe.failures==0 && frame_writes==4);
    lib_win32_close_handle(probe.retired); lib_win32_close_handle(frame_idle); frame_idle=LIB_NULL;
}

static kvm_input_event reset_events[16];
static lib_u32 reset_event_count;
static lib_i32 reset_input(void *opaque, const kvm_input_event *event)
{
    (void)opaque;
    lib_test_assert(reset_event_count < 16);
    reset_events[reset_event_count++] = *event;
    return 1;
}
static void reset_failure(void *opaque, lib_u64 id, lib_status status)
{ (void)opaque; (void)id; (void)status; lib_test_assert(0); }
static void check_input_reset(void)
{
    kvm_console *c;
    kvm_console_options options = { .input_sink=reset_input, .failure_sink=reset_failure };
    lib_console_event e = { .kind=LIB_CONSOLE_EVENT_RAW_KEY, .binding_generation=1 };
    static kvm_console_text_frame frame = { .base = { .text_columns=80, .text_rows=25 } }, copied;
    lib_u32 generation;
    lib_test_assert(kvm_hotkey_registry_register(&options.hotkeys,'P',3,"CAP")==0);
    lib_test_assert(kvm_console_create(&c,&options)==0);
    lib_console *logical=kvm_console_get_console(c);
    lib_test_assert(lib_console_bind_generation(logical,1)==0);
    e.value.raw_key=(lib_console_raw_key){ .key=LIB_WIN32_KEY_CONTROL, .scan_code=0x1d, .pressed=1, .modifiers=1 };
    lib_test_assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==0);
    e.value.raw_key=(lib_console_raw_key){ .unicode=0xd83d, .pressed=1 };
    lib_test_assert(lib_console_deliver_event(logical,&e)==0);
    e.kind=LIB_CONSOLE_EVENT_RAW_MOUSE;
    e.value.raw_mouse=(lib_console_raw_mouse){ .delta_x=1, .delta_y=1 };
    lib_test_assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==1);
    lib_test_assert(kvm_console_publish_frame(c,&frame)==0); /* No output sink: pending. */
    lib_test_assert(lib_console_bind_generation(logical,2)==0);
    e.kind=LIB_CONSOLE_EVENT_INPUT_RESET;
    lib_test_assert(lib_console_deliver_event(logical,&e)==LIB_STATUS_OK);
    lib_test_assert(c->base.hotkey_matcher.held_count==1);
    e.binding_generation=2;
    lib_test_assert(lib_console_deliver_event(logical,&e)==0);
    lib_test_assert(!c->base.hotkey_matcher.held_count && c->base.hotkey_matcher.registry.count==1);
    lib_test_assert(kvm_component_mailboxes_capture_frame(&c->base.mailboxes,&generation,&copied,sizeof(copied)));
    lib_test_assert(copied.base.text_columns==80);
    e.kind=LIB_CONSOLE_EVENT_RAW_KEY;
    e.value.raw_key=(lib_console_raw_key){ .unicode=0xde00, .pressed=1 };
    lib_test_assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==1);
    e.value.raw_key=(lib_console_raw_key){ .key='A', .scan_code=0x1e, .pressed=1 };
    lib_test_assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==2);
    lib_test_assert(reset_events[1].type==KVM_EVENT_KEY && reset_events[1].data.key.key=='A');
    e.kind=LIB_CONSOLE_EVENT_RAW_MOUSE;
    e.value.raw_mouse=(lib_console_raw_mouse){ .delta_x=61, .delta_y=21 };
    lib_test_assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==3);
    lib_test_assert(reset_events[2].data.mouse.delta_x==0 && reset_events[2].data.mouse.delta_y==0);
    ++e.value.raw_mouse.delta_x; ++e.value.raw_mouse.delta_y;
    e.value.raw_mouse.buttons=LIB_WIN32_FROM_LEFT_1ST_BUTTON_PRESSED|LIB_WIN32_RIGHTMOST_BUTTON_PRESSED;
    lib_test_assert(lib_console_deliver_event(logical,&e)==0);
    lib_test_assert(reset_events[3].data.mouse.delta_x==8 && reset_events[3].data.mouse.delta_y==16);
    lib_test_assert(reset_events[2].data.mouse.buttons==0 && reset_events[3].data.mouse.buttons==
        (KVM_MOUSE_BUTTON_LEFT|KVM_MOUSE_BUTTON_RIGHT));
    lib_test_assert(reset_events[3].data.mouse.relative && reset_events[3].source_identity==
        reset_events[2].source_identity);
    lib_test_assert(kvm_console_mouse_buttons(0)==0);
    lib_test_assert(kvm_console_mouse_buttons(LIB_WIN32_FROM_LEFT_1ST_BUTTON_PRESSED)==KVM_MOUSE_BUTTON_LEFT);
    lib_test_assert(kvm_console_mouse_buttons(LIB_WIN32_RIGHTMOST_BUTTON_PRESSED)==KVM_MOUSE_BUTTON_RIGHT);
    lib_test_assert(kvm_console_mouse_buttons(LIB_WIN32_FROM_LEFT_2ND_BUTTON_PRESSED)==0);
    e.kind=LIB_CONSOLE_EVENT_RAW_KEY;
    e.value.raw_key=(lib_console_raw_key){ .key='P', .scan_code=0x19, .pressed=1, .modifiers=3 };
    lib_test_assert(lib_console_deliver_event(logical,&e)==0 && reset_event_count==5);
    lib_test_assert(reset_events[4].type==KVM_EVENT_HOTKEY); /* Accepted snapshot policy survives. */
    lib_test_assert(kvm_console_destroy(c) == LIB_STATUS_OK);
    lib_test_assert(reset_event_count==6 && reset_events[5].type==KVM_EVENT_SOURCE_RETIRED);
}

static void invalid_control_failure(void *opaque, lib_u64 identity, lib_status status)
{
    retirement_probe *probe = opaque;
    lib_test_assert(identity && status == LIB_STATUS_INVALID_ARGUMENT);
    lib_win32_interlocked_increment(&probe->failures);
}

static void check_invalid_control(void)
{
    retirement_probe probe = { 0 };
    kvm_console_options options = { .input_sink=retirement_input,
        .input_context=&probe, .failure_sink=invalid_control_failure,
        .failure_context=&probe };
    kvm_component_control control = { .kind=LIB_UINT32_MAX };
    kvm_console *console;
    probe.retired = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    lib_test_assert(probe.retired && kvm_console_create(&console, &options) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_enqueue_control(&console->base, &control) == LIB_STATUS_OK);
    lib_test_assert(lib_win32_wait_for_single_object(probe.retired, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(kvm_component_enqueue_control(&console->base, &control) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(kvm_console_destroy(console) == LIB_STATUS_OK);
    lib_test_assert(probe.failures == 1 && probe.event_count == 1);
    lib_win32_close_handle(probe.retired);
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
    lib_console_output_binding binding = { LIB_NULL, capture_characters, &captured };
    lib_test_assert(lib_console_create(&console.logical_console) == LIB_STATUS_OK);
    lib_test_assert(lib_console_set_output_binding(console.logical_console, &binding) == LIB_STATUS_OK);
    frame.base.cells[0].glyph_index = frame.base.cells[1].glyph_index = 65;
    frame.base.cells[1].foreground = 8;
    frame.characters.primary[65] = 0x263a;
    frame.characters.secondary[65] = 0x2665;
    lib_test_assert(kvm_console_publish_text_frame(&console, &frame) == LIB_STATUS_OK);
    lib_test_assert(captured.text[0] == 0x263a && captured.text[1] == 0x263a);
    frame.base.cells[1].glyph_bank = 1;
    lib_test_assert(kvm_console_publish_text_frame(&console, &frame) == LIB_STATUS_OK);
    lib_test_assert(captured.text[0] == 0x263a && captured.text[1] == 0x2665);
    frame.characters.secondary[65] = 0x03a9;
    lib_test_assert(kvm_console_publish_text_frame(&console, &frame) == LIB_STATUS_OK);
    lib_test_assert(captured.text[1] == 0x03a9);
    for (lib_u32 enabled=0;enabled<2;++enabled) {
        for (lib_u32 attribute=0;attribute<256;++attribute) {
            frame.base.cells[1].foreground=attribute & 15u;
            frame.base.cells[1].background=attribute >> 4;
            frame.base.cells[1].glyph_bank=enabled && (attribute & 8u);
            lib_test_assert(kvm_console_publish_text_frame(&console,&frame)==LIB_STATUS_OK);
            lib_test_assert(captured.text[1]==(enabled && (attribute & 8u) ? 0x03a9 : 0x263a));
            lib_test_assert(captured.foreground[1]==(attribute & 15u));
            lib_test_assert(captured.background[1]==(attribute >> 4));
        }
    }
    frame.base.cells[1].foreground=1; frame.base.cells[1].glyph_bank=1;
    lib_test_assert(kvm_console_publish_text_frame(&console,&frame)==LIB_STATUS_OK);
    lib_test_assert(captured.text[1]==0x03a9 && captured.foreground[1]==1);
    frame.base.text_rows = 2;
    frame.base.cells[KVM_TEXT_COLUMNS] = (kvm_text_cell){ 66, 0, 3, 4 };
    frame.base.cells[KVM_TEXT_COLUMNS + 1u] = (kvm_text_cell){ 67, 1, 5, 6 };
    frame.characters.primary[66] = 0x2500;
    frame.characters.secondary[67] = 0x2588;
    lib_test_assert(kvm_console_publish_text_frame(&console, &frame) == LIB_STATUS_OK);
    lib_test_assert(captured.text[80] == 0x2500 && captured.foreground[80] == 3 && captured.background[80] == 4);
    lib_test_assert(captured.text[81] == 0x2588 && captured.foreground[81] == 5 && captured.background[81] == 6);
    /* KVM scanlines are normalized before the independent Console boundary. */
    const struct { lib_u32 height, top, bottom, visible, out_bottom; } cases[] = {
        {0,14,15,1,15}, {16,20,21,0,15}, {16,14,31,1,15},
        {16,4,7,1,7}, {8,7,255,1,7}, {16,9,8,1,8}
    };
    for (lib_u32 i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        frame.base.font_height = cases[i].height;
        frame.base.cursor_top = cases[i].top;
        frame.base.cursor_bottom = cases[i].bottom;
        frame.base.cursor_visible = frame.base.cursor_phase = 1;
        lib_test_assert(kvm_console_publish_text_frame(&console, &frame) == LIB_STATUS_OK);
        lib_test_assert(captured.font_height == (cases[i].height ? cases[i].height : 16));
        lib_test_assert(captured.cursor_visible == cases[i].visible);
        lib_test_assert(captured.cursor_top == cases[i].top && captured.cursor_bottom == cases[i].out_bottom);
        lib_test_assert(captured.cursor_phase == 1);
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
