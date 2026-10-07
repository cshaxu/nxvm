#include "core/driver.h"
#include "core/machine.h"
#include "core/snapshot_interface.h"
#include "lib/base/clock_interface.h"
#include "lib/storage/file_interface.h"
#include "lib/types/file.h"

enum { PERF_ROUNDS = 5, PERF_WARMUP_FRAMES = 60, PERF_MEASURED_FRAMES = 120 };

typedef struct performance_result {
    lib_u64 execution_units, conversion_units, cycles, instructions;
    lib_u64 frame_hash, audio_hash, state_hash;
} performance_result;

static void hash_bytes(lib_u64 *hash, const void *data, lib_size count)
{
    const lib_u8 *bytes = data;
    for (lib_size index = 0u; index < count; ++index)
        *hash = (*hash ^ bytes[index]) * UINT64_C(1099511628211);
}

static lib_status hash_state(void *context, const lib_u8 *bytes, lib_size count)
{
    hash_bytes(context, bytes, count);
    return LIB_STATUS_OK;
}

static lib_status clock_read(lib_u64 *units, lib_u64 *frequency)
{
    return base_clock_monotonic_counter(units, frequency);
}

static void hash_frame(lib_u64 *hash, const common_machine_frame *frame)
{
    if (frame->window.graphics) {
        hash_bytes(hash, frame->window.image.palette,
            sizeof(frame->window.image.palette));
        hash_bytes(hash, frame->window.image.pixels, CORE_PPU_WIDTH * CORE_PPU_HEIGHT);
    } else {
        hash_bytes(hash, frame->window.text.base.cells,
            80u * 25u * sizeof(frame->window.text.base.cells[0]));
        hash_bytes(hash, frame->characters.primary, sizeof(frame->characters.primary));
        hash_bytes(hash, frame->characters.secondary, sizeof(frame->characters.secondary));
    }
}

/* Timed intervals use the actual hardware and converter, without a host pacer.
 * PCM/state/frame fingerprints run outside them; no physical endpoint is used.
 * These are guest-computation costs, not native displayed FPS or CPU attribution. */
static lib_status measure_round(const lib_u8 *rom, lib_size rom_size,
    lib_bool text, performance_result *result, lib_u64 *frequency)
{
    core_machine *machine = LIB_NULL;
    common_machine_frame *frame = LIB_NULL;
    core_driver driver = {0};
    lib_status status;
    lib_u64 start_cycles = 0u, start_instructions = 0u;
    lib_u64 begin, end, rate;
    lib_i32 samples[CORE_APU_SAMPLE_CAPACITY];

    *result = (performance_result) {
        .frame_hash = UINT64_C(14695981039346656037),
        .audio_hash = UINT64_C(14695981039346656037),
        .state_hash = UINT64_C(14695981039346656037)
    };
    status = core_machine_create(&machine, rom, rom_size,
        &(core_machine_options){.initial_ram_byte = 0u});
    if (status != LIB_STATUS_OK) return status;
    frame = lib_allocate_zero(1u, sizeof(*frame));
    if (frame == LIB_NULL) { status = LIB_STATUS_NO_MEMORY; goto done; }
    driver.machine = machine;
    driver.text_output = text;
    if (clock_read(&begin, frequency) != LIB_STATUS_OK) {
        status = LIB_STATUS_IO_ERROR; goto done;
    }

    for (lib_u32 number = 0u; number < PERF_WARMUP_FRAMES + PERF_MEASURED_FRAMES;
            ++number) {
        const lib_bool measured = number >= PERF_WARMUP_FRAMES;
        const lib_u32 previous = machine->ppu.frame_revision;
        lib_u32 quanta = 0u;
        lib_u8 buttons = number >= 60u && number < 66u ? 0x08u : 0u;
        if (number >= 120u) buttons = (lib_u8)(0x82u |
            (number % 120u < 20u ? 1u : 0u));
        core_controller_set_buttons(&machine->controller, buttons);
        if (number == PERF_WARMUP_FRAMES) {
            start_cycles = machine->cycles;
            start_instructions = machine->instructions;
        }
        while (machine->ppu.frame_revision == previous) {
            core_run_result run = {0};
            lib_u16 count;
            if (++quanta > 1000u) { status = LIB_STATUS_LIMIT_EXCEEDED; goto done; }
            if (clock_read(&begin, &rate) != LIB_STATUS_OK || rate != *frequency) {
                status = LIB_STATUS_IO_ERROR; goto done;
            }
            status = core_machine_run(machine, 256u, 1024u, &run);
            if (clock_read(&end, &rate) != LIB_STATUS_OK || rate != *frequency ||
                    end < begin) { status = LIB_STATUS_IO_ERROR; goto done; }
            if (status != LIB_STATUS_OK || run.trap_valid) {
                lib_c_fprintf(lib_c_stderr, "core trap at frame=%u pc=%04x reason=%u\n",
                    (unsigned)number, (unsigned)machine->pc, (unsigned)run.reason);
                if (status == LIB_STATUS_OK) status = LIB_STATUS_UNSUPPORTED;
                goto done;
            }
            if (measured) result->execution_units += end - begin;
            count = core_apu_take_samples(&machine->apu, samples, CORE_APU_SAMPLE_CAPACITY);
            if (measured) hash_bytes(&result->audio_hash, samples,
                (lib_size)count * sizeof(samples[0]));
        }
        if (machine->ppu.frame_revision != previous + 1u ||
                machine->apu.dropped_samples != 0u) {
            status = LIB_STATUS_INVALID_STATE; goto done;
        }
        if (!measured) continue;
        if (clock_read(&begin, &rate) != LIB_STATUS_OK) {
            status = LIB_STATUS_IO_ERROR; goto done;
        }
        status = core_driver_copy_frame(&driver, frame);
        if (clock_read(&end, &rate) != LIB_STATUS_OK || rate != *frequency ||
                end < begin) { status = LIB_STATUS_IO_ERROR; goto done; }
        if (status != LIB_STATUS_OK || !frame->window.valid) {
            if (status == LIB_STATUS_OK) status = LIB_STATUS_INVALID_STATE;
            goto done;
        }
        result->conversion_units += end - begin;
        hash_frame(&result->frame_hash, frame);
    }
    result->cycles = machine->cycles - start_cycles;
    result->instructions = machine->instructions - start_instructions;
    status = core_snapshot_write(machine, hash_state, &result->state_hash);
done:
    lib_release(frame);
    core_machine_destroy(machine);
    return status;
}

