#include "myraidenii/core/cop.h"

enum {
    MYRAIDENII_CORE_COP_STORAGE_START = 0x00400u,
    MYRAIDENII_CORE_COP_STORAGE_BYTES = 0x00106u,
    MYRAIDENII_CORE_COP_PROGRAM_WORDS = 256u,
    MYRAIDENII_CORE_COP_PROGRAM_SLOT_WORDS = 8u,
    MYRAIDENII_CORE_COP_PROGRAM_SLOTS =
        MYRAIDENII_CORE_COP_PROGRAM_WORDS / MYRAIDENII_CORE_COP_PROGRAM_SLOT_WORDS
};

typedef struct myraidenii_core_cop_range {
    lib_u32 physical_start;
    lib_size bytes;
    lib_bool command;
} myraidenii_core_cop_range;

typedef struct myraidenii_core_cop_program {
    lib_u16 words[MYRAIDENII_CORE_COP_PROGRAM_SLOT_WORDS];
    lib_u16 value;
    lib_u16 mask;
    lib_u16 trigger;
    lib_u16 written;
    lib_u8 metadata_written;
} myraidenii_core_cop_program;

struct myraidenii_core_cop {
    x86_core_machine *machine;
    lib_u8 registers[MYRAIDENII_CORE_COP_STORAGE_BYTES];
    myraidenii_core_cop_program programs[MYRAIDENII_CORE_COP_PROGRAM_SLOTS];
    lib_u8 metadata_written;
    lib_u16 pending_command;
    lib_u8 pending_slot;
};

static const myraidenii_core_cop_range myraidenii_core_cop_ranges[] = {
    {0x00428u, 0x00006u, LIB_FALSE},
    {0x00432u, 0x0000cu, LIB_FALSE},
    {0x00444u, 0x00002u, LIB_FALSE},
    {0x00476u, 0x0000au, LIB_FALSE},
    {0x004a0u, 0x0000eu, LIB_FALSE},
    {0x004c0u, 0x0000eu, LIB_FALSE},
    {0x00500u, 0x00006u, LIB_TRUE}
};

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

static lib_bool myraidenii_core_cop_command_ready(const myraidenii_core_cop *cop,
    lib_u16 command)
{
    if (command != 0x0205u && command != 0x0905u &&
        command != 0x5205u && command != 0x5a05u && command != 0xf205u)
        return LIB_FALSE;
    for (lib_size index = 0u; index < MYRAIDENII_CORE_COP_PROGRAM_SLOTS; ++index) {
        const myraidenii_core_cop_program *program = &cop->programs[index];

        if (program->written == 0xffffu && program->metadata_written == 0x3fu &&
            program->trigger == command)
            return LIB_TRUE;
    }
    return LIB_FALSE;
}

static void myraidenii_core_cop_register_written(myraidenii_core_cop *cop,
    lib_u32 physical)
{
    if (physical >= 0x438u && physical <= 0x43du)
        cop->metadata_written |= (lib_u8)(1u << (physical - 0x438u));
    if ((physical & ~1u) == 0x432u) {
        lib_u16 address = myraidenii_core_cop_word(cop, 0x434u);
        lib_u16 word = address % MYRAIDENII_CORE_COP_PROGRAM_SLOT_WORDS;
        myraidenii_core_cop_program *program =
            &cop->programs[address / MYRAIDENII_CORE_COP_PROGRAM_SLOT_WORDS];

        program->words[word] = myraidenii_core_cop_word(cop, 0x432u);
        program->written |= (lib_u16)(1u << (word * 2u + (physical & 1u)));
        program->value = myraidenii_core_cop_word(cop, 0x438u);
        program->mask = myraidenii_core_cop_word(cop, 0x43au);
        program->trigger = myraidenii_core_cop_word(cop, 0x43cu);
        program->metadata_written = cop->metadata_written;
    }
}

