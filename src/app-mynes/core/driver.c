#include "core/driver.h"

#include "core/machine.h"
#include "core/snapshot_interface.h"
#include "lib/base/clock_interface.h"
#include "lib/base/sync_interface.h"

#define CORE_DRIVER_CPU_HZ 1789773u
#define CORE_DRIVER_PACING_WAIT_MAXIMUM_MS 5u
#define CORE_DRIVER_PACING_LAG_REBASE_MS 100u

static void core_driver_clear_staged_audio(core_driver *driver)
{
    if (driver == LIB_NULL) return;
    driver->audio_staging_read = 0u;
    driver->audio_staging_write = 0u;
    driver->audio_staging_count = 0u;
}

static lib_status core_driver_snapshot_write(void *opaque, const lib_u8 *bytes,
    lib_size byte_count)
{
    const common_machine_state_writer *writer = opaque;
    return writer == LIB_NULL || writer->write == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        writer->write(writer->context, bytes, byte_count);
}

static lib_status core_driver_snapshot_read(void *opaque, lib_u8 *bytes,
    lib_size byte_count)
{
    const common_machine_state_reader *reader = opaque;
    return reader == LIB_NULL || reader->read == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        reader->read(reader->context, bytes, byte_count);
}

static void core_driver_capture_audio(core_driver *driver)
{
    lib_u16 capacity;
    lib_u16 captured;

    if (driver == LIB_NULL || driver->machine == LIB_NULL) return;
    while (driver->audio_staging_count < CORE_DRIVER_AUDIO_STAGING_CAPACITY) {
        capacity = (lib_u16)(CORE_DRIVER_AUDIO_STAGING_CAPACITY -
            driver->audio_staging_count);
        if (capacity > CORE_DRIVER_AUDIO_STAGING_CAPACITY - driver->audio_staging_write)
            capacity = (lib_u16)(CORE_DRIVER_AUDIO_STAGING_CAPACITY -
                driver->audio_staging_write);
        if (capacity > CORE_APU_SAMPLE_CAPACITY) capacity = CORE_APU_SAMPLE_CAPACITY;
        captured = core_apu_take_samples(&driver->machine->apu,
            &driver->audio_staging[driver->audio_staging_write], capacity);
        if (captured == 0u) return;
        driver->audio_staging_write = (lib_u16)((driver->audio_staging_write + captured) %
            CORE_DRIVER_AUDIO_STAGING_CAPACITY);
        driver->audio_staging_count = (lib_u16)(driver->audio_staging_count + captured);
        if (captured < capacity) return;
    }
}

static void core_driver_reset_pacing(core_driver *driver)
{
    driver->pacing_ready = base_clock_milliseconds(&driver->pacing_origin_ms) == LIB_STATUS_OK;
    driver->pacing_origin_cycles = driver->machine == LIB_NULL ? 0u : driver->machine->cycles;
}

static void core_driver_pace(core_driver *driver)
{
    lib_u64 now;
    lib_u64 elapsed;
    lib_u64 due;
    if (!driver->pacing_ready || driver->machine == LIB_NULL) return;
    /* A Common wake requests prompt callback service; it must not grant the
     * guest extra elapsed time.  The bounded wait below already services it
     * within five milliseconds. */
    (void)lib_atomic_i32_exchange_explicit(&driver->wake_requested, 0,
        LIB_MEMORY_ORDER_SEQ_CST);
    elapsed = (driver->machine->cycles - driver->pacing_origin_cycles) * 1000u /
        CORE_DRIVER_CPU_HZ;
    due = driver->pacing_origin_ms + elapsed;
    for (;;) {
        lib_u64 wait;
        if (base_clock_milliseconds(&now) != LIB_STATUS_OK) return;
        if (now >= due) {
            if (now - due > CORE_DRIVER_PACING_LAG_REBASE_MS) {
                driver->pacing_origin_ms = now;
                driver->pacing_origin_cycles = driver->machine->cycles;
            }
            return;
        }
        if (lib_atomic_i32_load_explicit(&driver->stop_requested,
                LIB_MEMORY_ORDER_SEQ_CST) != 0) return;
        wait = due - now;
        base_sync_sleep_milliseconds((lib_u32)(wait > CORE_DRIVER_PACING_WAIT_MAXIMUM_MS ?
            CORE_DRIVER_PACING_WAIT_MAXIMUM_MS : wait));
    }
}