static int measure_conversion(lib_bool text)
{
    core_machine *machine = lib_allocate_zero(1u, sizeof(*machine));
    common_machine_frame *frame = lib_allocate_zero(1u, sizeof(*frame));
    core_driver driver = {0};
    if (machine == LIB_NULL || frame == LIB_NULL) {
        lib_release(machine); lib_release(frame); return 1;
    }
    driver.machine = machine;
    driver.text_output = text;
    machine->ppu.frame_ready = LIB_TRUE;
    lib_c_printf("scope,mode,scene,round,frames,conversion_ms,frame_hash\n");
    for (lib_u32 scene = 0u; scene < 4u; ++scene) {
        lib_u64 expected = 0u;
        for (lib_u32 index = 0u; index < CORE_PPU_WIDTH * CORE_PPU_HEIGHT; ++index)
            machine->ppu.completed[index] = scene == 0u ? 0x20u :
                scene == 1u ? (lib_u16)((index / 8u) & 63u) :
                scene == 2u ? (lib_u16)((index * 73u + index / 256u * 17u) & 511u) :
                    (lib_u16)(0x800du | ((index & 7u) << 6u));
        for (lib_u32 round = 0u; round < PERF_ROUNDS; ++round) {
            lib_u64 begin, end, frequency, rate;
            lib_u64 hash = UINT64_C(14695981039346656037);
            if (clock_read(&begin, &frequency) != LIB_STATUS_OK) goto failure;
            for (lib_u32 number = 0u; number < 240u; ++number) {
                ++machine->ppu.frame_revision;
                if (core_driver_copy_frame(&driver, frame) != LIB_STATUS_OK ||
                        !frame->window.valid) goto failure;
            }
            if (clock_read(&end, &rate) != LIB_STATUS_OK || rate != frequency ||
                    end < begin) goto failure;
            hash_frame(&hash, frame);
            if (round != 0u && hash != expected) goto failure;
            expected = hash;
            lib_c_printf("conversion-only,%s,%u,%u,240,%.6f,%016llx\n",
                text ? "text" : "graphics", (unsigned)scene, (unsigned)round,
                (double)(end - begin) * 1000.0 / (double)frequency / 240.0,
                (unsigned long long)hash);
        }
    }
    lib_release(frame); lib_release(machine); return 0;
failure:
    lib_release(frame); lib_release(machine); return 1;
}

int main(int argc, char **argv)
{
    void *rom = LIB_NULL;
    lib_size size = 0u;
    performance_result reference = {0};
    lib_bool text;
    lib_status status;

    if (argc != 3 || (lib_text_compare(argv[2], "graphics") != 0 &&
            lib_text_compare(argv[2], "text") != 0)) return 2;
    text = lib_text_compare(argv[2], "text") == 0;
    if (lib_text_compare(argv[1], "--conversion") == 0)
        return measure_conversion(text);
    status = lib_storage_file_read_owned(argv[1], core_cartridge_maximum_image_bytes(),
        &rom, &size);
    if (status != LIB_STATUS_OK || !core_cartridge_normalize_ines_size(rom, &size)) {
        lib_release(rom); return 1;
    }
    lib_c_printf("scope,mode,round,frames,cycles,instructions,core_ms,conversion_ms,frame_hash,pcm_hash,state_hash\n");
    for (lib_u32 round = 0u; round < PERF_ROUNDS; ++round) {
        performance_result result;
        lib_u64 frequency;
        status = measure_round(rom, size, text, &result, &frequency);
        if (status != LIB_STATUS_OK) break;
        if (round != 0u && (result.cycles != reference.cycles ||
                result.instructions != reference.instructions ||
                result.frame_hash != reference.frame_hash ||
                result.audio_hash != reference.audio_hash ||
                result.state_hash != reference.state_hash)) {
            status = LIB_STATUS_INVALID_STATE; break;
        }
        reference = result;
        lib_c_printf("core+conversion,%s,%u,%u,%llu,%llu,%.6f,%.6f,%016llx,%016llx,%016llx\n",
            text ? "text" : "graphics", (unsigned)round, (unsigned)PERF_MEASURED_FRAMES,
            (unsigned long long)result.cycles, (unsigned long long)result.instructions,
            (double)result.execution_units * 1000.0 / (double)frequency / PERF_MEASURED_FRAMES,
            (double)result.conversion_units * 1000.0 / (double)frequency / PERF_MEASURED_FRAMES,
            (unsigned long long)result.frame_hash, (unsigned long long)result.audio_hash,
            (unsigned long long)result.state_hash);
    }
    lib_release(rom);
    if (status != LIB_STATUS_OK)
        lib_c_fprintf(lib_c_stderr, "performance probe rejected: status=%d\n", (int)status);
    return status == LIB_STATUS_OK ? 0 : 1;
}