static const myraidenii_core_cop_range *myraidenii_core_cop_range_find(lib_u32 physical,
    lib_uptr bytes)
{
    lib_size index;

    if (bytes == 0u || bytes > 2u) return LIB_NULL;
    for (index = 0u; index < sizeof(myraidenii_core_cop_ranges) /
        sizeof(myraidenii_core_cop_ranges[0]); ++index) {
        const myraidenii_core_cop_range *range = &myraidenii_core_cop_ranges[index];

        if (physical >= range->physical_start &&
            (lib_u64)physical - range->physical_start + bytes <= range->bytes)
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
    (void)observe_only;
    if (owner == LIB_NULL || destination == 0u || bytes == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (myraidenii_core_cop_range_find(physical, bytes) == LIB_NULL)
        return LIB_STATUS_UNSUPPORTED;
    if ((physical >= 0x4a0u && physical + bytes <= 0x4aeu) ||
        (physical >= 0x4c0u && physical + bytes <= 0x4ceu)) {
        const myraidenii_core_cop *cop = owner;

        lib_memory_copy((void *)destination,
            cop->registers + physical - MYRAIDENII_CORE_COP_STORAGE_START, bytes);
        return LIB_STATUS_OK;
    }
    /* Result registers and write-only configuration still have no read model. */
    return LIB_STATUS_INVALID_STATE;
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
    if (cop->pending_command != 0u) return LIB_STATUS_INVALID_STATE;
    if (range->command) {
        const lib_u8 *data = (const lib_u8 *)source;
        lib_u16 command;

        if (bytes != 2u || (physical & 1u) != 0u) return LIB_STATUS_INVALID_STATE;
        command = (lib_u16)(data[0] | (lib_u16)data[1] << 8u);
        if (!myraidenii_core_cop_command_ready(cop, command))
            return LIB_STATUS_INVALID_STATE;
        cop->pending_command = command;
        cop->pending_slot = (lib_u8)((physical - 0x500u) / 2u);
        return LIB_STATUS_OK;
    }
    /* Validate the complete candidate address before changing either byte. */
    if (physical <= 0x435u && physical + bytes > 0x434u) {
        lib_u16 address = myraidenii_core_cop_word(cop, 0x434u);

        for (lib_uptr index = 0u; index < bytes; ++index) {
            lib_u32 current = physical + (lib_u32)index;

            if (current == 0x434u || current == 0x435u) {
                lib_u16 shift = (lib_u16)((current - 0x434u) * 8u);

                address = (lib_u16)((address & ~(0xffu << shift)) |
                    (lib_u16)((const lib_u8 *)source)[index] << shift);
            }
        }
        if (address >= MYRAIDENII_CORE_COP_PROGRAM_WORDS)
            return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (lib_uptr index = 0u; index < bytes; ++index) {
        lib_u32 current = physical + (lib_u32)index;

        cop->registers[current - MYRAIDENII_CORE_COP_STORAGE_START] =
            ((const lib_u8 *)source)[index];
        myraidenii_core_cop_register_written(cop, current);
    }
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_cop_create(myraidenii_core_cop **out_cop)
{
    myraidenii_core_cop *cop;

    if (out_cop == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_cop = LIB_NULL;
    cop = lib_allocate_zero(1u, sizeof(*cop));
    if (cop == LIB_NULL) return LIB_STATUS_NO_MEMORY;
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
        lib_memory_set(cop->programs, 0, sizeof(cop->programs));
        cop->metadata_written = 0u;
        cop->pending_command = 0u;
        cop->pending_slot = 0u;
    }
}

static lib_status myraidenii_core_cop_memory(myraidenii_core_cop *cop,
    lib_u32 base, lib_u32 displacement, lib_u8 bytes, lib_bool write, lib_u32 *value)
{
    lib_u32 address;
    lib_u32 result = 0u;

    if (base > LIB_UINT32_MAX - displacement) return LIB_STATUS_INVALID_ARGUMENT;
    address = base + displacement;
    for (lib_u8 index = 0u; index < bytes; index += 2u) {
        lib_u16 lane = (lib_u16)(*value >> (index * 8u));
        lib_status status;

        if (address > LIB_UINT32_MAX - index) return LIB_STATUS_INVALID_ARGUMENT;
        status = x86_core_machine_dma_memory_cycle(cop->machine, address + index,
            2u, 0u, write ? X86_CORE_MACHINE_MEMORY_ACCESS_WRITE :
                X86_CORE_MACHINE_MEMORY_ACCESS_READ, &lane, LIB_NULL, LIB_NULL, cop);
        if (status != LIB_STATUS_OK) return status;
        result |= (lib_u32)lane << (index * 8u);
    }
    if (!write) *value = result;
    return LIB_STATUS_OK;
}

static lib_status myraidenii_core_cop_update(myraidenii_core_cop *cop)
{
    const lib_u32 base = myraidenii_core_cop_address(cop, 0u);
    const lib_u32 slot = cop->pending_slot * 4u;
    const lib_u32 position_offset = cop->pending_command == 0x0205u ? 4u : 16u;
    const lib_u32 increment_offset = cop->pending_command == 0x0205u ? 16u : 40u;
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
    if (status != LIB_STATUS_OK || cop->pending_command != 0x0205u) return status;
    status = myraidenii_core_cop_memory(cop, base, 30u + slot, 2u, LIB_FALSE, &coordinate);
    if (status != LIB_STATUS_OK) return status;
    coordinate = (lib_u16)(coordinate + (next >> 16u) - (previous >> 16u));
    return myraidenii_core_cop_memory(cop, base, 30u + slot, 2u, LIB_TRUE, &coordinate);
}

lib_status myraidenii_core_cop_advance(myraidenii_core_cop *cop)
{
    lib_u32 value = 0u;
    lib_u32 source;
    lib_u32 destination;
    lib_status status = LIB_STATUS_OK;

    if (cop == LIB_NULL || cop->machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (cop->pending_command == 0u) return LIB_STATUS_OK;
    if (cop->pending_command == 0x0205u || cop->pending_command == 0x0905u) {
        status = myraidenii_core_cop_update(cop);
        cop->pending_command = 0u;
        return status;
    }
    source = myraidenii_core_cop_address(cop, 0u);
    destination = myraidenii_core_cop_address(cop, cop->pending_command == 0xf205u ? 2u : 1u);
    if (cop->pending_command == 0xf205u) {
        if (source > LIB_UINT32_MAX - 4u) status = LIB_STATUS_INVALID_ARGUMENT;
        else source += 4u;
    }
    /* L2 transfer service at the first Core advance after the command
     * bus write has committed. No physical COP latency is asserted. */
    if (status == LIB_STATUS_OK)
        status = myraidenii_core_cop_memory(cop, source, 0u, 4u, LIB_FALSE, &value);
    if (status == LIB_STATUS_OK)
        status = myraidenii_core_cop_memory(cop, destination, 0u, 4u, LIB_TRUE, &value);
    cop->pending_command = 0u;
    return status;
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
    }
    lib_status status = x86_core_machine_install_memory_device_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]), LIB_NULL, LIB_NULL, cop);

    if (status == LIB_STATUS_OK) cop->machine = machine;
    return status;
}