static void core_driver_drain_audio(core_driver *driver)
{
    lib_u32 queued;
    lib_u32 writable;
    lib_i16 pcm[LIB_AUDIO_STREAM_MAX_FRAMES_PER_SUBMISSION];
    lib_u32 accepted;
    lib_u32 index;

    if (driver->audio == LIB_NULL || driver->machine == LIB_NULL) return;
    core_driver_capture_audio(driver);
    if (driver->audio_staging_count < CORE_DRIVER_AUDIO_PLAY_BLOCK)
        return;
    while (driver->audio_staging_count >= CORE_DRIVER_AUDIO_PLAY_BLOCK) {
        if (lib_audio_stream_query(driver->audio, &queued, &writable) != LIB_STATUS_OK ||
            writable < CORE_DRIVER_AUDIO_PLAY_BLOCK) return;
        for (index = 0u; index < CORE_DRIVER_AUDIO_PLAY_BLOCK; ++index) {
            lib_i32 mixed = driver->audio_staging[(driver->audio_staging_read + index) %
                CORE_DRIVER_AUDIO_STAGING_CAPACITY];
            if (mixed > 32767) pcm[index] = 32767;
            else if (mixed < -32768) pcm[index] = -32768;
            else pcm[index] = (lib_i16)mixed;
        }
        if (lib_audio_stream_enqueue(driver->audio, pcm, CORE_DRIVER_AUDIO_PLAY_BLOCK,
                &accepted) != LIB_STATUS_OK || accepted > CORE_DRIVER_AUDIO_PLAY_BLOCK)
            return;
        driver->audio_staging_read = (lib_u16)((driver->audio_staging_read + accepted) %
            CORE_DRIVER_AUDIO_STAGING_CAPACITY);
        driver->audio_staging_count = (lib_u16)(driver->audio_staging_count - accepted);
        if (accepted != CORE_DRIVER_AUDIO_PLAY_BLOCK) return;
    }
}

#define CORE_DRIVER_FACTOR(count) ((count) == 0u ? 16u : (count) == 1u ? 12u : 9u)
#define CORE_DRIVER_RED(rgb, mask) ((((rgb) >> 16u) * CORE_DRIVER_FACTOR((((mask) >> 1u) & 1u) + (((mask) >> 2u) & 1u)) + 8u) / 16u)
#define CORE_DRIVER_GREEN(rgb, mask) (((((rgb) >> 8u) & 0xffu) * CORE_DRIVER_FACTOR(((mask) & 1u) + (((mask) >> 2u) & 1u)) + 8u) / 16u)
#define CORE_DRIVER_BLUE(rgb, mask) ((((rgb) & 0xffu) * CORE_DRIVER_FACTOR(((mask) & 1u) + (((mask) >> 1u) & 1u)) + 8u) / 16u)
#define CORE_DRIVER_EMPHASIZED(rgb, mask) ((CORE_DRIVER_RED((rgb), (mask)) << 16u) | (CORE_DRIVER_GREEN((rgb), (mask)) << 8u) | CORE_DRIVER_BLUE((rgb), (mask)))
#define CORE_DRIVER_COLOR_SET(rgb) CORE_DRIVER_EMPHASIZED((rgb), 0u), \
    CORE_DRIVER_EMPHASIZED((rgb), 1u), CORE_DRIVER_EMPHASIZED((rgb), 2u), \
    CORE_DRIVER_EMPHASIZED((rgb), 3u), CORE_DRIVER_EMPHASIZED((rgb), 4u), \
    CORE_DRIVER_EMPHASIZED((rgb), 5u), CORE_DRIVER_EMPHASIZED((rgb), 6u), \
    CORE_DRIVER_EMPHASIZED((rgb), 7u)

/* Nestopia's established NTSC base palette.  A 2C02 produces composite video,
 * not fixed RGB, so no unique palette exists; this replaces the former muted
 * project approximation with a widely used, saturated host presentation. */
