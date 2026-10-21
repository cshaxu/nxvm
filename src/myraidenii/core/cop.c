#include "myraidenii/core/cop.h"
#include "myraidenii/core/video.h"
#include "lib/types/math.h"
#include "x86-core/machine/machine_interface.h"

#define MYRAIDENII_CORE_COP_PI 3.14159265358979323846

enum {
    MYRAIDENII_CORE_COP_STORAGE_START = 0x00400u,
    MYRAIDENII_CORE_COP_STORAGE_BYTES = 0x001b6u,
    MYRAIDENII_CORE_COP_COLLISION_RESULT = 0x00580u,
    MYRAIDENII_CORE_COP_DECIMAL_RESULT = 0x00590u,
    MYRAIDENII_CORE_COP_DECIMAL_DIGITS = 9u,
    MYRAIDENII_CORE_COP_STATUS = 0x005b0u,
    MYRAIDENII_CORE_COP_DISTANCE = 0x005b2u,
    MYRAIDENII_CORE_COP_ANGLE = 0x005b4u,
    MYRAIDENII_CORE_COP_DMA_CONTEXTS = 512u,
    MYRAIDENII_CORE_COP_DMA_TILEMAP_MODE = 0x0014u,
    MYRAIDENII_CORE_COP_DMA_TILEMAP_SOURCE = 0x033fu,
    MYRAIDENII_CORE_COP_DMA_TILEMAP_SIZE = 0x027bu,
    MYRAIDENII_CORE_COP_DMA_TILEMAP_DESTINATION = 0xfffeu,
    MYRAIDENII_CORE_COP_DMA_PALETTE_MODE = 0x0015u,
    MYRAIDENII_CORE_COP_DMA_PALETTE_SOURCE = 0x07c0u,
    MYRAIDENII_CORE_COP_DMA_PALETTE_SIZE = 0x00ffu
};

typedef enum myraidenii_core_cop_register_role {
    MYRAIDENII_CORE_COP_CONFIG,
    MYRAIDENII_CORE_COP_DECIMAL_INPUT,
    MYRAIDENII_CORE_COP_READ_WRITE,
    MYRAIDENII_CORE_COP_COMMAND,
    MYRAIDENII_CORE_COP_RESULT,
    MYRAIDENII_CORE_COP_RANDOM,
    MYRAIDENII_CORE_COP_DMA_CONFIGURATION,
    MYRAIDENII_CORE_COP_SORT_CONFIGURATION,
    MYRAIDENII_CORE_COP_DMA_TRIGGER,
    MYRAIDENII_CORE_COP_SORT_TRIGGER,
    MYRAIDENII_CORE_COP_UNMODELLED_REGISTER
} myraidenii_core_cop_register_role;

typedef enum myraidenii_core_cop_command_kind {
    MYRAIDENII_CORE_COP_UNSUPPORTED,
    MYRAIDENII_CORE_COP_COPY,
    MYRAIDENII_CORE_COP_UPDATE,
    MYRAIDENII_CORE_COP_POSITION,
    MYRAIDENII_CORE_COP_HITBOX,
    MYRAIDENII_CORE_COP_ANGLE_APPROACH,
    MYRAIDENII_CORE_COP_NUMERIC,
    MYRAIDENII_CORE_COP_OFFSET_ADD
} myraidenii_core_cop_command_kind;

typedef struct myraidenii_core_cop_range {
    lib_u32 physical_start;
    lib_size bytes;
    myraidenii_core_cop_register_role role;
} myraidenii_core_cop_range;

typedef struct myraidenii_core_cop_collision {
    lib_i16 position[3];
    lib_i16 lower[3];
    lib_i16 upper[3];
} myraidenii_core_cop_collision;

typedef struct myraidenii_core_cop_dma_context {
    lib_u16 source;
    lib_u16 size;
    lib_u16 destination;
} myraidenii_core_cop_dma_context;

typedef enum myraidenii_core_cop_pending_kind {
    MYRAIDENII_CORE_COP_PENDING_NONE,
    MYRAIDENII_CORE_COP_PENDING_COMMAND,
    MYRAIDENII_CORE_COP_PENDING_DMA,
    MYRAIDENII_CORE_COP_PENDING_SORT
} myraidenii_core_cop_pending_kind;

typedef struct myraidenii_core_cop_pending {
    myraidenii_core_cop_pending_kind kind;
    lib_u16 command;
    lib_u16 mode;
    lib_u16 relative;
    lib_u16 count;
    lib_u8 slot;
} myraidenii_core_cop_pending;

typedef struct myraidenii_core_cop_sort_entry {
    lib_u16 offset;
    lib_i32 key;
} myraidenii_core_cop_sort_entry;

struct myraidenii_core_cop {
    x86_core_machine *machine;
    myraidenii_core_video *video;
    lib_u8 registers[MYRAIDENII_CORE_COP_STORAGE_BYTES];
    myraidenii_core_cop_collision collision[2];
    myraidenii_core_cop_dma_context dma_contexts[MYRAIDENII_CORE_COP_DMA_CONTEXTS];
    myraidenii_core_cop_pending pending;
    myraidenii_core_cop_failure_observation failure;
    myraidenii_core_cop_provider_failure_observation provider_failure;
};

static const myraidenii_core_cop_range myraidenii_core_cop_ranges[] = {
    {0x0041cu, 0x00004u, MYRAIDENII_CORE_COP_CONFIG},
    {0x00420u, 0x00004u, MYRAIDENII_CORE_COP_DECIMAL_INPUT},
    {0x00424u, 0x00002u, MYRAIDENII_CORE_COP_CONFIG},
    {0x00428u, 0x00006u, MYRAIDENII_CORE_COP_CONFIG},
    {0x00432u, 0x0000cu, MYRAIDENII_CORE_COP_CONFIG},
    {0x00444u, 0x00002u, MYRAIDENII_CORE_COP_CONFIG},
    {0x00450u, 0x0000au, MYRAIDENII_CORE_COP_SORT_CONFIGURATION},
    {0x0045au, 0x00004u, MYRAIDENII_CORE_COP_UNMODELLED_REGISTER},
    {0x00476u, 0x00008u, MYRAIDENII_CORE_COP_DMA_CONFIGURATION},
    {0x0047eu, 0x00002u, MYRAIDENII_CORE_COP_READ_WRITE},
    {0x004a0u, 0x0000eu, MYRAIDENII_CORE_COP_READ_WRITE},
    {0x004c0u, 0x0000eu, MYRAIDENII_CORE_COP_READ_WRITE},
    {0x00500u, 0x00006u, MYRAIDENII_CORE_COP_COMMAND},
    {MYRAIDENII_CORE_COP_COLLISION_RESULT, 10u, MYRAIDENII_CORE_COP_RESULT},
    {MYRAIDENII_CORE_COP_DECIMAL_RESULT,
        MYRAIDENII_CORE_COP_DECIMAL_DIGITS + 1u, MYRAIDENII_CORE_COP_RESULT},
    {0x005a0u, 0x00008u, MYRAIDENII_CORE_COP_RANDOM},
    {MYRAIDENII_CORE_COP_STATUS, 6u, MYRAIDENII_CORE_COP_RESULT},
    {0x006fcu, 0x00002u, MYRAIDENII_CORE_COP_DMA_TRIGGER},
    {0x006feu, 0x00002u, MYRAIDENII_CORE_COP_SORT_TRIGGER}
};

