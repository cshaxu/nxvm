#include "lib/types/file.h"
#include "myraidenii/core/board_interface.h"
#include "myraidenii/core/sound.h"
#include "myraidenii/core/video.h"
#include "myraidenii/machine/configuration_interface.h"
#include "x86-core/machine/machine_interface.h"

#define MYRAIDENII_STARTUP_DEFAULT_INSTRUCTION_BUDGET 2000000u
#define MYRAIDENII_STARTUP_MAXIMUM_INSTRUCTION_BUDGET 20000000u
#define MYRAIDENII_STARTUP_FRAME_SAMPLE_COUNT 4u

typedef struct myraidenii_startup_frame_sample {
    lib_u64 executed;
    lib_u64 digest;
    lib_u64 nonblack;
} myraidenii_startup_frame_sample;

static lib_bool myraidenii_startup_parse_arguments(lib_i32 argument_count, char **arguments,
    lib_bool *out_coin_start_held, lib_u64 *out_instruction_budget)
{
    static const char budget_prefix[] = "--instruction-budget=";
    lib_bool coin_start_held = LIB_FALSE;
    lib_bool budget_seen = LIB_FALSE;
    lib_u64 instruction_budget = MYRAIDENII_STARTUP_DEFAULT_INSTRUCTION_BUDGET;

    if (arguments == LIB_NULL || out_coin_start_held == LIB_NULL ||
        out_instruction_budget == LIB_NULL || argument_count < 13 || argument_count > 15)
        return LIB_FALSE;
    for (lib_i32 index = 13; index < argument_count; ++index) {
        const char *argument = arguments[index];

        if (argument == LIB_NULL) return LIB_FALSE;
        if (lib_text_compare(argument, "--coin-start-held") == 0) {
            if (coin_start_held) return LIB_FALSE;
            coin_start_held = LIB_TRUE;
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
    *out_coin_start_held = coin_start_held;
    *out_instruction_budget = instruction_budget;
    return LIB_TRUE;
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
    return LIB_STATUS_OK;
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
    lib_bool coin_start_held;
    myraidenii_startup_frame_sample frame_samples[MYRAIDENII_STARTUP_FRAME_SAMPLE_COUNT] = {0};
    lib_u8 frame_sample_count = 0u;
    lib_u8 frame_transition_count = 0u;
    int exit_code = 1;

    if (!myraidenii_startup_parse_arguments(argument_count, arguments, &coin_start_held,
            &instruction_budget)) {
        lib_c_printf("MYRAIDENII:STARTUP-PROBE:USAGE\n");
        return 2;
    }
    remaining = instruction_budget;
    for (lib_size index = 0u; index < MYRAIDENII_MACHINE_IMAGE_COUNT; ++index)
        image_paths[index] = arguments[index + 1u];
    status = myraidenii_machine_images_load(".", image_paths, &images);
    if (status == LIB_STATUS_OK) {
        status = myraidenii_machine_images_board_config(images, &config);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_board_create(&config, &board);
    }
    if (status == LIB_STATUS_OK && coin_start_held) {
        status = myraidenii_core_board_set_control(board,
            MYRAIDENII_CORE_BOARD_CONTROL_COIN_1, LIB_TRUE);
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_board_set_control(board,
                MYRAIDENII_CORE_BOARD_CONTROL_START_1, LIB_TRUE);
    }
    machine = myraidenii_core_board_machine_get(board);
    if (status == LIB_STATUS_OK) {
        pixels = lib_allocate(MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS * sizeof(*pixels));
        if (pixels == LIB_NULL) status = LIB_STATUS_NO_MEMORY;
    }
    next_frame_sample = instruction_budget / MYRAIDENII_STARTUP_FRAME_SAMPLE_COUNT;
    while (status == LIB_STATUS_OK && remaining != 0u) {
        const lib_u64 quantum = remaining < 4096u ? remaining : 4096u;

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
            "coin-start-held=%u instruction-budget=%llu "
            "executed=%llu ticks=%llu frame-width=%u frame-height=%u nonblack=%llu "
            "frame-digest=%016llx frame-samples=%u frame-transitions=%u "
            "pcm-frames=%u pcm-dropped=%llu pcm-nonzero=%llu "
            "pcm-digest=%016llx\n",
            (lib_u32)result.reason, result.detail, (lib_u32)fault_valid, fault_code, fault_mask,
            fault_linear, delivered_exceptions, (lib_u32)cop_failure.operation,
            (lib_u32)cop_failure.operation_value, (lib_u32)cop_failure.dma_source,
            (lib_u32)cop_failure.dma_size, (lib_u32)cop_failure.dma_destination,
            (lib_u32)cop_failure.dma_relative,
            cop_failure.physical,
            (lib_u32)cop_failure.status,
            (lib_u32)control_failure.valid, control_failure.physical,
            (lib_u32)control_failure.bytes, (lib_u32)control_failure.access,
            (lib_u32)coin_start_held,
            (unsigned long long)instruction_budget,
            (unsigned long long)(instruction_budget - remaining),
            (unsigned long long)result.elapsed_ticks, MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH,
            MYRAIDENII_CORE_GRAPHICS_FRAME_HEIGHT, (unsigned long long)nonblack_pixels,
            (unsigned long long)frame_digest, frame_sample_count, frame_transition_count, pcm_frames,
            (unsigned long long)dropped_pcm_frames,
            (unsigned long long)nonzero_pcm_frames, (unsigned long long)pcm_digest);
        for (lib_u8 index = 0u; index < frame_sample_count; ++index) {
            lib_c_printf("MYRAIDENII:STARTUP-PROBE:FRAME-SAMPLE index=%u executed=%llu "
                "digest=%016llx nonblack=%llu\n", index + 1u,
                (unsigned long long)frame_samples[index].executed,
                (unsigned long long)frame_samples[index].digest,
                (unsigned long long)frame_samples[index].nonblack);
        }
        exit_code = 0;
    } else {
        lib_c_printf("MYRAIDENII:STARTUP-PROBE:FAIL status=%u\n", (lib_u32)status);
    }
    lib_release(pixels);
    lib_release(pcm_samples);
    myraidenii_core_board_destroy(board);
    myraidenii_machine_images_destroy(images);
    return exit_code;
}