static const lib_u32 core_driver_rgb_table[512] = {
    CORE_DRIVER_COLOR_SET(0x666666u),CORE_DRIVER_COLOR_SET(0x002a88u),CORE_DRIVER_COLOR_SET(0x1412a7u),CORE_DRIVER_COLOR_SET(0x3b00a4u),CORE_DRIVER_COLOR_SET(0x5c007eu),CORE_DRIVER_COLOR_SET(0x6e0040u),CORE_DRIVER_COLOR_SET(0x6c0600u),CORE_DRIVER_COLOR_SET(0x561d00u),
    CORE_DRIVER_COLOR_SET(0x333500u),CORE_DRIVER_COLOR_SET(0x0b4800u),CORE_DRIVER_COLOR_SET(0x005200u),CORE_DRIVER_COLOR_SET(0x004f08u),CORE_DRIVER_COLOR_SET(0x00404du),CORE_DRIVER_COLOR_SET(0u),CORE_DRIVER_COLOR_SET(0u),CORE_DRIVER_COLOR_SET(0u),
    CORE_DRIVER_COLOR_SET(0xadadadu),CORE_DRIVER_COLOR_SET(0x155fd9u),CORE_DRIVER_COLOR_SET(0x4240ffu),CORE_DRIVER_COLOR_SET(0x7527feu),CORE_DRIVER_COLOR_SET(0xa01accu),CORE_DRIVER_COLOR_SET(0xb71e7bu),CORE_DRIVER_COLOR_SET(0xb53120u),CORE_DRIVER_COLOR_SET(0x994e00u),
    CORE_DRIVER_COLOR_SET(0x6b6d00u),CORE_DRIVER_COLOR_SET(0x388700u),CORE_DRIVER_COLOR_SET(0x0c9300u),CORE_DRIVER_COLOR_SET(0x008f32u),CORE_DRIVER_COLOR_SET(0x007c8du),CORE_DRIVER_COLOR_SET(0u),CORE_DRIVER_COLOR_SET(0u),CORE_DRIVER_COLOR_SET(0u),
    CORE_DRIVER_COLOR_SET(0xfffeffu),CORE_DRIVER_COLOR_SET(0x64b0ffu),CORE_DRIVER_COLOR_SET(0x9290ffu),CORE_DRIVER_COLOR_SET(0xc676ffu),CORE_DRIVER_COLOR_SET(0xf36affu),CORE_DRIVER_COLOR_SET(0xfe6eccu),CORE_DRIVER_COLOR_SET(0xfe8170u),CORE_DRIVER_COLOR_SET(0xea9e22u),
    CORE_DRIVER_COLOR_SET(0xbcbe00u),CORE_DRIVER_COLOR_SET(0x88d800u),CORE_DRIVER_COLOR_SET(0x5ce430u),CORE_DRIVER_COLOR_SET(0x45e082u),CORE_DRIVER_COLOR_SET(0x48cddeu),CORE_DRIVER_COLOR_SET(0x4f4f4fu),CORE_DRIVER_COLOR_SET(0u),CORE_DRIVER_COLOR_SET(0u),
    CORE_DRIVER_COLOR_SET(0xfffeffu),CORE_DRIVER_COLOR_SET(0xc0dfffu),CORE_DRIVER_COLOR_SET(0xd3d2ffu),CORE_DRIVER_COLOR_SET(0xe8c8ffu),CORE_DRIVER_COLOR_SET(0xfbc2ffu),CORE_DRIVER_COLOR_SET(0xfec4eau),CORE_DRIVER_COLOR_SET(0xfeccc5u),CORE_DRIVER_COLOR_SET(0xf7d8a5u),
    CORE_DRIVER_COLOR_SET(0xe4e594u),CORE_DRIVER_COLOR_SET(0xcfef96u),CORE_DRIVER_COLOR_SET(0xbdf4abu),CORE_DRIVER_COLOR_SET(0xb3f3ccu),CORE_DRIVER_COLOR_SET(0xb5ebf2u),CORE_DRIVER_COLOR_SET(0xb8b8b8u),CORE_DRIVER_COLOR_SET(0u),CORE_DRIVER_COLOR_SET(0u)
};

static lib_u32 core_driver_rgb(lib_u16 sample)
{
    return core_driver_rgb_table[((sample >> 6u) & 7u) + ((sample & 0x3fu) << 3u)];
}

static lib_u8 core_driver_nearest_rgbi(lib_u32 rgb)
{
    static const lib_u32 colors[16] = {
        0x000000u,0x0000aau,0x00aa00u,0x00aaaau,0xaa0000u,0xaa00aau,0xaa5500u,0xaaaaaau,
        0x555555u,0x5555ffu,0x55ff55u,0x55ffffu,0xff5555u,0xff55ffu,0xffff55u,0xffffffu
    };
    lib_u32 best = 0u;
    lib_u32 best_distance = LIB_UINT32_MAX;
    lib_u32 index;
    for (index = 0u; index < 16u; ++index) {
        lib_i32 red = (lib_i32)(rgb >> 16u) - (lib_i32)(colors[index] >> 16u);
        lib_i32 green = (lib_i32)((rgb >> 8u) & 0xffu) -
            (lib_i32)((colors[index] >> 8u) & 0xffu);
        lib_i32 blue = (lib_i32)(rgb & 0xffu) - (lib_i32)(colors[index] & 0xffu);
        lib_u32 distance = (lib_u32)(red * red + green * green + blue * blue);
        if (distance < best_distance) { best_distance = distance; best = index; }
    }
    return (lib_u8)best;
}
enum {
    CORE_DRIVER_BINDING_A_K,
    CORE_DRIVER_BINDING_A_ALT_LEFT,
    CORE_DRIVER_BINDING_A_ALT_RIGHT,
    CORE_DRIVER_BINDING_A_ALT_VIRTUAL,
    CORE_DRIVER_BINDING_B_J,
    CORE_DRIVER_BINDING_B_CONTROL_LEFT,
    CORE_DRIVER_BINDING_B_CONTROL_RIGHT,
    CORE_DRIVER_BINDING_B_CONTROL_VIRTUAL,
    CORE_DRIVER_BINDING_SELECT_SHIFT_LEFT,
    CORE_DRIVER_BINDING_SELECT_SHIFT_RIGHT,
    CORE_DRIVER_BINDING_SELECT_SHIFT_VIRTUAL,
    CORE_DRIVER_BINDING_START_ENTER,
    CORE_DRIVER_BINDING_UP_W,
    CORE_DRIVER_BINDING_DOWN_S,
    CORE_DRIVER_BINDING_LEFT_A,
    CORE_DRIVER_BINDING_RIGHT_D
};