static myraidenii_core_cop_command_kind myraidenii_core_cop_classify(lib_u16 command)
{
    switch (command) {
    case 0x0205u: case 0x0905u: return MYRAIDENII_CORE_COP_UPDATE;
    case 0x2a05u: return MYRAIDENII_CORE_COP_OFFSET_ADD;
    case 0x5205u: case 0x5a05u: case 0xf205u: return MYRAIDENII_CORE_COP_COPY;
    case 0xa100u: case 0xa900u: return MYRAIDENII_CORE_COP_POSITION;
    case 0xb100u: case 0xb900u: return MYRAIDENII_CORE_COP_HITBOX;
    case 0x6200u: return MYRAIDENII_CORE_COP_ANGLE_APPROACH;
    case 0x130eu: case 0x138eu: case 0x2208u: case 0x330eu: case 0x338eu: case 0x3bb0u:
    case 0x42c2u: case 0x4aa0u: case 0x8100u: case 0x8900u:
        return MYRAIDENII_CORE_COP_NUMERIC;
    default: return MYRAIDENII_CORE_COP_UNSUPPORTED;
    }
}

static lib_u16 myraidenii_core_cop_word(const myraidenii_core_cop *cop,
    lib_u32 physical)
{
    lib_size index = physical - MYRAIDENII_CORE_COP_STORAGE_START;

    return (lib_u16)(cop->registers[index] | (lib_u16)cop->registers[index + 1u] << 8u);
}

static lib_u32 myraidenii_core_cop_address(const myraidenii_core_cop *cop,
    lib_u8 index)
{
    return (lib_u32)myraidenii_core_cop_word(cop, 0x4a0u + index * 2u) << 16u |
        myraidenii_core_cop_word(cop, 0x4c0u + index * 2u);
}

static lib_status myraidenii_core_cop_dma_context_get(myraidenii_core_cop *cop,
    myraidenii_core_cop_dma_context **out_context)
{
    const lib_u16 mode = myraidenii_core_cop_word(cop, 0x47eu);

    if (mode >= MYRAIDENII_CORE_COP_DMA_CONTEXTS) return LIB_STATUS_INVALID_STATE;
    *out_context = &cop->dma_contexts[mode];
    return LIB_STATUS_OK;
}

static lib_bool myraidenii_core_cop_dma_mode_supported(lib_u16 mode)
{
    return mode == 0x009u || mode == 0x00eu ||
        mode == MYRAIDENII_CORE_COP_DMA_TILEMAP_MODE ||
        mode == MYRAIDENII_CORE_COP_DMA_PALETTE_MODE || mode == 0x116u ||
        (mode >= 0x118u && mode <= 0x11fu);
}

static void myraidenii_core_cop_note_failure(myraidenii_core_cop *cop,
    lib_status status, lib_u32 physical);

static void myraidenii_core_cop_pending_clear(myraidenii_core_cop *cop)
{
    cop->pending = (myraidenii_core_cop_pending){0};
}

static void myraidenii_core_cop_provider_failure_note(void *owner,
    lib_u32 physical, lib_uptr bytes, x86_core_machine_memory_access access,
    lib_status status)
{
    myraidenii_core_cop *cop = owner;

    (void)physical;
    (void)bytes;
    if (cop == LIB_NULL || cop->provider_failure.valid) return;
    cop->provider_failure.valid = LIB_TRUE;
    cop->provider_failure.access = access;
    cop->provider_failure.status = status;
}

static void myraidenii_core_cop_decimal_update(myraidenii_core_cop *cop)
{
    lib_u32 number = myraidenii_core_cop_word(cop, 0x420u) |
        (lib_u32)myraidenii_core_cop_word(cop, 0x422u) << 16u;
    lib_u8 *digits = cop->registers +
        (MYRAIDENII_CORE_COP_DECIMAL_RESULT - MYRAIDENII_CORE_COP_STORAGE_START);
    lib_size used = 0u;

    do {
        digits[used++] = (lib_u8)('0' + number % 10u);
        number /= 10u;
    } while (number != 0u && used < MYRAIDENII_CORE_COP_DECIMAL_DIGITS);
    lib_memory_set(digits + used, myraidenii_core_cop_word(cop, 0x424u) == 3u ? '0' : ' ',
        MYRAIDENII_CORE_COP_DECIMAL_DIGITS - used);
    digits[MYRAIDENII_CORE_COP_DECIMAL_DIGITS] = 0u;
}

static const myraidenii_core_cop_range *myraidenii_core_cop_range_find(lib_u32 physical,
    lib_uptr bytes)
{
    lib_size index;

    if (bytes == 0u) return LIB_NULL;
    for (index = 0u; index < sizeof(myraidenii_core_cop_ranges) /
        sizeof(myraidenii_core_cop_ranges[0]); ++index) {
        const myraidenii_core_cop_range *range = &myraidenii_core_cop_ranges[index];

        if (physical >= range->physical_start &&
            physical - range->physical_start < range->bytes &&
            bytes <= range->bytes - (physical - range->physical_start) &&
            (bytes <= 2u || range->role == MYRAIDENII_CORE_COP_UNMODELLED_REGISTER))
            return range;
    }
    return LIB_NULL;
}

static lib_status myraidenii_core_cop_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    (void)access;
    if (owner == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    return myraidenii_core_cop_range_find(physical, bytes) == LIB_NULL ?
        LIB_STATUS_UNSUPPORTED : LIB_STATUS_OK;
}

