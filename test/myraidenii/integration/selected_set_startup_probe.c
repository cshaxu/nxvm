#include "lib/types/file.h"
#include "myraidenii/core/board_interface.h"
#include "myraidenii/core/sound.h"
#include "myraidenii/core/video.h"
#include "myraidenii/machine/configuration_interface.h"
#include "myraidenii/machine/driver_interface.h"
#include "x86-core/machine/machine_interface.h"

#define MYRAIDENII_STARTUP_DEFAULT_INSTRUCTION_BUDGET 2000000u
#define MYRAIDENII_STARTUP_MAXIMUM_INSTRUCTION_BUDGET 20000000u
#define MYRAIDENII_STARTUP_FRAME_SAMPLE_COUNT 4u

typedef enum myraidenii_startup_control_mode {
    MYRAIDENII_STARTUP_CONTROL_NONE,
    MYRAIDENII_STARTUP_CONTROL_HELD,
    MYRAIDENII_STARTUP_CONTROL_PULSE,
    MYRAIDENII_STARTUP_CONTROL_SEQUENCE
} myraidenii_startup_control_mode;

typedef struct myraidenii_startup_frame_sample {
    lib_u64 executed;
    lib_u64 digest;
    lib_u64 nonblack;
    lib_u8 sound_command_0;
    lib_u8 sound_command_1;
    lib_bool sound_interrupt_pending;
} myraidenii_startup_frame_sample;

static lib_bool myraidenii_startup_parse_arguments(lib_i32 argument_count, char **arguments,
    myraidenii_startup_control_mode *out_control_mode, lib_u64 *out_instruction_budget)
{
    static const char budget_prefix[] = "--instruction-budget=";
    myraidenii_startup_control_mode control_mode = MYRAIDENII_STARTUP_CONTROL_NONE;
    lib_bool budget_seen = LIB_FALSE;
    lib_u64 instruction_budget = MYRAIDENII_STARTUP_DEFAULT_INSTRUCTION_BUDGET;

    if (arguments == LIB_NULL || out_control_mode == LIB_NULL ||
        out_instruction_budget == LIB_NULL || argument_count < 13 || argument_count > 15)
        return LIB_FALSE;
    for (lib_i32 index = 13; index < argument_count; ++index) {
        const char *argument = arguments[index];

        if (argument == LIB_NULL) return LIB_FALSE;
        if (lib_text_compare(argument, "--coin-start-held") == 0) {
            if (control_mode != MYRAIDENII_STARTUP_CONTROL_NONE) return LIB_FALSE;
            control_mode = MYRAIDENII_STARTUP_CONTROL_HELD;
        } else if (lib_text_compare(argument, "--coin-start-pulse") == 0) {
            if (control_mode != MYRAIDENII_STARTUP_CONTROL_NONE) return LIB_FALSE;
            control_mode = MYRAIDENII_STARTUP_CONTROL_PULSE;
        } else if (lib_text_compare(argument, "--coin-start-sequence") == 0) {
            if (control_mode != MYRAIDENII_STARTUP_CONTROL_NONE) return LIB_FALSE;
            control_mode = MYRAIDENII_STARTUP_CONTROL_SEQUENCE;
        } else {
            lib_size prefix_length = sizeof(budget_prefix) - 1u;
            lib_u64 value = 0u;

            if (budget_seen || lib_memory_compare(argument, budget_prefix, prefix_length) != 0 ||
                argument[prefix_length] == '\0') return LIB_FALSE;
            for (const char *digit = argument + prefix_length; *digit != '\0'; ++digit) {
                if (*digit < '0' || *digit > '9' ||
                    value > (MYRAIDENII_STARTUP_MAXIMUM_INSTRUCTION_BUDGET -
                        (lib_u64)(*digit - '0')) / 10u)
                    return LIB_FALSE;
                value = value * 10u + (lib_u64)(*digit - '0');
            }
            if (value < MYRAIDENII_STARTUP_DEFAULT_INSTRUCTION_BUDGET) return LIB_FALSE;
            instruction_budget = value;
            budget_seen = LIB_TRUE;
        }
    }
    *out_control_mode = control_mode;
    *out_instruction_budget = instruction_budget;
    return LIB_TRUE;
}