static lib_u8 core_driver_modifier_binding(const kvm_input_event *event,
    lib_u16 scan_code, lib_u8 left, lib_u8 right, lib_u8 virtual)
{
    if (event->data.key.scan_code != scan_code) return virtual;
    return (event->data.key.flags & KVM_KEY_FLAG_EXTENDED) != 0u ? right : left;
}

static lib_u8 core_driver_binding_for_key(const kvm_input_event *event)
{
    /* KVM deliberately normalizes both physical sides of a modifier to one
     * neutral key.  A scan code distinguishes simultaneous physical sides
     * when present; virtual keyboards and RDP may omit it and use the third,
     * scan-less binding without changing the controller mapping. */
    if (event->data.key.key == 'K') return CORE_DRIVER_BINDING_A_K;
    if (event->data.key.key == KVM_KEY_ALT)
        return core_driver_modifier_binding(event, 0x38u,
            CORE_DRIVER_BINDING_A_ALT_LEFT, CORE_DRIVER_BINDING_A_ALT_RIGHT,
            CORE_DRIVER_BINDING_A_ALT_VIRTUAL);
    if (event->data.key.key == 'J') return CORE_DRIVER_BINDING_B_J;
    if (event->data.key.key == KVM_KEY_CONTROL)
        return core_driver_modifier_binding(event, 0x1du,
            CORE_DRIVER_BINDING_B_CONTROL_LEFT, CORE_DRIVER_BINDING_B_CONTROL_RIGHT,
            CORE_DRIVER_BINDING_B_CONTROL_VIRTUAL);
    if (event->data.key.key == KVM_KEY_SHIFT) {
        if (event->data.key.scan_code == 0x2au) return CORE_DRIVER_BINDING_SELECT_SHIFT_LEFT;
        if (event->data.key.scan_code == 0x36u) return CORE_DRIVER_BINDING_SELECT_SHIFT_RIGHT;
        return CORE_DRIVER_BINDING_SELECT_SHIFT_VIRTUAL;
    }
    if (event->data.key.key == KVM_KEY_ENTER) return CORE_DRIVER_BINDING_START_ENTER;
    if (event->data.key.key == 'W') return CORE_DRIVER_BINDING_UP_W;
    if (event->data.key.key == 'S') return CORE_DRIVER_BINDING_DOWN_S;
    if (event->data.key.key == 'A') return CORE_DRIVER_BINDING_LEFT_A;
    if (event->data.key.key == 'D') return CORE_DRIVER_BINDING_RIGHT_D;
    return 0xffu;
}

static lib_u8 core_driver_button_for_binding(lib_u8 binding)
{
    if (binding <= CORE_DRIVER_BINDING_A_ALT_VIRTUAL) return 0u;
    if (binding <= CORE_DRIVER_BINDING_B_CONTROL_VIRTUAL) return 1u;
    if (binding <= CORE_DRIVER_BINDING_SELECT_SHIFT_VIRTUAL) return 2u;
    if (binding == CORE_DRIVER_BINDING_START_ENTER) return 3u;
    if (binding == CORE_DRIVER_BINDING_UP_W) return 4u;
    if (binding == CORE_DRIVER_BINDING_DOWN_S) return 5u;
    if (binding == CORE_DRIVER_BINDING_LEFT_A) return 6u;
    if (binding == CORE_DRIVER_BINDING_RIGHT_D) return 7u;
    return 0xffu;
}

static lib_u8 core_driver_button_for_text(lib_u32 scalar)
{
    if (scalar == '\r' || scalar == '\n') return 3u;
    if (scalar == 'w' || scalar == 'W') return 4u;
    if (scalar == 's' || scalar == 'S') return 5u;
    if (scalar == 'a' || scalar == 'A') return 6u;
    if (scalar == 'd' || scalar == 'D') return 7u;
    if (scalar == 'j' || scalar == 'J') return 1u;
    if (scalar == 'k' || scalar == 'K') return 0u;
    return 0xffu;
}

