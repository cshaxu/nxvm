#include "lib/storage/file_interface.h"
#include "lib/types/file.h"
#include "myraidenii/core/board_interface.h"
#include "myraidenii/core/sound.h"
#include "myraidenii/core/video.h"
#include "x86-core/machine/machine_interface.h"

#define MYRAIDENII_STARTUP_INSTRUCTION_BUDGET 2000000u

static lib_status myraidenii_startup_read_image(const char *path, lib_size expected_bytes,
    lib_u8 **out_bytes)
{
    void *bytes = LIB_NULL;
    lib_size byte_count = 0u;
    lib_status status;

    if (path == LIB_NULL || expected_bytes == 0u || out_bytes == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_bytes = LIB_NULL;
    status = lib_storage_file_read_owned(path, expected_bytes, &bytes, &byte_count);
    if (status != LIB_STATUS_OK) return status;
    if (byte_count != expected_bytes) {
        lib_release(bytes);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_bytes = bytes;
    return LIB_STATUS_OK;
}

static void myraidenii_startup_images_release(lib_u8 **images, lib_size image_count)
{
    lib_size index;

    for (index = 0u; index < image_count; ++index) lib_release(images[index]);
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

int main(int argument_count, char **arguments)
{
    const myraidenii_core_board_config config = {
        .program_lane_bytes = MYRAIDENII_CORE_PROGRAM_LANE_BYTES
    };
    const lib_size image_sizes[9] = {
        MYRAIDENII_CORE_PROGRAM_LANE_BYTES,
        MYRAIDENII_CORE_PROGRAM_LANE_BYTES,
        MYRAIDENII_CORE_CHARACTER_IMAGE_BYTES,
        MYRAIDENII_CORE_BACKGROUND_IMAGE_BYTES,
        MYRAIDENII_CORE_BACKGROUND_IMAGE_BYTES,
        MYRAIDENII_CORE_SPRITE_IMAGE_BYTES,
        MYRAIDENII_CORE_SPRITE_IMAGE_BYTES,
        MYRAIDENII_CORE_SPRITE_IMAGE_BYTES,
        MYRAIDENII_CORE_SPRITE_IMAGE_BYTES
    };
    myraidenii_core_board *board = LIB_NULL;
    x86_core_machine *machine;
    lib_u8 *images[9] = {LIB_NULL};
    lib_u8 *sound_program = LIB_NULL;
    lib_u32 *pixels = LIB_NULL;
    lib_u64 remaining = MYRAIDENII_STARTUP_INSTRUCTION_BUDGET;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_diagnostic diagnostic = {0};
    myraidenii_core_cop_failure_observation cop_failure = {0};
    myraidenii_core_board_control_failure_observation control_failure = {0};
    lib_status status;
    lib_bool terminal_fault = LIB_FALSE;
    lib_u64 frame_digest = 0u;
    lib_u64 nonblack_pixels = 0u;
    lib_size index;
    int exit_code = 1;

    if (argument_count != 11) {
        lib_c_printf("MYRAIDENII:STARTUP-PROBE:USAGE\n");
        return 2;
    }
    status = LIB_STATUS_OK;
    for (index = 0u; index < 9u && status == LIB_STATUS_OK; ++index)
        status = myraidenii_startup_read_image(arguments[index + 1u], image_sizes[index],
            &images[index]);
    if (status == LIB_STATUS_OK)
        status = myraidenii_startup_read_image(arguments[10],
            MYRAIDENII_CORE_SOUND_PROGRAM_BYTES, &sound_program);
    if (status == LIB_STATUS_OK) {
        const myraidenii_core_board_graphics_images graphics = {
            .characters = images[2], .background_1 = images[3], .background_2 = images[4],
            .sprites_1 = images[5], .sprites_2 = images[6], .sprites_3 = images[7],
            .sprites_4 = images[8]
        };
        myraidenii_core_board_config mutable_config = config;

        mutable_config.program_even = images[0];
        mutable_config.program_odd = images[1];
        mutable_config.graphics_images = &graphics;
        mutable_config.sound_program = sound_program;
        mutable_config.sound_program_bytes = MYRAIDENII_CORE_SOUND_PROGRAM_BYTES;
        status = myraidenii_core_board_create(&mutable_config, &board);
    }
    myraidenii_startup_images_release(images, 9u);
    lib_release(sound_program);
    machine = myraidenii_core_board_machine_get(board);
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
    }
    if (status == LIB_STATUS_OK || terminal_fault) {
        pixels = lib_allocate(MYRAIDENII_CORE_VIDEO_FRAME_PIXELS * sizeof(*pixels));
        if (pixels == LIB_NULL) status = LIB_STATUS_NO_MEMORY;
        if (status == LIB_STATUS_OK || terminal_fault) {
            status = myraidenii_core_board_frame_copy(board, pixels,
                MYRAIDENII_CORE_VIDEO_FRAME_PIXELS);
            if (status == LIB_STATUS_OK)
                myraidenii_startup_frame_observe(pixels,
                    MYRAIDENII_CORE_VIDEO_FRAME_PIXELS, &frame_digest, &nonblack_pixels);
        }
    }
    if (status == LIB_STATUS_OK || terminal_fault) {
        const lib_u32 fault_code = terminal_fault &&
            x86_core_machine_get_cpu_diagnostic(machine, &diagnostic) == LIB_STATUS_OK &&
            diagnostic.first_fault.valid ? diagnostic.first_fault.exception_code : 0u;

        if (terminal_fault) {
            (void)myraidenii_core_board_cop_failure_get(board, &cop_failure);
            (void)myraidenii_core_board_control_failure_get(board, &control_failure);
        }

        lib_c_printf("MYRAIDENII:STARTUP-PROBE:OK reason=%u detail=%u fault-code=%u "
            "cop-operation=%u cop-value=%u cop-source=%u cop-size=%u "
            "cop-destination=%u cop-relative=%u cop-physical=%u cop-status=%u "
            "control-valid=%u control-physical=%u control-bytes=%u control-access=%u "
            "executed=%llu ticks=%llu frame-width=%u frame-height=%u nonblack=%llu "
            "frame-digest=%016llx\n",
            (lib_u32)result.reason, result.detail, fault_code, (lib_u32)cop_failure.operation,
            (lib_u32)cop_failure.operation_value, (lib_u32)cop_failure.dma_source,
            (lib_u32)cop_failure.dma_size, (lib_u32)cop_failure.dma_destination,
            (lib_u32)cop_failure.dma_relative,
            cop_failure.physical,
            (lib_u32)cop_failure.status,
            (lib_u32)control_failure.valid, control_failure.physical,
            (lib_u32)control_failure.bytes, (lib_u32)control_failure.access,
            (unsigned long long)(MYRAIDENII_STARTUP_INSTRUCTION_BUDGET - remaining),
            (unsigned long long)result.elapsed_ticks, MYRAIDENII_CORE_VIDEO_FRAME_WIDTH,
            MYRAIDENII_CORE_VIDEO_FRAME_HEIGHT, (unsigned long long)nonblack_pixels,
            (unsigned long long)frame_digest);
        exit_code = 0;
    } else {
        lib_c_printf("MYRAIDENII:STARTUP-PROBE:FAIL status=%u\n", (lib_u32)status);
    }
    lib_release(pixels);
    myraidenii_core_board_destroy(board);
    return exit_code;
}