static lib_status myraidenii_startup_set_control(const emulator_machine_driver *driver,
    lib_u16 key, lib_bool pressed)
{
    kvm_input_event event = {0};

    if (driver == LIB_NULL || driver->context == LIB_NULL ||
        driver->deliver_input == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    event.type = KVM_EVENT_KEY;
    event.data.key.key = key;
    event.data.key.pressed = pressed;
    driver->deliver_input(driver->context, &event);
    return LIB_STATUS_OK;
}

static lib_status myraidenii_startup_set_coin_start(const emulator_machine_driver *driver,
    lib_bool pressed)
{
    lib_status status = myraidenii_startup_set_control(driver, '5', pressed);

    return status == LIB_STATUS_OK ?
        myraidenii_startup_set_control(driver, '1', pressed) : status;
}

static void myraidenii_startup_frame_observe(const lib_u32 *pixels, lib_size pixel_count,
    lib_u64 *out_digest, lib_u64 *out_nonblack)
{
    lib_u64 digest = 1469598103934665603ull;
    lib_u64 nonblack = 0u;
    lib_size index;

    for (index = 0u; index < pixel_count; ++index) {
        digest ^= pixels[index];
        digest *= 1099511628211ull;
        if (pixels[index] != 0u) ++nonblack;
    }
    *out_digest = digest;
    *out_nonblack = nonblack;
}

static lib_status myraidenii_startup_frame_sample_capture(
    const myraidenii_core_board *board, lib_u32 *pixels, lib_u64 executed,
    myraidenii_startup_frame_sample *out_sample)
{
    lib_status status;

    if (board == LIB_NULL || pixels == LIB_NULL || out_sample == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = myraidenii_core_board_frame_copy(board, pixels,
        MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS);
    if (status != LIB_STATUS_OK) return status;
    out_sample->executed = executed;
    myraidenii_startup_frame_observe(pixels, MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS,
        &out_sample->digest, &out_sample->nonblack);
    status = myraidenii_core_board_sound_command_get(board, 0u, &out_sample->sound_command_0);
    if (status != LIB_STATUS_OK) return status;
    status = myraidenii_core_board_sound_command_get(board, 1u, &out_sample->sound_command_1);
    if (status != LIB_STATUS_OK) return status;
    return myraidenii_core_board_sound_interrupt_is_pending(board,
        &out_sample->sound_interrupt_pending);
}

static void myraidenii_startup_pcm_observe(const lib_i16 *samples, lib_u32 frame_count,
    lib_u64 *out_digest, lib_u64 *out_nonzero)
{
    lib_u64 digest = 1469598103934665603ull;
    lib_u64 nonzero = 0u;

    for (lib_u32 index = 0u; index < frame_count; ++index) {
        const lib_u16 left = (lib_u16)samples[index *
            MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
        const lib_u16 right = (lib_u16)samples[index *
            MYRAIDENII_CORE_AUDIO_PCM_CHANNELS + 1u];

        digest ^= left;
        digest *= 1099511628211ull;
        digest ^= right;
        digest *= 1099511628211ull;
        if (left != 0u || right != 0u) ++nonzero;
    }
    *out_digest = digest;
    *out_nonzero = nonzero;
}

int main(int argument_count, char **arguments)
{
    const char *image_paths[MYRAIDENII_MACHINE_IMAGE_COUNT];
    myraidenii_machine_images *images = LIB_NULL;
    myraidenii_core_board_config config;
    myraidenii_core_board *board = LIB_NULL;
    myraidenii_machine *machine_adapter = LIB_NULL;
    emulator_machine_driver driver = {0};
    x86_core_machine *machine;
    lib_u32 *pixels = LIB_NULL;
    lib_i16 *pcm_samples = LIB_NULL;
    lib_u64 instruction_budget;
    lib_u64 remaining;
    lib_u64 next_frame_sample;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_diagnostic diagnostic = {0};
    myraidenii_core_cop_failure_observation cop_failure = {0};
    myraidenii_core_board_control_failure_observation control_failure = {0};
    lib_status status;
    lib_bool terminal_fault = LIB_FALSE;
    lib_u64 frame_digest = 0u;
    lib_u64 nonblack_pixels = 0u;
    lib_u64 pcm_digest = 0u;
    lib_u64 nonzero_pcm_frames = 0u;
    lib_u64 dropped_pcm_frames = 0u;
    lib_u32 pcm_frames = 0u;
    myraidenii_core_board_sound_observation sound_observation = {0};
    myraidenii_core_board_video_observation video_observation = {0};
    myraidenii_startup_control_mode control_mode;
    lib_bool coin_pressed = LIB_FALSE;
    lib_bool coin_released = LIB_FALSE;
    lib_bool start_pressed = LIB_FALSE;
    lib_bool start_released = LIB_FALSE;
    lib_u64 pulse_press_at;
    lib_u64 pulse_release_at;
    lib_u64 start_press_at;
    lib_u64 start_release_at;
    myraidenii_startup_frame_sample frame_samples[MYRAIDENII_STARTUP_FRAME_SAMPLE_COUNT] = {0};
    lib_u8 frame_sample_count = 0u;
    lib_u8 frame_transition_count = 0u;
    int exit_code = 1;

    if (!myraidenii_startup_parse_arguments(argument_count, arguments, &control_mode,
            &instruction_budget)) {
        lib_c_printf("MYRAIDENII:STARTUP-PROBE:USAGE\n");
        return 2;
    }
    remaining = instruction_budget;
    pulse_press_at = instruction_budget / 4u;
    pulse_release_at = instruction_budget / 2u;
    start_press_at = instruction_budget * 3u / 5u;
    start_release_at = instruction_budget * 13u / 20u;
    for (lib_size index = 0u; index < MYRAIDENII_MACHINE_IMAGE_COUNT; ++index)
        image_paths[index] = arguments[index + 1u];
    status = myraidenii_machine_images_load(".", image_paths, &images);
    if (status == LIB_STATUS_OK) {
        status = myraidenii_machine_images_board_config(images, &config);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_board_create(&config, &board);
    }
    if (status == LIB_STATUS_OK)
        status = myraidenii_machine_create(board, LIB_NULL, &machine_adapter);
    if (status == LIB_STATUS_OK)
        status = myraidenii_machine_driver_get(machine_adapter, &driver);
    if (status == LIB_STATUS_OK && control_mode == MYRAIDENII_STARTUP_CONTROL_HELD)
        status = myraidenii_startup_set_coin_start(&driver, LIB_TRUE);
    machine = myraidenii_core_board_machine_get(board);
    if (status == LIB_STATUS_OK) {
        pixels = lib_allocate(MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS * sizeof(*pixels));
        if (pixels == LIB_NULL) status = LIB_STATUS_NO_MEMORY;
    }
    next_frame_sample = instruction_budget / MYRAIDENII_STARTUP_FRAME_SAMPLE_COUNT;
    while (status == LIB_STATUS_OK && remaining != 0u) {
        const lib_u64 executed = instruction_budget - remaining;
        const lib_u64 quantum = remaining < 4096u ? remaining : 4096u;

        if (control_mode == MYRAIDENII_STARTUP_CONTROL_PULSE && !coin_pressed &&
            executed >= pulse_press_at) {
            status = myraidenii_startup_set_coin_start(&driver, LIB_TRUE);
            coin_pressed = status == LIB_STATUS_OK;
            start_pressed = coin_pressed;
        }
        if (status == LIB_STATUS_OK && control_mode == MYRAIDENII_STARTUP_CONTROL_PULSE &&
            coin_pressed && !coin_released && executed >= pulse_release_at) {
            status = myraidenii_startup_set_coin_start(&driver, LIB_FALSE);
            coin_released = status == LIB_STATUS_OK;
            start_released = coin_released;
        }
        if (status == LIB_STATUS_OK && control_mode == MYRAIDENII_STARTUP_CONTROL_SEQUENCE &&
            !coin_pressed && executed >= pulse_press_at) {
            status = myraidenii_startup_set_control(&driver, '5', LIB_TRUE);
            coin_pressed = status == LIB_STATUS_OK;
        }
        if (status == LIB_STATUS_OK && control_mode == MYRAIDENII_STARTUP_CONTROL_SEQUENCE &&
            coin_pressed && !coin_released && executed >= pulse_release_at) {
            status = myraidenii_startup_set_control(&driver, '5', LIB_FALSE);
            coin_released = status == LIB_STATUS_OK;
        }
        if (status == LIB_STATUS_OK && control_mode == MYRAIDENII_STARTUP_CONTROL_SEQUENCE &&
            coin_released && !start_pressed && executed >= start_press_at) {
            status = myraidenii_startup_set_control(&driver, '1', LIB_TRUE);
            start_pressed = status == LIB_STATUS_OK;
        }
        if (status == LIB_STATUS_OK && control_mode == MYRAIDENII_STARTUP_CONTROL_SEQUENCE &&
            start_pressed && !start_released && executed >= start_release_at) {
            status = myraidenii_startup_set_control(&driver, '1', LIB_FALSE);
            start_released = status == LIB_STATUS_OK;
        }
        if (status != LIB_STATUS_OK) break;
        status = x86_core_machine_run(machine, (x86_core_machine_run_budget){quantum, 0u},
            &result);
        terminal_fault = status == LIB_STATUS_INTERNAL_ERROR &&
            result.reason == X86_CORE_MACHINE_STOP_FAULT;
        if ((!terminal_fault && status != LIB_STATUS_OK) || result.executed > remaining) break;
        remaining -= result.executed;
        if (terminal_fault || result.reason != X86_CORE_MACHINE_STOP_BUDGET) break;
        if (result.executed == 0u) {
            status = LIB_STATUS_INTERNAL_ERROR;
            break;
        }
        while (frame_sample_count < MYRAIDENII_STARTUP_FRAME_SAMPLE_COUNT &&
            instruction_budget - remaining >= next_frame_sample) {
            status = myraidenii_startup_frame_sample_capture(board, pixels,
                instruction_budget - remaining, &frame_samples[frame_sample_count]);
            if (status != LIB_STATUS_OK) break;
            ++frame_sample_count;
            next_frame_sample = frame_sample_count == MYRAIDENII_STARTUP_FRAME_SAMPLE_COUNT ?
                instruction_budget : instruction_budget * (frame_sample_count + 1u) /
                    MYRAIDENII_STARTUP_FRAME_SAMPLE_COUNT;
        }
    }
    if (status == LIB_STATUS_OK || terminal_fault) {
        if (frame_sample_count == 0u) {
            status = myraidenii_startup_frame_sample_capture(board, pixels,
                instruction_budget - remaining, &frame_samples[frame_sample_count]);
            if (status == LIB_STATUS_OK) ++frame_sample_count;
        }
        if (status == LIB_STATUS_OK)
            frame_digest = frame_samples[frame_sample_count - 1u].digest;
        if (status == LIB_STATUS_OK)
            nonblack_pixels = frame_samples[frame_sample_count - 1u].nonblack;
    }
    if (status == LIB_STATUS_OK || terminal_fault) {
        pcm_samples = lib_allocate(MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY *
            MYRAIDENII_CORE_AUDIO_PCM_CHANNELS * sizeof(*pcm_samples));
        if (pcm_samples == LIB_NULL) status = LIB_STATUS_NO_MEMORY;
        if (status == LIB_STATUS_OK || terminal_fault) {
            status = myraidenii_core_board_pcm_copy(board, pcm_samples,
                MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY, &pcm_frames,
                &dropped_pcm_frames);
            if (status == LIB_STATUS_OK)
                myraidenii_startup_pcm_observe(pcm_samples, pcm_frames, &pcm_digest,
                    &nonzero_pcm_frames);
            if (status == LIB_STATUS_OK)
                status = myraidenii_core_board_sound_observation_get(board,
                    &sound_observation);
            if (status == LIB_STATUS_OK)
                status = myraidenii_core_board_video_observation_get(board,
                    &video_observation);
        }
    }
    if (status == LIB_STATUS_OK || terminal_fault) {
        const lib_bool has_diagnostic = terminal_fault &&
            x86_core_machine_get_cpu_diagnostic(machine, &diagnostic) == LIB_STATUS_OK;
        const lib_bool fault_valid = has_diagnostic && diagnostic.first_fault.valid;
        const lib_u32 fault_code = fault_valid ? diagnostic.first_fault.exception_code : 0u;
        const lib_u32 fault_mask = fault_valid ? diagnostic.first_fault.exception_mask : 0u;
        const lib_u32 fault_linear = fault_valid ? diagnostic.first_fault.point.linear_pc : 0u;
        const lib_u32 delivered_exceptions = has_diagnostic ?
            diagnostic.delivered_exception_count : 0u;

        if (terminal_fault) {
            (void)myraidenii_core_board_cop_failure_get(board, &cop_failure);
            (void)myraidenii_core_board_control_failure_get(board, &control_failure);
        }
        for (lib_u8 index = 1u; index < frame_sample_count; ++index) {
            if (frame_samples[index].digest != frame_samples[index - 1u].digest ||
                frame_samples[index].nonblack != frame_samples[index - 1u].nonblack) {
                ++frame_transition_count;
            }
        }

        lib_c_printf("MYRAIDENII:STARTUP-PROBE:OK reason=%u detail=%u fault-valid=%u "
            "fault-code=%u fault-mask=%u fault-linear=%u delivered-exceptions=%u "
            "cop-operation=%u cop-value=%u cop-source=%u cop-size=%u "
            "cop-destination=%u cop-relative=%u cop-physical=%u cop-status=%u "
            "control-valid=%u control-physical=%u control-bytes=%u control-access=%u "
            "input-route=machine-driver coin-start-mode=%u coin-press-at=%llu coin-release-at=%llu "
            "start-press-at=%llu start-release-at=%llu coin-pressed=%u coin-released=%u "
            "start-pressed=%u start-released=%u instruction-budget=%llu "
            "executed=%llu ticks=%llu frame-width=%u frame-height=%u nonblack=%llu "
            "frame-digest=%016llx frame-samples=%u frame-transitions=%u "
            "pcm-frames=%u pcm-dropped=%llu pcm-nonzero=%llu "
            "pcm-digest=%016llx sound-command-0-reads=%llu sound-command-1-reads=%llu "
            "sound-command-acknowledgements=%llu sound-interrupt-accepts=%llu "
            "ym-address-writes=%llu ym-data-writes=%llu "
            "oki-1-writes=%llu oki-2-writes=%llu oki-1-commands=%llu "
            "oki-2-commands=%llu oki-1-selections=%llu oki-2-selections=%llu "
            "oki-1-stops=%llu oki-2-stops=%llu oki-1-rejected=%llu "
            "oki-2-rejected=%llu oki-1-ignored=%llu oki-2-ignored=%llu "
            "oki-1-voice-starts=%llu oki-2-voice-starts=%llu pcm-nonzero-generated=%llu "
            "video-vblanks=%llu video-crtc-writes=%llu video-sprite-flag-writes=%llu "
            "video-sprite-latches=%llu video-palette-commits=%llu video-tilemap-commits=%llu\n",
            (lib_u32)result.reason, result.detail, (lib_u32)fault_valid, fault_code, fault_mask,
            fault_linear, delivered_exceptions, (lib_u32)cop_failure.operation,
            (lib_u32)cop_failure.operation_value, (lib_u32)cop_failure.dma_source,
            (lib_u32)cop_failure.dma_size, (lib_u32)cop_failure.dma_destination,
            (lib_u32)cop_failure.dma_relative,
            cop_failure.physical,
            (lib_u32)cop_failure.status,
            (lib_u32)control_failure.valid, control_failure.physical,
            (lib_u32)control_failure.bytes, (lib_u32)control_failure.access,
            (lib_u32)control_mode, (unsigned long long)pulse_press_at,
            (unsigned long long)pulse_release_at, (unsigned long long)start_press_at,
            (unsigned long long)start_release_at, (lib_u32)coin_pressed,
            (lib_u32)coin_released, (lib_u32)start_pressed, (lib_u32)start_released,
            (unsigned long long)instruction_budget,
            (unsigned long long)(instruction_budget - remaining),
            (unsigned long long)result.elapsed_ticks, MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH,
            MYRAIDENII_CORE_GRAPHICS_FRAME_HEIGHT, (unsigned long long)nonblack_pixels,
            (unsigned long long)frame_digest, frame_sample_count, frame_transition_count, pcm_frames,
            (unsigned long long)dropped_pcm_frames,
            (unsigned long long)nonzero_pcm_frames, (unsigned long long)pcm_digest,
            (unsigned long long)sound_observation.transport_command_reads[0u],
            (unsigned long long)sound_observation.transport_command_reads[1u],
            (unsigned long long)sound_observation.transport_command_acknowledgements,
            (unsigned long long)sound_observation.transport_interrupt_accepts,
            (unsigned long long)sound_observation.ym_address_writes,
            (unsigned long long)sound_observation.ym_data_writes,
            (unsigned long long)sound_observation.oki_writes[0u],
            (unsigned long long)sound_observation.oki_writes[1u],
            (unsigned long long)sound_observation.oki_sample_commands[0u],
            (unsigned long long)sound_observation.oki_sample_commands[1u],
            (unsigned long long)sound_observation.oki_voice_selections[0u],
            (unsigned long long)sound_observation.oki_voice_selections[1u],
            (unsigned long long)sound_observation.oki_stops[0u],
            (unsigned long long)sound_observation.oki_stops[1u],
            (unsigned long long)sound_observation.oki_rejected_selections[0u],
            (unsigned long long)sound_observation.oki_rejected_selections[1u],
            (unsigned long long)sound_observation.oki_ignored_controls[0u],
            (unsigned long long)sound_observation.oki_ignored_controls[1u],
            (unsigned long long)sound_observation.oki_voice_starts[0u],
            (unsigned long long)sound_observation.oki_voice_starts[1u],
            (unsigned long long)sound_observation.pcm_nonzero_frames,
            (unsigned long long)video_observation.vblank_events,
            (unsigned long long)video_observation.crtc_byte_writes,
            (unsigned long long)video_observation.sprite_flag_byte_writes,
            (unsigned long long)video_observation.sprite_latch_events,
            (unsigned long long)video_observation.palette_commits,
            (unsigned long long)video_observation.tilemap_commits);
        for (lib_u8 index = 0u; index < frame_sample_count; ++index) {
            lib_c_printf("MYRAIDENII:STARTUP-PROBE:FRAME-SAMPLE index=%u executed=%llu "
                "digest=%016llx nonblack=%llu sound-command-0=%u sound-command-1=%u "
                "sound-interrupt-pending=%u\n", index + 1u,
                (unsigned long long)frame_samples[index].executed,
                (unsigned long long)frame_samples[index].digest,
                (unsigned long long)frame_samples[index].nonblack,
                frame_samples[index].sound_command_0, frame_samples[index].sound_command_1,
                (lib_u32)frame_samples[index].sound_interrupt_pending);
        }
        exit_code = 0;
    } else {
        lib_c_printf("MYRAIDENII:STARTUP-PROBE:FAIL status=%u\n", (lib_u32)status);
    }
    lib_release(pixels);
    lib_release(pcm_samples);
    (void)myraidenii_machine_destroy(&machine_adapter);
    myraidenii_core_board_destroy(board);
    myraidenii_machine_images_destroy(images);
    return exit_code;
}