static void core_driver_refresh_source_buttons(core_driver *driver, lib_u32 source)
{
    lib_u16 bindings = driver->input_source_bindings[source];
    lib_u8 buttons = 0u;

    if ((bindings & 0x000fu) != 0u) buttons |= 0x01u;
    if ((bindings & 0x00f0u) != 0u) buttons |= 0x02u;
    if ((bindings & 0x0700u) != 0u) buttons |= 0x04u;
    if ((bindings & 0x0800u) != 0u) buttons |= 0x08u;
    if ((bindings & 0x1000u) != 0u) buttons |= 0x10u;
    if ((bindings & 0x2000u) != 0u) buttons |= 0x20u;
    if ((bindings & 0x4000u) != 0u) buttons |= 0x40u;
    if ((bindings & 0x8000u) != 0u) buttons |= 0x80u;
    driver->input_source_buttons[source] = buttons;
}

static lib_u8 core_driver_combined_buttons(const core_driver *driver)
{
    lib_u32 index;
    lib_u8 buttons = 0u;
    for (index = 0u; index < CORE_DRIVER_INPUT_SOURCE_CAPACITY; ++index)
        if (driver->input_source_active[index]) buttons |= driver->input_source_buttons[index];
    if ((buttons & 0x30u) == 0x30u) buttons &= (lib_u8)~0x30u;
    if ((buttons & 0xc0u) == 0xc0u) buttons &= (lib_u8)~0xc0u;
    return buttons;
}

static void core_driver_publish_buttons(core_driver *driver)
{
    core_controller_set_buttons(&driver->machine->controller,
        core_driver_combined_buttons(driver));
}

static void core_driver_clear_inputs(core_driver *driver)
{
    lib_u32 index;
    for (index = 0u; index < CORE_DRIVER_INPUT_SOURCE_CAPACITY; ++index) {
        driver->input_source_identities[index] = 0u;
        driver->input_source_buttons[index] = 0u;
        driver->input_source_bindings[index] = 0u;
        driver->input_source_active[index] = LIB_FALSE;
    }
    if (driver->machine != LIB_NULL) {
        core_controller_clear_transient_buttons(&driver->machine->controller);
        core_driver_publish_buttons(driver);
    }
}

static lib_u32 core_driver_find_source(const core_driver *driver, lib_u64 identity)
{
    lib_u32 index;
    for (index = 0u; index < CORE_DRIVER_INPUT_SOURCE_CAPACITY; ++index)
        if (driver->input_source_active[index] &&
            driver->input_source_identities[index] == identity) return index;
    return CORE_DRIVER_INPUT_SOURCE_CAPACITY;
}

static lib_u32 core_driver_admit_source(core_driver *driver, lib_u64 identity)
{
    lib_u32 index = core_driver_find_source(driver, identity);
    if (index != CORE_DRIVER_INPUT_SOURCE_CAPACITY) return index;
    for (index = 0u; index < CORE_DRIVER_INPUT_SOURCE_CAPACITY; ++index)
        if (!driver->input_source_active[index]) {
            driver->input_source_identities[index] = identity;
            driver->input_source_buttons[index] = 0u;
            driver->input_source_bindings[index] = 0u;
            driver->input_source_active[index] = LIB_TRUE;
            return index;
        }
    return CORE_DRIVER_INPUT_SOURCE_CAPACITY;
}