static lib_status myraidenii_core_cop_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    const myraidenii_core_cop *cop = owner;
    const myraidenii_core_cop_range *range;

    (void)observe_only;
    if (owner == LIB_NULL || destination == 0u || bytes == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    range = myraidenii_core_cop_range_find(physical, bytes);
    if (range == LIB_NULL) return LIB_STATUS_UNSUPPORTED;
    if (range->role == MYRAIDENII_CORE_COP_READ_WRITE) {
        lib_memory_copy((void *)destination,
            cop->registers + (physical - MYRAIDENII_CORE_COP_STORAGE_START), bytes);
        return LIB_STATUS_OK;
    }
    if (range->role == MYRAIDENII_CORE_COP_RANDOM) {
        x86_core_machine_timeline_observation time;
        lib_status status = x86_core_machine_get_timeline_observation(cop->machine, &time);
        lib_u32 divisor = (lib_u32)myraidenii_core_cop_word(cop, 0x42cu) + 1u;
        lib_u16 value;

        if (status != LIB_STATUS_OK) return status;
        /* Nominal L2 RNG: observe the one guest timeline without advancing it. */
        value = (lib_u16)(time.now % divisor);
        for (lib_uptr index = 0u; index < bytes; ++index)
            ((lib_u8 *)destination)[index] =
                (lib_u8)(value >> (((physical + (lib_u32)index) & 1u) * 8u));
        return LIB_STATUS_OK;
    }
    if (range->role == MYRAIDENII_CORE_COP_RESULT) {
        for (lib_uptr index = 0u; index < bytes; ++index) {
            lib_u32 address = physical + (lib_u32)index;

            /* 588h is a second view of the one L2 status word at 580h. */
            if (address >= MYRAIDENII_CORE_COP_COLLISION_RESULT + 8u &&
                address < MYRAIDENII_CORE_COP_COLLISION_RESULT + 10u)
                address -= 8u;
            ((lib_u8 *)destination)[index] =
                cop->registers[address - MYRAIDENII_CORE_COP_STORAGE_START];
        }
        return LIB_STATUS_OK;
    }
    if (range->role == MYRAIDENII_CORE_COP_COMMAND) {
        /* L2: the external map establishes write-only decoding, not a physical
         * open-bus value. Keep the one admitted provider deterministic without
         * inventing a command-result latch or changing pending state. */
        lib_memory_set((void *)destination, 0, bytes);
        return LIB_STATUS_OK;
    }
    /* Unmodelled result/configuration reads remain explicit failures. */
    return LIB_STATUS_INVALID_STATE;
}

