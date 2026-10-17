#include "lib/types/test.h"
#include "lib/kvm-base/input_interface.h"

static lib_u8 layout_modifiers;
static lib_bool map_scalar(lib_u32 scalar, base_key_input *out)
{
    if (scalar != 'a') return LIB_FALSE;
    *out = (base_key_input){.key='A', .scan_code=0x1e, .modifiers=layout_modifiers};
    return LIB_TRUE;
}
#define base_keyboard_map_scalar map_scalar
#include "lib/kvm-base/input.c"
#undef base_keyboard_map_scalar

static kvm_input_event events[16];
static lib_u32 count;
static lib_bool capture(void *context, const kvm_input_event *event)
{
    (void)context;
    if (count == 16u) return LIB_FALSE;
    events[count++] = *event;
    return LIB_TRUE;
}
static lib_bool match(void *context, const kvm_input_event *event)
{ return kvm_hotkey_matcher_submit(context,event,capture,LIB_NULL,LIB_TRUE); }

int main(void)
{
    base_keyboard_decoder decoder = {0};
    base_keyboard_record record = {.kind=BASE_KEYBOARD_CHARACTER,
        .utf16=0x4e00, .pressed=LIB_TRUE, .repeat_count=1};
    lib_test_assert(!kvm_keyboard_submit_input(LIB_NULL,LIB_NULL,capture,LIB_NULL));
    lib_test_assert(!kvm_keyboard_submit_input(LIB_NULL,LIB_NULL,LIB_NULL,
        &(base_key_input){.key='A'}));
    count=0;
    lib_test_assert(kvm_keyboard_submit_record(&decoder,LIB_NULL,LIB_NULL,capture,
        &record)==BASE_KEYBOARD_ACCEPTED);
    lib_test_assert(count==1 && events[0].data.text.scalar==0x4e00);
    count=16;
    lib_test_assert(kvm_keyboard_submit_record(&decoder,LIB_NULL,LIB_NULL,capture,
        &record)==BASE_KEYBOARD_REJECTED);
    record=(base_keyboard_record){.kind=BASE_KEYBOARD_TRANSITION,
        .key=0xffff, .pressed=LIB_TRUE};
    count=0;
    lib_test_assert(kvm_keyboard_submit_record(&decoder,LIB_NULL,LIB_NULL,capture,
        &record)==BASE_KEYBOARD_UNMAPPED);
    lib_test_assert(count==0);
    const struct {base_key key; lib_u16 scan; lib_bool extended;} physical[] = {
        {'A',0x1e,0}, {BASE_KEY_ESCAPE,1,0}, {BASE_KEY_ENTER,0x1c,0},
        {BASE_KEY_RIGHT,0x4d,1}, {BASE_KEY_UP,0,0}
    };
    for (lib_size i=0;i<sizeof(physical)/sizeof(physical[0]);++i) {
        base_key_input input={.key=physical[i].key,.scan_code=physical[i].scan,
            .extended=physical[i].extended,.pressed=LIB_TRUE,.repeat_count=1,
            .modifiers=BASE_KEY_MODIFIER_CONTROL|BASE_KEY_MODIFIER_ALT};
        count=0;
        lib_test_assert(kvm_keyboard_submit_input(LIB_NULL,LIB_NULL,capture,&input));
        input.pressed=LIB_FALSE;
        lib_test_assert(kvm_keyboard_submit_input(LIB_NULL,LIB_NULL,capture,&input));
        lib_test_assert(count==2);
        for(lib_u32 j=0;j<count;++j) {
            lib_test_assert(events[j].type==KVM_EVENT_KEY);
            lib_test_assert(events[j].data.key.key==input.key && events[j].data.key.scan_code==input.scan_code);
            lib_test_assert(events[j].data.key.pressed==(j==0));
            lib_test_assert(events[j].data.key.modifiers==input.modifiers);
            lib_test_assert(events[j].data.key.flags==(input.extended?BASE_KEY_FLAG_EXTENDED:0));
        }
    }
    for(layout_modifiers=0;layout_modifiers<8;++layout_modifiers) {
        lib_u32 modifiers=0;
        for(lib_u32 bit=0;bit<3;++bit) if(layout_modifiers&(1u<<bit)) ++modifiers;
        count=0;
        lib_test_assert(kvm_keyboard_submit_input(LIB_NULL,LIB_NULL,capture,
            &(base_key_input){.scalar='a',.repeat_count=1}));
        lib_test_assert(count==2*modifiers+2);
        lib_test_assert(events[modifiers].data.key.key=='A' &&
            events[modifiers].data.key.modifiers==layout_modifiers);
        for(lib_u32 i=0;i<modifiers;++i) {
            lib_test_assert(events[i].data.key.pressed && !events[count-1-i].data.key.pressed);
            lib_test_assert(events[i].data.key.key==events[count-1-i].data.key.key);
        }
    }
    const lib_u32 scalars[]={0x1f600,0x4e00,'<'};
    for(lib_size i=0;i<sizeof(scalars)/sizeof(scalars[0]);++i) {
        count=0;
        lib_test_assert(kvm_keyboard_submit_input(LIB_NULL,LIB_NULL,capture,
            &(base_key_input){.scalar=scalars[i],.repeat_count=3}));
        lib_test_assert(count==3);
        for(lib_u32 j=0;j<count;++j)
            lib_test_assert(events[j].type==KVM_EVENT_TEXT && events[j].data.text.scalar==scalars[i]);
    }
    count=16;
    lib_test_assert(!kvm_keyboard_submit_input(LIB_NULL,LIB_NULL,capture,
        &(base_key_input){.scalar=0x4e00}));
    const base_key triggers[]={'P','D','F'};
    for(lib_size i=0;i<sizeof(triggers)/sizeof(triggers[0]);++i) {
        kvm_hotkey_registry registry;
        kvm_hotkey_matcher matcher;
        kvm_hotkey_registry_initialize(&registry);
        lib_test_assert(kvm_hotkey_registry_register(&registry,triggers[i],
            BASE_KEY_MODIFIER_CONTROL|BASE_KEY_MODIFIER_ALT,"action")==LIB_STATUS_OK);
        kvm_hotkey_matcher_initialize(&matcher,&registry);
        const base_key keys[]={BASE_KEY_CONTROL,BASE_KEY_ALT,triggers[i]};
        count=0;
        for(lib_u32 j=0;j<3;++j)
            lib_test_assert(kvm_keyboard_submit_input(LIB_NULL,&matcher,match,
                &(base_key_input){.key=keys[j],.pressed=LIB_TRUE,
                    .modifiers=j==0?BASE_KEY_MODIFIER_CONTROL:BASE_KEY_MODIFIER_CONTROL|BASE_KEY_MODIFIER_ALT}));
        for(lib_u32 j=3;j>0;--j)
            lib_test_assert(kvm_keyboard_submit_input(LIB_NULL,&matcher,match,
                &(base_key_input){.key=keys[j-1],.pressed=LIB_FALSE}));
        lib_test_assert(count==1 && events[0].type==KVM_EVENT_HOTKEY);
        lib_test_assert(lib_text_compare((const char *)events[0].data.hotkey.identifier,"action")==0);
        kvm_hotkey_matcher_discard(&matcher);
    }
    return 0;
}