lib_status core_driver_create(core_driver **out_driver,
    const core_driver_options *options)
{
    core_driver *driver;

    if (out_driver == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_driver = LIB_NULL;
    if (options == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    driver = lib_allocate_zero(1u, sizeof(*driver));
    if (driver == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    driver->options = *options;
    driver->text_output = options->text_output;
    {
        lib_status audio_status = lib_audio_stream_create(&(lib_audio_stream_options) {
            .sample_rate = 48000u, .channel_count = 1u
        }, &driver->audio);
        /* Input, video and CPU execution remain useful in an RDP session or
         * host without a WaveOut endpoint.  Audio is an optional host sink;
         * preserve the machine rather than reject its construction. */
        if (audio_status != LIB_STATUS_OK && audio_status != LIB_STATUS_IO_ERROR &&
            audio_status != LIB_STATUS_UNSUPPORTED) {
            lib_release(driver);
            return audio_status;
        }
    }
    lib_atomic_i32_initialize(&driver->stop_requested, 0);
    lib_atomic_i32_initialize(&driver->wake_requested, 0);
    lib_atomic_i32_initialize(&driver->debug_stop_requested, 0);
    lib_atomic_i32_initialize(&driver->input_reset_requested, 0);
    *out_driver = driver;
    return LIB_STATUS_OK;
}

lib_status core_driver_destroy(core_driver *driver)
{
    if (driver == LIB_NULL) return LIB_STATUS_OK;
    if (lib_audio_stream_destroy(&driver->audio) != LIB_STATUS_OK) return LIB_STATUS_IO_ERROR;
    core_machine_destroy(driver->machine);
    lib_release(driver);
    return LIB_STATUS_OK;
}

lib_status core_driver_make_driver(core_driver *driver,
    common_machine_driver *out_common_driver)
{
    if (driver == LIB_NULL || out_common_driver == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_common_driver = (common_machine_driver) {
        .context = driver,
        .reset = core_driver_reset,
        .run = core_driver_run,
        .request_stop = core_driver_request_stop,
        .request_wake = core_driver_request_wake,
        .set_heartbeat = core_driver_set_heartbeat,
        .set_executor_callback = core_driver_set_executor_callback,
        .deliver_input = core_driver_deliver_input,
        .copy_frame = core_driver_copy_frame,
        .set_removable_media = core_driver_set_media,
        .begin_state_read = core_driver_begin_state_read,
        .take_state_read_result = core_driver_take_state_read_result,
        .write_state = core_driver_write_state,
        .execute_debug = core_driver_debug_execute(),
        .take_debug_stop = core_driver_take_debug_stop,
        .cancel_debug = core_driver_cancel_debug
    };
    return LIB_STATUS_OK;
}

lib_status core_driver_begin_state_read(void *context,
    const common_machine_state_writer *writer)
{
    core_driver *driver = context;
    if (driver == LIB_NULL || driver->machine == LIB_NULL || writer == LIB_NULL ||
        writer->write == LIB_NULL || driver->state_read_ready) return LIB_STATUS_INVALID_STATE;
    driver->state_writer = *writer;
    driver->state_read_status = core_snapshot_write(driver->machine,
        core_driver_snapshot_write, &driver->state_writer);
    driver->state_read_ready = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_bool core_driver_take_state_read_result(void *context, lib_status *out_status)
{
    core_driver *driver = context;
    if (driver == LIB_NULL || out_status == LIB_NULL || !driver->state_read_ready)
        return LIB_FALSE;
    *out_status = driver->state_read_status;
    driver->state_read_ready = LIB_FALSE;
    driver->state_writer = (common_machine_state_writer) { 0 };
    return LIB_TRUE;
}

lib_status core_driver_write_state(void *context,
    const common_machine_state_reader *reader)
{
    core_driver *driver = context;
    lib_status status;
    if (driver == LIB_NULL || driver->machine == LIB_NULL || reader == LIB_NULL ||
        reader->read == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = core_snapshot_read(driver->machine, core_driver_snapshot_read, (void *)reader);
    if (status == LIB_STATUS_OK) {
        /* Restore starts a new run without reset: retire the previous run's
         * stop latch and frame cache only after the image has committed. */
        lib_atomic_i32_store_explicit(&driver->stop_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
        driver->published_frame_revision = 0u;
        core_driver_clear_staged_audio(driver);
        if (driver->audio != LIB_NULL) (void)lib_audio_stream_clear(driver->audio);
        core_driver_reset_pacing(driver);
    }
    return status;
}

void core_driver_request_input_reset(core_driver *driver)
{
    if (driver == LIB_NULL) return;
    lib_atomic_i32_store_explicit(&driver->input_reset_requested, 1,
        LIB_MEMORY_ORDER_SEQ_CST);
}

lib_bool core_driver_has_cartridge(const core_driver *driver)
{
    return driver != LIB_NULL && driver->machine != LIB_NULL;
}

lib_bool core_driver_reset(void *context)
{
    core_driver *driver = context;

    if (driver == LIB_NULL || driver->machine == LIB_NULL) return LIB_FALSE;
    lib_atomic_i32_store_explicit(&driver->stop_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_store_explicit(&driver->wake_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_store_explicit(&driver->debug_stop_requested, 0, LIB_MEMORY_ORDER_SEQ_CST);
    if (driver->audio != LIB_NULL) (void)lib_audio_stream_clear(driver->audio);
    core_driver_clear_staged_audio(driver);
    core_driver_reset_pacing(driver);
    return core_machine_reset(driver->machine, CORE_RESET_POWER) == LIB_STATUS_OK;
}

lib_bool core_driver_run(void *context)
{
    core_driver *driver = context;
    core_run_result result;

    if (driver == LIB_NULL) return LIB_FALSE;
    if (lib_atomic_i32_exchange_explicit(&driver->input_reset_requested, 0,
            LIB_MEMORY_ORDER_SEQ_CST) != 0) {
        core_driver_clear_inputs(driver);
    }
    if (driver->executor_callback != LIB_NULL)
        driver->executor_callback(driver->executor_context);
    if (lib_atomic_i32_load_explicit(&driver->stop_requested,
            LIB_MEMORY_ORDER_SEQ_CST) != 0) return LIB_TRUE;
    if (driver->machine == LIB_NULL) return LIB_FALSE;
    if (core_machine_run(driver->machine, 256u, 1024u, &result) != LIB_STATUS_OK)
        return LIB_FALSE;
    core_driver_drain_audio(driver);
    core_driver_pace(driver);
    if (result.trap_valid || result.reason == CORE_MACHINE_STOP_BREAKPOINT)
        lib_atomic_i32_store_explicit(&driver->debug_stop_requested,
        1, LIB_MEMORY_ORDER_SEQ_CST);
    if (driver->executor_callback != LIB_NULL)
        driver->executor_callback(driver->executor_context);
    return LIB_TRUE;
}

void core_driver_request_stop(void *context)
{
    core_driver *driver = context;
    if (driver != LIB_NULL) lib_atomic_i32_store_explicit(&driver->stop_requested,
        1, LIB_MEMORY_ORDER_SEQ_CST);
}

void core_driver_request_wake(void *context)
{
    core_driver *driver = context;
    if (driver != LIB_NULL) lib_atomic_i32_store_explicit(&driver->wake_requested,
        1, LIB_MEMORY_ORDER_SEQ_CST);
}

void core_driver_set_heartbeat(void *context, lib_bool enabled)
{
    core_driver *driver = context;
    if (driver == LIB_NULL) return;
    if (driver->audio != LIB_NULL) (void)lib_audio_stream_set_active(driver->audio, enabled);
    if (!enabled) core_driver_clear_staged_audio(driver);
    if (enabled) core_driver_reset_pacing(driver);
}

void core_driver_set_executor_callback(void *context,
    common_machine_executor_callback callback, void *callback_context)
{
    core_driver *driver = context;
    if (driver == LIB_NULL) return;
    driver->executor_callback = callback;
    driver->executor_context = callback_context;
}

void core_driver_deliver_input(void *context, const kvm_input_event *event)
{
    core_driver *driver = context;
    lib_u8 button;
    lib_u32 source;

    if (driver == LIB_NULL || event == LIB_NULL || driver->machine == LIB_NULL) return;
    if (event->type == KVM_EVENT_SOURCE_RETIRED) {
        source = core_driver_find_source(driver, event->source_identity);
        if (source == CORE_DRIVER_INPUT_SOURCE_CAPACITY) return;
        driver->input_source_identities[source] = 0u;
        driver->input_source_buttons[source] = 0u;
        driver->input_source_bindings[source] = 0u;
        driver->input_source_active[source] = LIB_FALSE;
        core_driver_publish_buttons(driver);
        return;
    }
    if (event->type == KVM_EVENT_TEXT) {
        button = core_driver_button_for_text(event->data.text.scalar);
        if (button == 0xffu || lib_atomic_i32_load_explicit(&driver->input_reset_requested,
                LIB_MEMORY_ORDER_SEQ_CST) != 0) return;
        core_controller_tap_buttons(&driver->machine->controller,
            (lib_u8)(1u << button));
        return;
    }
    if (event->type != KVM_EVENT_KEY || event->data.key.key == 0u) return;
    button = core_driver_binding_for_key(event);
    if (button == 0xffu || lib_atomic_i32_load_explicit(&driver->input_reset_requested,
            LIB_MEMORY_ORDER_SEQ_CST) != 0) return;
    source = core_driver_admit_source(driver, event->source_identity);
    if (source == CORE_DRIVER_INPUT_SOURCE_CAPACITY) return;
    if (event->data.key.pressed) {
        /* A NES only observes input on its next controller latch.  Preserve
         * every mapped make until that latch even if an RDP/virtual keyboard
         * delivers its paired break before the guest samples it.  The source
         * binding still owns held input and release semantics. */
        core_controller_tap_buttons(&driver->machine->controller,
            (lib_u8)(1u << core_driver_button_for_binding(button)));
        driver->input_source_bindings[source] |= (lib_u16)(1u << button);
    } else driver->input_source_bindings[source] &= (lib_u16)~(1u << button);
    core_driver_refresh_source_buttons(driver, source);
    core_driver_publish_buttons(driver);
}

lib_status core_driver_copy_frame(void *context, common_machine_frame *out_frame)
{
    core_driver *driver = context;
    lib_u32 index;
    lib_u32 palette_count;
    lib_bool cube;

    if (driver == LIB_NULL || out_frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    out_frame->window.valid = 0u;
    if (driver->machine == LIB_NULL || !driver->machine->ppu.frame_ready ||
        driver->machine->ppu.frame_revision == driver->published_frame_revision)
        return LIB_STATUS_OK;
    out_frame->window.graphics = driver->text_output ? 0u : 1u;
    if (driver->text_output) {
        lib_u32 row;
        lib_u32 column;
        out_frame->window.text.base.text_columns = 80u;
        out_frame->window.text.base.text_rows = 25u;
        out_frame->window.text.base.cursor_visible = 0u;
        out_frame->window.text.base.font_height = 16u;
        for (row = 0u; row < 25u; ++row) for (column = 0u; column < 80u; ++column) {
            static const lib_u8 glyphs[] = {' ', '.', ':', '-', '=', '+', '*', '#', '%', '@', 0u};
            lib_u32 source_y_begin = row * 240u / 25u;
            lib_u32 source_y_end = (row + 1u) * 240u / 25u;
            lib_u32 source_x_begin;
            lib_u32 source_x_end;
            lib_u32 total_red = 0u;
            lib_u32 total_green = 0u;
            lib_u32 total_blue = 0u;
            lib_u32 samples = 0u;
            lib_u32 source_y;
            lib_u32 source_x;
            kvm_text_cell *cell = &out_frame->window.text.base.cells[row * 80u + column];
            if (column < 7u || column >= 73u) {
                cell->glyph_index = ' '; cell->foreground = 0u;
                cell->background = 0u; cell->glyph_bank = 0u;
                continue;
            }
            source_x_begin = (column - 7u) * 256u / 66u;
            source_x_end = (column - 6u) * 256u / 66u;
            for (source_y = source_y_begin; source_y < source_y_end; ++source_y)
                for (source_x = source_x_begin; source_x < source_x_end; ++source_x) {
                    lib_u32 rgb = core_driver_rgb(driver->machine->ppu.completed[
                        source_y * 256u + source_x]);
                    total_red += rgb >> 16u;
                    total_green += (rgb >> 8u) & 0xffu;
                    total_blue += rgb & 0xffu;
                    ++samples;
                }
            {
                lib_u32 red = total_red / samples;
                lib_u32 green = total_green / samples;
                lib_u32 blue = total_blue / samples;
                lib_u32 luminance = (77u * red + 150u * green + 29u * blue + 128u) >> 8u;
                cell->glyph_index = (lib_u8)glyphs[luminance * 9u / 255u];
                cell->foreground = core_driver_nearest_rgbi((red << 16u) | (green << 8u) | blue);
                cell->background = 0u; cell->glyph_bank = 0u;
            }
        }
        for (index = 0u; index < 256u; ++index) {
            out_frame->characters.primary[index] = index < 128u ? (lib_u16)index : ' ';
            out_frame->characters.secondary[index] = index < 128u ? (lib_u16)index : ' ';
        }
        out_frame->window.valid = 1u;
        driver->published_frame_revision = driver->machine->ppu.frame_revision;
        return LIB_STATUS_OK;
    }
    out_frame->window.image.width = CORE_PPU_WIDTH;
    out_frame->window.image.height = CORE_PPU_HEIGHT;
    out_frame->window.image.stride = CORE_PPU_WIDTH;
    lib_memory_set(out_frame->window.image.palette, 0,
        sizeof(out_frame->window.image.palette));
    palette_count = 0u;
    cube = LIB_FALSE;
    for (index = 0u; index < CORE_PPU_WIDTH * CORE_PPU_HEIGHT; ++index) {
        lib_u32 rgb = core_driver_rgb(driver->machine->ppu.completed[index]);
        lib_u32 candidate;
        for (candidate = 0u; candidate < palette_count; ++candidate)
            if (out_frame->window.image.palette[candidate] == rgb) break;
        if (candidate == palette_count) {
            if (palette_count == KVM_WINDOW_GRAPHICS_PALETTE_ENTRIES) {
                cube = LIB_TRUE;
                break;
            }
            out_frame->window.image.palette[palette_count++] = rgb;
        }
        out_frame->window.image.pixels[index] = (lib_u8)candidate;
    }
    if (cube) for (index = 0u; index < KVM_WINDOW_GRAPHICS_PALETTE_ENTRIES; ++index) {
        lib_u32 red = index / 36u;
        lib_u32 green = (index / 6u) % 6u;
        lib_u32 blue = index % 6u;
        out_frame->window.image.palette[index] = (red * 51u << 16u) |
            (green * 51u << 8u) | blue * 51u;
    }
    if (cube) for (index = 0u; index < CORE_PPU_WIDTH * CORE_PPU_HEIGHT; ++index) {
        lib_u32 rgb = core_driver_rgb(driver->machine->ppu.completed[index]);
        lib_u32 red = ((rgb >> 16u) + 25u) / 51u;
        lib_u32 green = (((rgb >> 8u) & 0xffu) + 25u) / 51u;
        lib_u32 blue = ((rgb & 0xffu) + 25u) / 51u;
        out_frame->window.image.pixels[index] = (lib_u8)(36u * red + 6u * green + blue);
    }
    out_frame->window.valid = 1u;
    driver->published_frame_revision = driver->machine->ppu.frame_revision;
    return LIB_STATUS_OK;
}

lib_bool core_driver_take_debug_stop(void *context)
{
    core_driver *driver = context;
    return driver != LIB_NULL && lib_atomic_i32_exchange_explicit(
        &driver->debug_stop_requested, 0, LIB_MEMORY_ORDER_SEQ_CST) != 0;
}

void core_driver_cancel_debug(void *context)
{
    core_driver *driver = context;
    if (driver != LIB_NULL) lib_atomic_i32_store_explicit(&driver->debug_stop_requested,
        0, LIB_MEMORY_ORDER_SEQ_CST);
}