static lib_status myraidenii_core_cop_dma_configuration_write(myraidenii_core_cop *cop,
    lib_u32 physical, const lib_u8 *source, lib_uptr bytes)
{
    myraidenii_core_cop_dma_context *context = LIB_NULL;
    myraidenii_core_cop_dma_context candidate;
    lib_bool changes_context = physical < 0x47eu && physical + bytes > 0x478u;

    if (changes_context) {
        lib_status status = myraidenii_core_cop_dma_context_get(cop, &context);

        if (status != LIB_STATUS_OK) return status;
        candidate = *context;
        for (lib_uptr index = 0u; index < bytes; ++index) {
            const lib_u32 current = physical + (lib_u32)index;
            lib_u16 *word;

            if (current < 0x478u || current > 0x47du) continue;
            word = current < 0x47au ? &candidate.source :
                current < 0x47cu ? &candidate.size : &candidate.destination;
            *word = (lib_u16)((*word & ~(0xffu << ((current & 1u) * 8u))) |
                (lib_u16)source[index] << ((current & 1u) * 8u));
        }
        *context = candidate;
    }
    for (lib_uptr index = 0u; index < bytes; ++index) {
        const lib_u32 current = physical + (lib_u32)index;

        if (current >= 0x478u && current <= 0x47du) continue;
        cop->registers[current - MYRAIDENII_CORE_COP_STORAGE_START] = source[index];
    }
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_cop_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    myraidenii_core_cop *cop = owner;
    const myraidenii_core_cop_range *range;

    if (cop == LIB_NULL || source == 0u || bytes == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    range = myraidenii_core_cop_range_find(physical, bytes);
    if (range == LIB_NULL) return LIB_STATUS_UNSUPPORTED;
    if (range->role == MYRAIDENII_CORE_COP_RESULT ||
        range->role == MYRAIDENII_CORE_COP_RANDOM ||
        range->role == MYRAIDENII_CORE_COP_UNMODELLED_REGISTER)
        return LIB_STATUS_INVALID_STATE;
    if (cop->pending.kind != MYRAIDENII_CORE_COP_PENDING_NONE) {
        if (range->role == MYRAIDENII_CORE_COP_COMMAND && bytes == 2u &&
            (physical & 1u) == 0u && !cop->failure.valid) {
            const lib_u8 *data = (const lib_u8 *)source;

            cop->failure.valid = LIB_TRUE;
            cop->failure.operation = MYRAIDENII_CORE_COP_OPERATION_COMMAND;
            cop->failure.operation_value = (lib_u16)(data[0] | (lib_u16)data[1] << 8u);
            cop->failure.physical = physical;
            cop->failure.status = LIB_STATUS_INVALID_STATE;
        }
        return LIB_STATUS_INVALID_STATE;
    }
    if (range->role == MYRAIDENII_CORE_COP_COMMAND) {
        const lib_u8 *data = (const lib_u8 *)source;
        lib_u16 command;

        if (bytes != 2u || (physical & 1u) != 0u) return LIB_STATUS_INVALID_STATE;
        command = (lib_u16)(data[0] | (lib_u16)data[1] << 8u);
        if (myraidenii_core_cop_classify(command) == MYRAIDENII_CORE_COP_UNSUPPORTED)
            return LIB_STATUS_INVALID_STATE;
        cop->pending.kind = MYRAIDENII_CORE_COP_PENDING_COMMAND;
        cop->pending.command = command;
        cop->pending.slot = (lib_u8)((physical - 0x500u) / 2u);
        return LIB_STATUS_OK;
    }
    if (range->role == MYRAIDENII_CORE_COP_DMA_TRIGGER ||
        range->role == MYRAIDENII_CORE_COP_SORT_TRIGGER) {
        const lib_u8 *data = (const lib_u8 *)source;
        lib_u16 value;

        if (bytes != 2u || (physical & 1u) != 0u) return LIB_STATUS_INVALID_STATE;
        value = (lib_u16)(data[0] | (lib_u16)data[1] << 8u);
        if (range->role == MYRAIDENII_CORE_COP_DMA_TRIGGER) {
            const lib_u16 mode = myraidenii_core_cop_word(cop, 0x47eu);

            if (!myraidenii_core_cop_dma_mode_supported(mode)) {
                if (!cop->failure.valid) {
                    myraidenii_core_cop_dma_context *context = LIB_NULL;

                    cop->failure.operation = MYRAIDENII_CORE_COP_OPERATION_DMA;
                    cop->failure.operation_value = mode;
                    if (myraidenii_core_cop_dma_context_get(cop, &context) == LIB_STATUS_OK) {
                        cop->failure.dma_source = context->source;
                        cop->failure.dma_size = context->size;
                        cop->failure.dma_destination = context->destination;
                        cop->failure.dma_relative = myraidenii_core_cop_word(cop, 0x476u);
                    }
                    myraidenii_core_cop_note_failure(cop, LIB_STATUS_INVALID_STATE, physical);
                }
                return LIB_STATUS_INVALID_STATE;
            }
            cop->pending.kind = MYRAIDENII_CORE_COP_PENDING_DMA;
            cop->pending.mode = mode;
            cop->pending.relative = myraidenii_core_cop_word(cop, 0x476u);
        } else {
            cop->pending.kind = MYRAIDENII_CORE_COP_PENDING_SORT;
            cop->pending.count = value;
        }
        return LIB_STATUS_OK;
    }
    if (range->role == MYRAIDENII_CORE_COP_DMA_CONFIGURATION)
        return myraidenii_core_cop_dma_configuration_write(cop, physical,
            (const lib_u8 *)source, bytes);
    for (lib_uptr index = 0u; index < bytes; ++index) {
        lib_u32 current = physical + (lib_u32)index;

        cop->registers[current - MYRAIDENII_CORE_COP_STORAGE_START] =
            ((const lib_u8 *)source)[index];
    }
    if (range->role == MYRAIDENII_CORE_COP_DECIMAL_INPUT)
        myraidenii_core_cop_decimal_update(cop);
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_cop_create(myraidenii_core_video *video,
    myraidenii_core_cop **out_cop)
{
    myraidenii_core_cop *cop;

    if (video == LIB_NULL || out_cop == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_cop = LIB_NULL;
    cop = lib_allocate_zero(1u, sizeof(*cop));
    if (cop == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    cop->video = video;
    *out_cop = cop;
    return LIB_STATUS_OK;
}

void myraidenii_core_cop_destroy(myraidenii_core_cop *cop)
{
    lib_release(cop);
}

void myraidenii_core_cop_reset(myraidenii_core_cop *cop)
{
    if (cop != LIB_NULL) {
        lib_memory_set(cop->registers, 0, sizeof(cop->registers));
        lib_memory_set(cop->collision, 0, sizeof(cop->collision));
        lib_memory_set(cop->dma_contexts, 0, sizeof(cop->dma_contexts));
        lib_memory_set(&cop->failure, 0, sizeof(cop->failure));
        lib_memory_set(&cop->provider_failure, 0, sizeof(cop->provider_failure));
        myraidenii_core_cop_pending_clear(cop);
    }
}

static void myraidenii_core_cop_note_failure(myraidenii_core_cop *cop,
    lib_status status, lib_u32 physical)
{
    if (!cop->failure.valid) {
        cop->failure.valid = LIB_TRUE;
        cop->failure.status = status;
        cop->failure.physical = physical;
    }
}

static lib_status myraidenii_core_cop_memory(myraidenii_core_cop *cop,
    lib_u32 base, lib_u32 displacement, lib_u8 bytes, lib_bool write, lib_u32 *value)
{
    lib_u32 address;
    lib_u32 result = 0u;
    const lib_u8 width = bytes == 1u ? 1u : 2u;

    if (base > LIB_UINT32_MAX - displacement) {
        myraidenii_core_cop_note_failure(cop, LIB_STATUS_INVALID_ARGUMENT, base);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    address = base + displacement;
    for (lib_u8 index = 0u; index < bytes; index += width) {
        lib_u16 lane = write ? (lib_u16)(*value >> (index * 8u)) : 0u;
        lib_status status;

        if (address > LIB_UINT32_MAX - index) {
            myraidenii_core_cop_note_failure(cop, LIB_STATUS_INVALID_ARGUMENT, address);
            return LIB_STATUS_INVALID_ARGUMENT;
        }
        status = x86_core_machine_dma_memory_cycle(cop->machine, address + index,
            width, 0u, write ? X86_CORE_MACHINE_MEMORY_ACCESS_WRITE :
                X86_CORE_MACHINE_MEMORY_ACCESS_READ, &lane, LIB_NULL, LIB_NULL, cop);
        if (status != LIB_STATUS_OK) {
            myraidenii_core_cop_note_failure(cop, status, address + index);
            return status;
        }
        result |= (lib_u32)lane << (index * 8u);
    }
    if (!write) *value = result;
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_cop_approach_angle(myraidenii_core_cop *cop)
{
    const lib_u32 base = myraidenii_core_cop_address(cop, 0u);
    const lib_u8 target = (lib_u8)myraidenii_core_cop_word(cop, 0x41cu);
    const lib_u8 step = (lib_u8)myraidenii_core_cop_word(cop, 0x41eu);
    lib_u32 angle = 0u, flags = 0u, forward, distance;
    lib_status status = myraidenii_core_cop_memory(cop, base, 0x34u, 1u,
        LIB_FALSE, &angle);

    if (status == LIB_STATUS_OK)
        status = myraidenii_core_cop_memory(cop, base, 0u, 2u, LIB_FALSE, &flags);
    if (status != LIB_STATUS_OK) return status;
    forward = (target + 256u - angle) & 0xffu;
    distance = forward <= 128u ? forward : 256u - forward;
    flags &= ~4u;
    if (distance <= step) {
        angle = target;
        flags |= 4u;
    } else {
        angle = (angle + (forward <= 128u ? step : 256u - step)) & 0xffu;
    }
    status = myraidenii_core_cop_memory(cop, base, 0u, 2u, LIB_TRUE, &flags);
    return status == LIB_STATUS_OK ?
        myraidenii_core_cop_memory(cop, base, 0x34u, 1u, LIB_TRUE, &angle) : status;
}

static lib_status myraidenii_core_cop_update(myraidenii_core_cop *cop)
{
    const lib_u32 base = myraidenii_core_cop_address(cop, 0u);
    const lib_u32 slot = cop->pending.slot * 4u;
    const lib_u32 position_offset = cop->pending.command == 0x0205u ? 4u : 16u;
    const lib_u32 increment_offset = cop->pending.command == 0x0205u ? 16u : 40u;
    lib_u32 previous = 0u, increment = 0u, next, coordinate = 0u;
    lib_status status = myraidenii_core_cop_memory(cop, base,
        position_offset + slot, 4u, LIB_FALSE, &previous);

    if (status == LIB_STATUS_OK)
        status = myraidenii_core_cop_memory(cop, base,
            increment_offset + slot, 4u, LIB_FALSE, &increment);
    if (status != LIB_STATUS_OK) return status;
    next = previous + increment;
    status = myraidenii_core_cop_memory(cop, base,
        position_offset + slot, 4u, LIB_TRUE, &next);
    if (status != LIB_STATUS_OK || cop->pending.command != 0x0205u) return status;
    status = myraidenii_core_cop_memory(cop, base, 30u + slot, 2u, LIB_FALSE, &coordinate);
    if (status != LIB_STATUS_OK) return status;
    coordinate = (lib_u16)(coordinate + (next >> 16u) - (previous >> 16u));
    return myraidenii_core_cop_memory(cop, base, 30u + slot, 2u, LIB_TRUE, &coordinate);
}

static lib_status myraidenii_core_cop_offset_add(myraidenii_core_cop *cop)
{
    const lib_u32 primary = myraidenii_core_cop_address(cop, 0u);
    const lib_u32 secondary = myraidenii_core_cop_address(cop, 1u);
    const lib_u32 slot = cop->pending.slot * 4u;
    lib_u32 delta = 0u, value = 0u;

    lib_status status = myraidenii_core_cop_memory(cop, secondary, 0x1eu + slot,
        2u, LIB_FALSE, &delta);
    if (status == LIB_STATUS_OK)
        status = myraidenii_core_cop_memory(cop, primary, 6u + slot, 2u,
            LIB_FALSE, &value);
    if (status == LIB_STATUS_OK) {
        value += delta;
        status = myraidenii_core_cop_memory(cop, primary, 6u + slot, 4u,
            LIB_TRUE, &value);
    }
    if (status == LIB_STATUS_OK)
        status = myraidenii_core_cop_memory(cop, primary, 0x1eu + slot, 2u,
            LIB_FALSE, &value);
    if (status == LIB_STATUS_OK) {
        value += delta;
        status = myraidenii_core_cop_memory(cop, primary, 0x1eu + slot, 4u,
            LIB_TRUE, &value);
    }
    return status;
}

static lib_i16 myraidenii_core_cop_signed_word(lib_u32 value)
{
    lib_u16 word = (lib_u16)value;

    return (lib_i16)((word & 0x8000u) != 0u ? (lib_i32)word - 0x10000 : (lib_i32)word);
}

static lib_status myraidenii_core_cop_capture_position(myraidenii_core_cop *cop)
{
    const lib_u8 slot = cop->pending.command == 0xa900u ? 1u : 0u;
    const lib_u32 base = myraidenii_core_cop_address(cop, slot);
    lib_i16 position[3];
    lib_u32 word = 0u;
    /* Selected non-flipping forms still consume the object's flags word. */
    lib_status status = myraidenii_core_cop_memory(cop, base, 2u, 2u, LIB_FALSE, &word);

    for (lib_u8 axis = 0u; axis < 3u && status == LIB_STATUS_OK; ++axis) {
        status = myraidenii_core_cop_memory(cop, base, 6u + axis * 4u, 2u, LIB_FALSE, &word);
        position[axis] = myraidenii_core_cop_signed_word(word);
    }
    if (status == LIB_STATUS_OK)
        lib_memory_copy(cop->collision[slot].position, position, sizeof(position));
    return status;
}

static void myraidenii_core_cop_store_word(myraidenii_core_cop *cop,
    lib_u32 physical, lib_u16 word)
{
    const lib_size index = physical - MYRAIDENII_CORE_COP_STORAGE_START;

    cop->registers[index] = (lib_u8)word;
    cop->registers[index + 1u] = (lib_u8)(word >> 8u);
}

static lib_status myraidenii_core_cop_coordinates(myraidenii_core_cop *cop,
    lib_u32 difference[2])
{
    for (lib_u8 axis = 0u; axis < 2u; ++axis) {
        lib_u32 first = 0u, second = 0u;
        lib_status status = myraidenii_core_cop_memory(cop,
            myraidenii_core_cop_address(cop, 1u), 4u + axis * 4u, 4u, LIB_FALSE, &second);

        if (status == LIB_STATUS_OK)
            status = myraidenii_core_cop_memory(cop,
                myraidenii_core_cop_address(cop, 0u), 4u + axis * 4u, 4u, LIB_FALSE, &first);
        if (status != LIB_STATUS_OK) return status;
        difference[axis] = second - first;
    }
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_cop_numeric(myraidenii_core_cop *cop)
{
    const lib_u16 command = cop->pending.command;
    const lib_u32 base = myraidenii_core_cop_address(cop, 0u);
    const lib_u8 scale = (lib_u8)(myraidenii_core_cop_word(cop, 0x444u) & 3u);
    lib_u32 value = 0u, output = 0u;
    lib_status status;

    if (command == 0x130eu || command == 0x138eu || command == 0x2208u ||
        command == 0x330eu || command == 0x338eu) {
        lib_i64 x, y;
        lib_u32 operands[2] = {0u, 0u};
        lib_u16 result_status = command == 0x2208u ?
            myraidenii_core_cop_word(cop, MYRAIDENII_CORE_COP_STATUS) : 7u;

        if (command == 0x2208u) {
            status = myraidenii_core_cop_memory(cop, base, 0x12u, 2u,
                LIB_FALSE, &operands[0]);
            if (status == LIB_STATUS_OK)
                status = myraidenii_core_cop_memory(cop, base, 0x16u, 2u,
                    LIB_FALSE, &operands[1]);
            x = operands[0];
            y = operands[1];
        } else {
            status = myraidenii_core_cop_coordinates(cop, operands);
            /* Explicit L2 modulo32 subtraction and signed interpretation. */
            x = operands[0] <= LIB_INT32_MAX ? (lib_i64)operands[0] :
                (lib_i64)operands[0] - 0x100000000LL;
            y = operands[1] <= LIB_INT32_MAX ? (lib_i64)operands[1] :
                (lib_i64)operands[1] - 0x100000000LL;
        }
        if (status != LIB_STATUS_OK) return status;
        if (y == 0) {
            result_status |= 0x8000u;
        } else {
            /* Commit to double before integer truncation on x87 as well as SSE. */
            volatile double units = lib_c_atan((double)x / (double)y) *
                128.0 / MYRAIDENII_CORE_COP_PI;
            lib_i32 angle = (lib_i32)units;

            if (y < 0) angle += 128;
            value = (lib_u8)angle;
        }
        myraidenii_core_cop_store_word(cop, MYRAIDENII_CORE_COP_STATUS, result_status);
        myraidenii_core_cop_store_word(cop, MYRAIDENII_CORE_COP_ANGLE, (lib_u16)value);
        return command == 0x138eu || command == 0x338eu ?
            myraidenii_core_cop_memory(cop, base, 0x34u, 1u, LIB_TRUE, &value) : LIB_STATUS_OK;
    }
    if (command == 0x3bb0u) {
        lib_u32 difference[2] = {0u, 0u};

        status = myraidenii_core_cop_coordinates(cop, difference);
        if (status != LIB_STATUS_OK) return status;
        /* Live-read L2 integer parts, with wide squares instead of signed overflow. */
        const lib_i64 x = myraidenii_core_cop_signed_word(difference[0] >> 16u);
        const lib_i64 y = myraidenii_core_cop_signed_word(difference[1] >> 16u);

        value = (lib_u16)lib_c_sqrt((double)(x * x + y * y));
        myraidenii_core_cop_store_word(cop, MYRAIDENII_CORE_COP_DISTANCE, (lib_u16)value);
        return myraidenii_core_cop_memory(cop, base, 0x3au, 2u, LIB_TRUE, &value);
    }
    if (command == 0x42c2u || command == 0x4aa0u) {
        const lib_bool forward = command == 0x42c2u;

        status = myraidenii_core_cop_memory(cop, base, forward ? 0x36u : 0x38u,
            2u, LIB_FALSE, &value);
        if (status != LIB_STATUS_OK) return status;
        if (value == 0u && forward) {
            myraidenii_core_cop_store_word(cop, MYRAIDENII_CORE_COP_STATUS,
                myraidenii_core_cop_word(cop, MYRAIDENII_CORE_COP_STATUS) | 0x8000u);
        } else {
            if (value == 0u) value = 1u;
            output = ((lib_u32)myraidenii_core_cop_word(cop,
                MYRAIDENII_CORE_COP_DISTANCE) << (5u - scale)) / value;
            myraidenii_core_cop_store_word(cop, MYRAIDENII_CORE_COP_STATUS, 7u);
        }
        return myraidenii_core_cop_memory(cop, base, forward ? 0x38u : 0x36u,
            2u, LIB_TRUE, &output);
    }
    lib_u32 amplitude = 0u;

    status = myraidenii_core_cop_memory(cop, base, 0x34u, 2u, LIB_FALSE, &value);
    if (status == LIB_STATUS_OK)
        status = myraidenii_core_cop_memory(cop, base, 0x36u, 2u, LIB_FALSE, &amplitude);
    if (status != LIB_STATUS_OK) return status;
    const lib_u8 angle = (lib_u8)value;
    const double radians = angle * MYRAIDENII_CORE_COP_PI / 128.0;
    const lib_bool sine = command == 0x8100u;
    lib_i32 magnitude = (lib_i32)(amplitude & 0xffu) * 2048;

    /* Reference-derived axial quirks are explicit L2, not sine/cosine identities. */
    if (angle == (sine ? 0xc0u : 0x80u)) magnitude *= 2;
    volatile double units = magnitude * (sine ? lib_c_sin(radians) : lib_c_cos(radians));
    const lib_i32 component = (lib_i32)units;

    output = (lib_u32)(component * (1 << scale));
    return myraidenii_core_cop_memory(cop, base, sine ? 0x10u : 0x14u,
        4u, LIB_TRUE, &output);
}

static lib_status myraidenii_core_cop_capture_hitbox(myraidenii_core_cop *cop)
{
    const lib_u8 slot = cop->pending.command == 0xb900u ? 1u : 0u;
    myraidenii_core_cop_collision candidate = cop->collision[slot];
    lib_u32 pointer = 0u;
    lib_status status = myraidenii_core_cop_memory(cop,
        myraidenii_core_cop_address(cop, (lib_u8)(2u + slot)), 0u, 2u, LIB_FALSE, &pointer);
    lib_u16 separated = 7u;

    if (status != LIB_STATUS_OK) return status;
    pointer |= (lib_u32)myraidenii_core_cop_word(cop, 0x436u) << 16u;
    for (lib_u8 axis = 0u; axis < 3u; ++axis) {
        lib_u32 pair = 0u;
        lib_i32 displacement, lower;

        status = myraidenii_core_cop_memory(cop, pointer, axis * 2u, 2u, LIB_FALSE, &pair);
        if (status != LIB_STATUS_OK) return status;
        displacement = (lib_i32)(pair & 0xffu);
        if (displacement >= 0x80) displacement -= 0x100;
        lower = candidate.position[axis] + displacement;
        candidate.lower[axis] = myraidenii_core_cop_signed_word((lib_u32)lower);
        candidate.upper[axis] = myraidenii_core_cop_signed_word(
            (lib_u32)(lower + (lib_i32)(pair >> 8u)));
    }
    cop->collision[slot] = candidate;
    for (lib_u8 axis = 0u; axis < 3u; ++axis) {
        const myraidenii_core_cop_collision *first = &cop->collision[0];
        const myraidenii_core_cop_collision *second = &cop->collision[1];

        if (first->upper[axis] > second->lower[axis] &&
            first->lower[axis] < second->upper[axis])
            separated &= (lib_u16)~(1u << axis);
        myraidenii_core_cop_store_word(cop, MYRAIDENII_CORE_COP_COLLISION_RESULT + 2u + axis * 2u,
            (lib_u16)((lib_i32)first->position[axis] - second->position[axis]));
    }
    myraidenii_core_cop_store_word(cop, MYRAIDENII_CORE_COP_COLLISION_RESULT, separated);
    return LIB_STATUS_OK;
}

static lib_i32 myraidenii_core_cop_signed_dword(lib_u32 value)
{
    return value <= LIB_INT32_MAX ? (lib_i32)value :
        (lib_i32)((lib_i64)value - 0x100000000LL);
}

static lib_status myraidenii_core_cop_dma_copy(myraidenii_core_cop *cop,
    const myraidenii_core_cop_dma_context *context)
{
    const lib_u32 source = (lib_u32)context->source * 64u;
    const lib_u32 destination = (lib_u32)context->destination * 64u;
    const lib_u32 end = ((lib_u32)context->size + 1u) * 32u;

    if (end < destination) return LIB_STATUS_INVALID_ARGUMENT;
    /* Reference-derived L2: forward words preserve the observed overlap order. */
    for (lib_u32 index = 0u; index < end - destination; index += 2u) {
        lib_u32 value = 0u;
        lib_status status = myraidenii_core_cop_memory(cop, source, index, 2u,
            LIB_FALSE, &value);

        if (status != LIB_STATUS_OK) return status;
        status = myraidenii_core_cop_memory(cop, destination, index, 2u,
            LIB_TRUE, &value);
        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_cop_dma_fill(myraidenii_core_cop *cop,
    const myraidenii_core_cop_dma_context *context, lib_u16 mode)
{
    const lib_u32 destination = (lib_u32)context->source * 64u;
    const lib_u32 value = myraidenii_core_cop_word(cop, 0x428u) |
        (lib_u32)myraidenii_core_cop_word(cop, 0x42au) << 16u;
    const lib_u32 bytes = ((lib_u32)context->size + 1u) *
        (mode == 0x116u ? 16u : 32u);

    if (mode != 0x116u && context->destination != 0u) return LIB_STATUS_OK;
    for (lib_u32 index = 0u; index < bytes; index += 4u) {
        lib_u32 write = value;
        lib_status status = myraidenii_core_cop_memory(cop, destination, index, 4u,
            LIB_TRUE, &write);

        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_cop_dma_palette(myraidenii_core_cop *cop,
    const myraidenii_core_cop_dma_context *context)
{
    lib_u16 words[MYRAIDENII_CORE_VIDEO_PALETTE_WORDS];
    const lib_u32 source = (lib_u32)context->source * 64u;

    if (context->source != MYRAIDENII_CORE_COP_DMA_PALETTE_SOURCE ||
        context->size != MYRAIDENII_CORE_COP_DMA_PALETTE_SIZE ||
        context->destination != 0u) {
        return LIB_STATUS_INVALID_STATE;
    }
    for (lib_size index = 0u; index < MYRAIDENII_CORE_VIDEO_PALETTE_WORDS; ++index) {
        lib_u32 value = 0u;
        lib_status status = myraidenii_core_cop_memory(cop, source, (lib_u32)index * 2u,
            2u, LIB_FALSE, &value);

        if (status != LIB_STATUS_OK) return status;
        words[index] = (lib_u16)value;
    }
    return myraidenii_core_video_palette_load(cop->video, words,
        MYRAIDENII_CORE_VIDEO_PALETTE_WORDS);
}

static lib_status myraidenii_core_cop_dma_tilemap(myraidenii_core_cop *cop,
    const myraidenii_core_cop_dma_context *context)
{
    lib_u16 words[MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS];
    const lib_u32 source = (lib_u32)context->source * 64u;

    if (context->source != MYRAIDENII_CORE_COP_DMA_TILEMAP_SOURCE ||
        context->size != MYRAIDENII_CORE_COP_DMA_TILEMAP_SIZE ||
        context->destination != MYRAIDENII_CORE_COP_DMA_TILEMAP_DESTINATION ||
        cop->pending.relative != 0u) {
        return LIB_STATUS_INVALID_STATE;
    }
    for (lib_size index = 0u; index < MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS; ++index) {
        lib_u32 value = 0u;
        lib_status status = myraidenii_core_cop_memory(cop, source, (lib_u32)index * 2u,
            2u, LIB_FALSE, &value);

        if (status != LIB_STATUS_OK) return status;
        words[index] = (lib_u16)value;
    }
    return myraidenii_core_video_tilemap_load(cop->video, words,
        MYRAIDENII_CORE_VIDEO_TILEMAP_WORDS);
}

static lib_status myraidenii_core_cop_dma(myraidenii_core_cop *cop)
{
    const myraidenii_core_cop_dma_context *context =
        &cop->dma_contexts[cop->pending.mode];

    if (cop->pending.mode == 0x009u || cop->pending.mode == 0x00eu)
        return myraidenii_core_cop_dma_copy(cop, context);
    if (cop->pending.mode == MYRAIDENII_CORE_COP_DMA_TILEMAP_MODE)
        return myraidenii_core_cop_dma_tilemap(cop, context);
    if (cop->pending.mode == MYRAIDENII_CORE_COP_DMA_PALETTE_MODE)
        return myraidenii_core_cop_dma_palette(cop, context);
    return myraidenii_core_cop_dma_fill(cop, context, cop->pending.mode);
}

static void myraidenii_core_cop_sort_stable(myraidenii_core_cop_sort_entry *entries,
    myraidenii_core_cop_sort_entry *scratch, lib_u16 count, lib_bool descending)
{
    for (lib_size width = 1u; width < count; width *= 2u) {
        for (lib_size first = 0u; first < count; first += width * 2u) {
            const lib_size middle = first + width < count ? first + width : count;
            const lib_size last = middle + width < count ? middle + width : count;
            lib_size left = first, right = middle, output = first;

            while (left < middle && right < last) {
                const lib_bool take_left = descending ? entries[left].key >= entries[right].key :
                    entries[left].key <= entries[right].key;

                scratch[output++] = take_left ? entries[left++] : entries[right++];
            }
            while (left < middle) scratch[output++] = entries[left++];
            while (right < last) scratch[output++] = entries[right++];
            for (output = first; output < last; ++output) entries[output] = scratch[output];
        }
        if (width > count / 2u) break;
    }
}

static lib_status myraidenii_core_cop_sort(myraidenii_core_cop *cop)
{
    const lib_u32 object_base = (lib_u32)myraidenii_core_cop_word(cop, 0x450u) << 16u |
        myraidenii_core_cop_word(cop, 0x452u);
    const lib_u32 lookup_base = (lib_u32)myraidenii_core_cop_word(cop, 0x454u) << 16u |
        myraidenii_core_cop_word(cop, 0x456u);
    const lib_u16 parameter = myraidenii_core_cop_word(cop, 0x458u);
    const lib_u16 count = cop->pending.count;
    myraidenii_core_cop_sort_entry *entries;
    myraidenii_core_cop_sort_entry *scratch;
    lib_status status = LIB_STATUS_OK;

    if (count == 0u) return LIB_STATUS_OK;
    entries = lib_allocate((lib_size)count * sizeof(*entries) * 2u);
    if (entries == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    scratch = entries + count;
    for (lib_size index = 0u; index < count && status == LIB_STATUS_OK; ++index) {
        lib_u32 value = 0u;

        status = myraidenii_core_cop_memory(cop, lookup_base, (lib_u32)index * 2u,
            2u, LIB_FALSE, &value);
        entries[index].offset = (lib_u16)value;
        if (status == LIB_STATUS_OK)
            status = myraidenii_core_cop_memory(cop, object_base, entries[index].offset,
                4u, LIB_FALSE, &value);
        entries[index].key = myraidenii_core_cop_signed_dword(value);
    }
    if (status == LIB_STATUS_OK && (parameter == 1u || parameter == 2u))
        myraidenii_core_cop_sort_stable(entries, scratch, count, parameter == 2u);
    for (lib_size index = 0u; index < count && status == LIB_STATUS_OK; ++index) {
        lib_u32 value = entries[index].offset;

        status = myraidenii_core_cop_memory(cop, lookup_base, (lib_u32)index * 2u,
            2u, LIB_TRUE, &value);
    }
    lib_release(entries);
    return status;
}

lib_status myraidenii_core_cop_advance(myraidenii_core_cop *cop)
{
    myraidenii_core_cop_command_kind kind;
    lib_u32 value = 0u;
    lib_u32 source;
    lib_u32 destination;
    lib_status status = LIB_STATUS_OK;

    if (cop == LIB_NULL || cop->machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (cop->pending.kind == MYRAIDENII_CORE_COP_PENDING_NONE) return LIB_STATUS_OK;
    lib_memory_set(&cop->failure, 0, sizeof(cop->failure));
    if (cop->pending.kind == MYRAIDENII_CORE_COP_PENDING_COMMAND) {
        cop->failure.operation = MYRAIDENII_CORE_COP_OPERATION_COMMAND;
        cop->failure.operation_value = cop->pending.command;
    } else if (cop->pending.kind == MYRAIDENII_CORE_COP_PENDING_DMA) {
        cop->failure.operation = MYRAIDENII_CORE_COP_OPERATION_DMA;
        cop->failure.operation_value = cop->pending.mode;
    } else {
        cop->failure.operation = MYRAIDENII_CORE_COP_OPERATION_SORT;
        cop->failure.operation_value = cop->pending.count;
    }
    if (cop->pending.kind == MYRAIDENII_CORE_COP_PENDING_DMA) {
        status = myraidenii_core_cop_dma(cop);
    } else if (cop->pending.kind == MYRAIDENII_CORE_COP_PENDING_SORT) {
        status = myraidenii_core_cop_sort(cop);
    } else {
        kind = myraidenii_core_cop_classify(cop->pending.command);
        myraidenii_core_cop_store_word(cop, MYRAIDENII_CORE_COP_STATUS,
            myraidenii_core_cop_word(cop, MYRAIDENII_CORE_COP_STATUS) & 0x7fffu);
        if (kind == MYRAIDENII_CORE_COP_UPDATE) {
            status = myraidenii_core_cop_update(cop);
        } else if (kind == MYRAIDENII_CORE_COP_OFFSET_ADD) {
            status = myraidenii_core_cop_offset_add(cop);
        } else if (kind == MYRAIDENII_CORE_COP_ANGLE_APPROACH) {
            status = myraidenii_core_cop_approach_angle(cop);
        } else if (kind == MYRAIDENII_CORE_COP_NUMERIC) {
            status = myraidenii_core_cop_numeric(cop);
        } else if (kind == MYRAIDENII_CORE_COP_POSITION) {
            status = myraidenii_core_cop_capture_position(cop);
        } else if (kind == MYRAIDENII_CORE_COP_HITBOX) {
            status = myraidenii_core_cop_capture_hitbox(cop);
        } else if (kind == MYRAIDENII_CORE_COP_COPY) {
            source = myraidenii_core_cop_address(cop, 0u);
            destination = myraidenii_core_cop_address(cop,
                cop->pending.command == 0xf205u ? 2u : 1u);
            if (cop->pending.command == 0xf205u) {
                if (source > LIB_UINT32_MAX - 4u) status = LIB_STATUS_INVALID_ARGUMENT;
                else source += 4u;
            }
            if (status == LIB_STATUS_OK)
                status = myraidenii_core_cop_memory(cop, source, 0u, 4u,
                    LIB_FALSE, &value);
            if (status == LIB_STATUS_OK)
                status = myraidenii_core_cop_memory(cop, destination, 0u, 4u,
                    LIB_TRUE, &value);
        } else {
            status = LIB_STATUS_UNSUPPORTED;
        }
    }
    if (status != LIB_STATUS_OK) myraidenii_core_cop_note_failure(cop, status, 0u);
    myraidenii_core_cop_pending_clear(cop);
    return status;
}

lib_status myraidenii_core_cop_failure_get(const myraidenii_core_cop *cop,
    myraidenii_core_cop_failure_observation *out_observation)
{
    if (cop == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_observation = cop->failure;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_cop_provider_failure_get(const myraidenii_core_cop *cop,
    myraidenii_core_cop_provider_failure_observation *out_observation)
{
    if (cop == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_observation = cop->provider_failure;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_cop_install(myraidenii_core_cop *cop,
    x86_core_machine *machine)
{
    const x86_core_machine_memory_device_callbacks callbacks = {
        myraidenii_core_cop_read,
        myraidenii_core_cop_write,
        myraidenii_core_cop_query
    };
    x86_core_machine_memory_device_route routes[
        sizeof(myraidenii_core_cop_ranges) / sizeof(myraidenii_core_cop_ranges[0])];
    lib_size index;

    if (cop == LIB_NULL || machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < sizeof(routes) / sizeof(routes[0]); ++index) {
        routes[index].physical_start = myraidenii_core_cop_ranges[index].physical_start;
        routes[index].bytes = myraidenii_core_cop_ranges[index].bytes;
        routes[index].callbacks = callbacks;
        routes[index].mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT;
        routes[index].failure = myraidenii_core_cop_provider_failure_note;
    }
    lib_status status = x86_core_machine_install_memory_device_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]), LIB_NULL, LIB_NULL, cop);

    if (status == LIB_STATUS_OK) cop->machine = machine;
    return status;
}
