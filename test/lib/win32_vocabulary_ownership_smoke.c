#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
/* Compile the real component bodies against controlled external queries.
 * No KVM interaction, sleeping, or timing-dependent assertion is needed. */
#include "lib/types/win32/clock.h"
#include "lib/types/win32/input.h"
#include "lib/types/types_interface.h"

static lib_i32 counter_ok = 1, frequency_ok = 1;
static lib_win32_longlong counter_value = 123, frequency_value = 1000;
static lib_u32 query_count;
static lib_u32 pressed;
static lib_win32_short layout_result;
static lib_win32_short fake_key_scan(lib_win32_wchar scalar)
{ (void)scalar; return layout_result; }

static lib_win32_bool fake_counter(lib_win32_counter *out)
{ ++query_count; out->QuadPart = counter_value; return counter_ok; }
static lib_win32_bool fake_frequency(lib_win32_counter *out)
{ ++query_count; out->QuadPart = frequency_value; return frequency_ok; }
static lib_win32_key_state fake_key_state(lib_i32 key)
{
    lib_u32 bit = key == LIB_WIN32_KEY_CONTROL ? 1u : key == LIB_WIN32_KEY_ALT ? 2u : key == LIB_WIN32_KEY_SHIFT ? 4u : 0u;
    return (lib_win32_key_state)((pressed & bit) != 0u ? 0x8000u : 0u);
}

#undef lib_win32_query_performance_counter
#undef lib_win32_query_performance_frequency
#undef lib_win32_get_key_state
#define lib_win32_query_performance_counter fake_counter
#define lib_win32_query_performance_frequency fake_frequency
#define lib_win32_get_key_state fake_key_state
#undef lib_win32_key_scan
#define lib_win32_key_scan fake_key_scan
#include "lib/base/win32/clock.c"
#include "lib/kvm-window/win32/input.c"
#include "lib/kvm-base/win32/input.c"
#include "lib/kvm-base/input.c"
#include "lib/kvm-base/hotkey.c"

static kvm_input_event emitted[8];
static lib_u32 emitted_count;
static lib_i32 capture(void *context, const kvm_input_event *event)
{
    (void)context;
    if (emitted_count == 8u) return 0;
    emitted[emitted_count++] = *event;
    return 1;
}

#define CHECK(expression) do { if (!(expression)) return __LINE__; } while (0)
int main(void)
{
    lib_u64 units = 999u, frequency = 888u;
    CHECK(base_clock_platform_counter(LIB_NULL, &frequency) == LIB_STATUS_IO_ERROR);
    CHECK(base_clock_platform_counter(&units, LIB_NULL) == LIB_STATUS_IO_ERROR);
    CHECK(query_count == 0u);
    counter_ok = 0;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_IO_ERROR);
    CHECK(units == 999u && frequency == 888u && query_count == 1u);
    counter_ok = 1; frequency_ok = 0;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_IO_ERROR);
    frequency_ok = 1; frequency_value = 0;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_IO_ERROR);
    frequency_value = -1;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_IO_ERROR);
    frequency_value = 1000; counter_value = -1;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_IO_ERROR);
    CHECK(units == 999u && frequency == 888u);
    counter_value = 123;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_OK);
    CHECK(units == 123u && frequency == 1000u);
    for (pressed = 0u; pressed != 8u; ++pressed) {
        lib_u8 expected = 0u;
        if ((pressed & 1u) != 0u) expected |= KVM_HOTKEY_MODIFIER_CONTROL;
        if ((pressed & 2u) != 0u) expected |= KVM_HOTKEY_MODIFIER_ALT;
        if ((pressed & 4u) != 0u) expected |= KVM_HOTKEY_MODIFIER_SHIFT;
        CHECK(kvm_window_modifiers_from_key_state() == expected);
    }
    /* VkKeyScan uses a DIFFERENT mask from lib_win32_get_key_state/KVM modifiers.
     * Exercise all raw combinations and the actual emitted make/break path. */
    for (lib_u32 raw = 0u; raw != 8u; ++raw) {
        lib_u16 key;
        lib_u8 modifiers, expected = 0u;
        lib_u32 count = 0u;
        kvm_keyboard_normalizer state = { 0 };
        if (raw & 1u) { expected |= KVM_INPUT_MODIFIER_SHIFT; ++count; }
        if (raw & 2u) { expected |= KVM_INPUT_MODIFIER_CONTROL; ++count; }
        if (raw & 4u) { expected |= KVM_INPUT_MODIFIER_ALT; ++count; }
        layout_result = (lib_win32_short)((raw << 8u) | 'A');
        CHECK(kvm_keyboard_platform_map_scalar('a', &key, &modifiers));
        CHECK(key == 'A' && modifiers == expected);
        emitted_count = 0u;
        CHECK(kvm_keyboard_submit_record(&state, LIB_NULL, LIB_NULL, capture,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16='a', .repeat_count=1u }));
        CHECK(emitted_count == count * 2u + 2u);
        CHECK(emitted[count].data.key.key == 'A');
        CHECK(emitted[count].data.key.modifiers == expected);
        for (lib_u32 i = 0u; i != count; ++i) {
            CHECK(emitted[i].data.key.pressed);
            CHECK(!emitted[emitted_count - 1u - i].data.key.pressed);
            CHECK(emitted[i].data.key.key == emitted[emitted_count - 1u - i].data.key.key);
        }
    }
    layout_result = -1;
    { lib_u16 key = 99u; lib_u8 modifiers = 99u;
      CHECK(!kvm_keyboard_platform_map_scalar('a', &key, &modifiers));
      CHECK(key == 99u && modifiers == 99u); }
    /* Layout success is not proof of a representable neutral physical key. */
    {
        kvm_keyboard_normalizer state = {0};
        layout_result = LIB_WIN32_KEY_OEM_102;
        emitted_count = 0;
        CHECK(kvm_keyboard_submit_record(&state, LIB_NULL, LIB_NULL, capture,
        &(kvm_keyboard_record){ .kind=KVM_KEYBOARD_CHARACTER, .utf16='<', .repeat_count=3 }));
        CHECK(emitted_count == 3);
        for (lib_u32 i = 0; i < emitted_count; ++i)
            CHECK(emitted[i].type == KVM_EVENT_TEXT && emitted[i].data.text.scalar == '<');
    }
    return 0;
}
