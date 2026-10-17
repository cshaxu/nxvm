#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/kvm-console/console.h"
#include "lib/base/input_adapter_interface.h"
#include "lib/kvm-console/win32/component.c"
#include "lib/kvm-base/input_interface.h"
/* Isolate the leaf's message lifetime from the host keyboard layout. */
static lib_bool fixture_scalar(lib_u32 scalar, base_key_input *out)
{
    if (scalar!='a' && scalar!='A') return LIB_FALSE;
    *out=(base_key_input){.key='A',.scan_code=0x1e,
        .modifiers=scalar=='A'?BASE_KEY_MODIFIER_SHIFT:0};
    return LIB_TRUE;
}
#define base_keyboard_map_scalar fixture_scalar
#include "lib/kvm-base/input.c"
#undef base_keyboard_map_scalar

typedef struct capture {
    kvm_input_event events[256];
    lib_u32 count, attempts, reject_at;
} capture;
static lib_bool capture_event(void *opaque, const kvm_input_event *event)
{
    capture *c = opaque;
    if (++c->attempts == c->reject_at) return LIB_FALSE;
    lib_test_assert(c->count < 256);
    if (event->type == KVM_EVENT_KEY) {
        lib_test_assert(event->data.key.key == 'A' || event->data.key.key == 'B');
        lib_test_assert(event->data.key.scan_code == (event->data.key.key == 'A' ? 0x1eu : 0x30u));
        lib_test_assert(event->data.key.flags == 0 && event->data.key.modifiers == 0);
    } else if (event->type == KVM_EVENT_TEXT) {
        lib_test_assert(event->data.text.scalar == 0x1f600 || event->data.text.scalar == 0x4e00);
    } else lib_test_assert(event->type == KVM_EVENT_INPUT_RESET);

    c->events[c->count] = *event;
    c->events[c->count].source = LIB_NULL;
    c->events[c->count++].source_identity = 0;
    return LIB_TRUE;
}
static void key_sequence(const capture *c, const char *keys, lib_u32 pressed)
{
    lib_test_assert(c->count == lib_text_length(keys));
    for (lib_u32 i = 0; i < c->count; ++i) {
        lib_test_assert(c->events[i].type == KVM_EVENT_KEY);
        lib_test_assert(c->events[i].data.key.key == (lib_u32)keys[i]);
        lib_test_assert(c->events[i].data.key.pressed == ((pressed >> i) & 1u));
    }
}

static void no_failure(void *opaque, lib_u64 source, lib_status status)
{ (void)opaque; (void)source; (void)status; }
static lib_status no_join(kvm_component *component, lib_u32 timeout_ms)
{ (void)component; (void)timeout_ms; return LIB_STATUS_OK; }
static void no_dispose(kvm_component *component)
{ kvm_component_mailboxes_destroy(&component->mailboxes); }
static void console_record(kvm_console *console, lib_u32 key, lib_u32 scan,
    lib_u32 scalar, lib_bool down, lib_u32 repeat)
{
    lib_console_event e = { .kind = LIB_CONSOLE_EVENT_RAW_KEY };
    e.value.raw_key=(base_key_input){.key=key,.scan_code=(lib_u16)scan,
        .scalar=key?0:scalar,.pressed=down,.repeat_count=(lib_u16)repeat};
    kvm_console_receive_event(console,&e);
}
static void adapter_lifetime(lib_u32 scan)
{

    static kvm_console console;
    lib_memory_set(&console, 0, sizeof(console));
    kvm_console_win32_state state = {0};
    capture c = {0};
    kvm_component_options options = { .input_sink = capture_event, .failure_sink = no_failure };
    options.input_context = &c;
    lib_test_assert(kvm_component_initialize(&console.base, &options, no_join, no_dispose,
        &console.pending_frame, sizeof(console.pending_frame)) == LIB_STATUS_OK);
    lib_test_assert(kvm_component_mailboxes_select_notify(&console.base.mailboxes,
        LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    console.worker_state = &state;
    /* New source: the previous cooked reader consumed the make. */
    console_record(&console, 'A', scan, 'a', LIB_FALSE, 0);
    lib_test_assert(c.attempts == 0);
    /* Reset revokes the old make; its late break belongs to neither source. */
    console_record(&console, 'A', scan, 'a', LIB_TRUE, 0);
    lib_test_assert(kvm_component_reset_input(&console.base));
    lib_test_assert(c.count == 2);
    lib_test_assert(c.events[1].type == KVM_EVENT_INPUT_RESET);
    console_record(&console, 'A', scan, 'a', LIB_FALSE, 0);
    lib_test_assert(c.attempts == 2);
    c.count = c.attempts = 0;
    console_record(&console, 'A', scan, 'a', 1, 0);
    console_record(&console, 'A', scan, 'a', 0, 0);
    lib_test_assert(c.count == 2);
    key_sequence(&c, "AA", 1u);
    /* Neutral physical keys interleaved with releases emit no extra text.
     * Native scan recovery is verified by Base, before Console receives input. */
    console_record(&console, 'A', 0x1e, 'a', 1, 0);
    console_record(&console, 'B', 0x30, 'b', 1, 0);
    console_record(&console, 'B', 0x30, 'b', 0, 0);
    console_record(&console, 'A', 0x1e, 'a', 0, 0);
    key_sequence(&c, "AAABBA", 13u);
    lib_test_assert(c.count == 6);
    c.count = c.attempts = 0;
    console_record(&console,0,0,0x1f600,1,1);
    lib_test_assert(c.count==1 && c.events[0].type==KVM_EVENT_TEXT);
    /* Console receives complete scalars. Base owns UTF-16 decoding. */
    c.count=c.attempts=0;
    console_record(&console,'A',0x1e,0,1,4);
    console_record(&console,'A',0x1e,0,0,1);
    key_sequence(&c,"AAAAA",15u);
    for(lib_u32 repeat=1;repeat<=3;++repeat) {
        c.count=c.attempts=0;
        console_record(&console,0,0,0x4e00,1,repeat);
        lib_test_assert(c.count==repeat);
        for(lib_u32 i=0;i<c.count;++i)
            lib_test_assert(c.events[i].type==KVM_EVENT_TEXT && c.events[i].data.text.scalar==0x4e00);
    }
    c.count=c.attempts=0;
    console_record(&console,0,0,'a',1,2);
    key_sequence(&c,"AAAA",5u);
    c.count=c.attempts=0;
    c.reject_at=3;
    console_record(&console,0,0,0x1f600,1,4);
    lib_test_assert(console.base.stopping && c.attempts==3 && c.count==2);
    console_record(&console,'B',0x30,0,1,1);
    lib_test_assert(c.attempts==3);
    lib_test_assert(kvm_component_destroy(&console.base)==LIB_STATUS_OK);
}
int main(void)
{
    adapter_lifetime(0x1e);
    return 0;
}
