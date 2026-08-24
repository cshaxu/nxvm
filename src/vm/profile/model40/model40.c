#include "vm/profile/model40/model40.h"

#include "core/platform/file.h"

C_INT vm_profile_model40_external_rom_is_valid(
    const vm_profile_model40_external_rom *rom)
{
    return rom != STD_NULL && rom->even_bytes != STD_NULL &&
        rom->odd_bytes != STD_NULL &&
        rom->chip_byte_count == VM_PROFILE_MODEL40_ROM_CHIP_BYTES;
}


static type_unsigned_32 vm_profile_model40_sha256_rotate_right(
    type_unsigned_32 value, type_unsigned_8 bits)
{
    return (value >> bits) | (value << (32u - bits));
}

static C_VOID vm_profile_model40_sha256_block(type_unsigned_32 state[8],
    const type_unsigned_8 block[64])
{
    static const type_unsigned_32 constants[64] = {
        0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
        0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
        0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
        0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
        0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
        0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
        0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
        0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u };
    type_unsigned_32 words[64];
    type_unsigned_32 a, b, c, d, e, f, g, h;
    STD_SIZE_T index;

    for (index = 0u; index < 16u; ++index) {
        words[index] = ((type_unsigned_32)block[index * 4u] << 24u) |
            ((type_unsigned_32)block[index * 4u + 1u] << 16u) |
            ((type_unsigned_32)block[index * 4u + 2u] << 8u) | block[index * 4u + 3u];
    }
    for (; index < 64u; ++index) {
        type_unsigned_32 s0 = vm_profile_model40_sha256_rotate_right(words[index - 15u], 7u) ^
            vm_profile_model40_sha256_rotate_right(words[index - 15u], 18u) ^ (words[index - 15u] >> 3u);
        type_unsigned_32 s1 = vm_profile_model40_sha256_rotate_right(words[index - 2u], 17u) ^
            vm_profile_model40_sha256_rotate_right(words[index - 2u], 19u) ^ (words[index - 2u] >> 10u);
        words[index] = words[index - 16u] + s0 + words[index - 7u] + s1;
    }
    a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4]; f = state[5]; g = state[6]; h = state[7];
    for (index = 0u; index < 64u; ++index) {
        type_unsigned_32 s1 = vm_profile_model40_sha256_rotate_right(e, 6u) ^
            vm_profile_model40_sha256_rotate_right(e, 11u) ^ vm_profile_model40_sha256_rotate_right(e, 25u);
        type_unsigned_32 choice = (e & f) ^ ((~e) & g);
        type_unsigned_32 temp1 = h + s1 + choice + constants[index] + words[index];
        type_unsigned_32 s0 = vm_profile_model40_sha256_rotate_right(a, 2u) ^
            vm_profile_model40_sha256_rotate_right(a, 13u) ^ vm_profile_model40_sha256_rotate_right(a, 22u);
        type_unsigned_32 majority = (a & b) ^ (a & c) ^ (b & c);
        type_unsigned_32 temp2 = s0 + majority;
        h = g; g = f; f = e; e = d + temp1; d = c; c = b; b = a; a = temp1 + temp2;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

static C_VOID vm_profile_model40_sha256(const type_unsigned_8 *bytes,
    STD_SIZE_T byte_count, type_unsigned_8 digest[32])
{
    type_unsigned_32 state[8] = { 0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u };
    type_unsigned_8 final[128] = {0};
    STD_SIZE_T index, final_bytes;
    type_unsigned_64 bit_count = (type_unsigned_64)byte_count * 8u;

    for (index = 0u; index + 64u <= byte_count; index += 64u) {
        vm_profile_model40_sha256_block(state, bytes + index);
    }
    final_bytes = byte_count - index;
    if (final_bytes != 0u) STD_MEMCPY(final, bytes + index, final_bytes);
    final[final_bytes++] = 0x80u;
    if (final_bytes > 56u) {
        vm_profile_model40_sha256_block(state, final);
        STD_MEMSET(final, 0, 64u);
    }
    for (index = 0u; index < 8u; ++index) final[63u - index] = (type_unsigned_8)(bit_count >> (index * 8u));
    vm_profile_model40_sha256_block(state, final);
    for (index = 0u; index < 8u; ++index) {
        digest[index * 4u] = (type_unsigned_8)(state[index] >> 24u);
        digest[index * 4u + 1u] = (type_unsigned_8)(state[index] >> 16u);
        digest[index * 4u + 2u] = (type_unsigned_8)(state[index] >> 8u);
        digest[index * 4u + 3u] = (type_unsigned_8)state[index];
    }
}

static C_INT vm_profile_model40_hex_value(C_CHAR value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

static C_INT vm_profile_model40_sha256_matches(const type_unsigned_8 digest[32],
    const C_CHAR *text)
{
    STD_SIZE_T index;
    if (text == STD_NULL || STD_STRLEN(text) != 64u) return 0;
    for (index = 0u; index < 32u; ++index) {
        C_INT high = vm_profile_model40_hex_value(text[index * 2u]);
        C_INT low = vm_profile_model40_hex_value(text[index * 2u + 1u]);
        if (high < 0 || low < 0 || digest[index] != (type_unsigned_8)((high << 4u) | low)) return 0;
    }
    return 1;
}

C_INT vm_profile_model40_byob_manifest_is_valid(
    const vm_profile_model40_byob_manifest *manifest)
{
    return manifest != STD_NULL && manifest->even_path != STD_NULL && manifest->odd_path != STD_NULL &&
        manifest->even_path[0] != '\0' && manifest->odd_path[0] != '\0' &&
        manifest->provenance != STD_NULL && manifest->provenance[0] != '\0' &&
        manifest->even_sha256 != STD_NULL && manifest->odd_sha256 != STD_NULL &&
        STD_STRLEN(manifest->even_sha256) == 64u && STD_STRLEN(manifest->odd_sha256) == 64u;
}

static type_status vm_profile_model40_byob_read_chip(const C_CHAR *path,
    const C_CHAR *expected_sha256, type_unsigned_8 *bytes)
{
    type_unsigned_8 digest[32];
    C_VOID *loaded = STD_NULL;
    STD_SIZE_T count;
    if (path == STD_NULL || expected_sha256 == STD_NULL || bytes == STD_NULL ||
        core_platform_file_read_all(path, VM_PROFILE_MODEL40_ROM_CHIP_BYTES,
            &loaded, &count) != TYPE_FALSE || count != VM_PROFILE_MODEL40_ROM_CHIP_BYTES) {
        STD_FREE(loaded);
        return TYPE_STATUS_FAULT;
    }
    STD_MEMCPY(bytes, loaded, count);
    STD_FREE(loaded);
    vm_profile_model40_sha256(bytes, VM_PROFILE_MODEL40_ROM_CHIP_BYTES, digest);
    return vm_profile_model40_sha256_matches(digest, expected_sha256) ? TYPE_STATUS_OK : TYPE_STATUS_FAULT;
}

type_status vm_profile_model40_byob_manifest_load(
    const vm_profile_model40_byob_manifest *manifest,
    type_unsigned_8 *even_bytes, type_unsigned_8 *odd_bytes,
    vm_profile_model40_external_rom *out_rom)
{
    type_status status;
    if (!vm_profile_model40_byob_manifest_is_valid(manifest) || even_bytes == STD_NULL ||
        odd_bytes == STD_NULL || out_rom == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = vm_profile_model40_byob_read_chip(manifest->even_path, manifest->even_sha256, even_bytes);
    if (status != TYPE_STATUS_OK) return status;
    status = vm_profile_model40_byob_read_chip(manifest->odd_path, manifest->odd_sha256, odd_bytes);
    if (status != TYPE_STATUS_OK) { STD_MEMSET(even_bytes, 0, VM_PROFILE_MODEL40_ROM_CHIP_BYTES); return status; }
    *out_rom = (vm_profile_model40_external_rom) { even_bytes, odd_bytes, VM_PROFILE_MODEL40_ROM_CHIP_BYTES };
    return TYPE_STATUS_OK;
}
static C_VOID vm_profile_model40_rom_materialize(
    const vm_profile_model40_external_rom *rom,
    type_unsigned_8 *window)
{
    STD_SIZE_T index;

    for (index = 0u; index < VM_PROFILE_MODEL40_ROM_WINDOW_BYTES; ++index) {
        STD_SIZE_T logical = index % VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES;

        window[index] = (logical & 1u) == 0u ? rom->even_bytes[logical >> 1u] :
            rom->odd_bytes[logical >> 1u];
    }
}

static type_status vm_profile_model40_firmware_configure(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    const vm_profile_model40_external_rom *rom =
        (const vm_profile_model40_external_rom *)opaque;
    type_unsigned_8 window[VM_PROFILE_MODEL40_ROM_WINDOW_BYTES];
    type_status status;

    if (!vm_profile_model40_external_rom_is_valid(rom)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    vm_profile_model40_rom_materialize(rom, window);
    status = core_machine_firmware_register_immutable_rom(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START, window, sizeof(window));
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_firmware_register_immutable_rom_alias(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_COMPATIBILITY_ALIAS_START, sizeof(window));
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_firmware_register_immutable_rom_alias(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_HIGH_ALIAS_START, sizeof(window));
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_firmware_register_immutable_rom_alias(firmware,
        VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START,
        VM_PROFILE_MODEL40_ROM_HIGH_RESET_ALIAS_START, sizeof(window));
}

static type_status vm_profile_model40_firmware_reset(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    (C_VOID)firmware;
    return vm_profile_model40_external_rom_is_valid(
        (const vm_profile_model40_external_rom *)opaque) ? TYPE_STATUS_OK :
        TYPE_STATUS_INVALID_ARGUMENT;
}

static const core_machine_firmware_provider vm_profile_model40_provider = {
    vm_profile_model40_firmware_configure,
    vm_profile_model40_firmware_reset,
    STD_NULL
};

const core_machine_firmware_provider *vm_profile_model40_firmware_provider(C_VOID)
{
    return &vm_profile_model40_provider;
}

static C_INT vm_profile_model40_d4_memory_is_active(
    const vm_profile_model40_d4_memory *memory, type_unsigned_32 physical)
{
    return memory != STD_NULL && ((physical >= VM_PROFILE_MODEL40_D4_COMPATIBILITY_START &&
        physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_START <
            VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES) ||
        (physical >= VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START &&
            physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START <
                VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES) ||
        ((memory->control & 0x01u) == 0u &&
            physical >= VM_PROFILE_MODEL40_D4_REPLACEMENT_START &&
            physical - VM_PROFILE_MODEL40_D4_REPLACEMENT_START <
                VM_PROFILE_MODEL40_D4_REPLACEMENT_BYTES));
}

static STD_SIZE_T vm_profile_model40_d4_memory_offset(type_unsigned_32 physical)
{
    if (physical >= VM_PROFILE_MODEL40_D4_COMPATIBILITY_START &&
        physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_START <
            VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES) {
        return (STD_SIZE_T)(physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_START);
    }
    if (physical >= VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START &&
        physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START <
            VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES) {
        return (STD_SIZE_T)(physical - VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START);
    }
    return (STD_SIZE_T)(physical - VM_PROFILE_MODEL40_D4_REPLACEMENT_START);
}

static type_status vm_profile_model40_d4_memory_read(C_VOID *opaque,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    STD_SIZE_T offset;

    if (memory == STD_NULL || destination == 0u || !vm_profile_model40_d4_memory_is_active(
            memory, physical)) return TYPE_STATUS_UNSUPPORTED;
    offset = vm_profile_model40_d4_memory_offset(physical);
    if (bytes > VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES - offset) {
        return TYPE_STATUS_FAULT;
    }
    STD_MEMCPY((C_VOID *)destination, memory->compatibility + offset, bytes);
    return TYPE_STATUS_OK;
}

static type_status vm_profile_model40_d4_memory_write(C_VOID *opaque,
    type_unsigned_32 physical, type_virtual_address source,
    type_native_unsigned bytes)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    STD_SIZE_T offset;

    if (memory == STD_NULL || source == 0u || !vm_profile_model40_d4_memory_is_active(
            memory, physical)) return TYPE_STATUS_UNSUPPORTED;
    if ((memory->control & 0x02u) == 0u) return TYPE_STATUS_FAULT;
    offset = vm_profile_model40_d4_memory_offset(physical);
    if (bytes > VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES - offset) {
        return TYPE_STATUS_FAULT;
    }
    STD_MEMCPY(memory->compatibility + offset, (C_VOID *)source, bytes);
    return TYPE_STATUS_OK;
}

static type_status vm_profile_model40_d4_memory_query(C_VOID *opaque,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    STD_SIZE_T offset;

    if (memory == STD_NULL || !vm_profile_model40_d4_memory_is_active(memory, physical)) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    offset = vm_profile_model40_d4_memory_offset(physical);
    if (bytes > VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES - offset) {
        return TYPE_STATUS_FAULT;
    }
    if (access == CORE_MACHINE_MEMORY_ACCESS_WRITE && (memory->control & 0x02u) == 0u) {
        return TYPE_STATUS_FAULT;
    }
    return TYPE_STATUS_OK;
}

static C_VOID vm_profile_model40_d4_parity_fault(C_VOID *opaque,
    type_unsigned_32 physical)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    if (memory == STD_NULL || memory->machine == STD_NULL) return;
    memory->parity_fault_mask |= (type_unsigned_8)(1u << (physical & 3u));
    (C_VOID)core_machine_report_d4_iochk_fault(memory->machine);
}

static C_VOID vm_profile_model40_d4_memory_write_observer(C_VOID *opaque,
    type_unsigned_32 physical, type_native_unsigned bytes)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    (C_VOID)physical;
    (C_VOID)bytes;
    if (memory != STD_NULL && memory->machine != STD_NULL &&
        memory->parity_fault_mask != 0u) {
        (C_VOID)core_machine_clear_d4_iochk_fault(memory->machine);
    }
}
static type_status vm_profile_model40_d4_control_read(C_VOID *opaque,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    type_unsigned_32 offset;

    if (memory == STD_NULL || physical < VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL ||
        destination == 0u || bytes != 1u) return TYPE_STATUS_FAULT;
    offset = physical - VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL;
    if (offset >= VM_PROFILE_MODEL40_D4_CONTROL_WINDOW_BYTES) return TYPE_STATUS_UNSUPPORTED;
    if (offset >= VM_PROFILE_MODEL40_D4_CONTROL_REGISTER_BYTES) {
        *(type_unsigned_8 *)destination = 0xffu;
    } else if (offset == 0u) {
        *(type_unsigned_8 *)destination = (type_unsigned_8)(0xbfu & ~memory->parity_fault_mask);
    } else if (offset == 1u) {
        *(type_unsigned_8 *)destination = memory->diagnostic_high;
    } else if (offset == 2u) {
        *(type_unsigned_8 *)destination = (type_unsigned_8)memory->ram_setup;
    } else {
        *(type_unsigned_8 *)destination = (type_unsigned_8)(memory->ram_setup >> 8u);
    }
    return TYPE_STATUS_OK;
}

static type_status vm_profile_model40_d4_control_write(C_VOID *opaque,
    type_unsigned_32 physical, type_virtual_address source,
    type_native_unsigned bytes)
{
    vm_profile_model40_d4_memory *memory = (vm_profile_model40_d4_memory *)opaque;
    type_unsigned_32 offset;

    if (memory == STD_NULL || physical < VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL ||
        source == 0u || bytes != 1u) return TYPE_STATUS_FAULT;
    offset = physical - VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL;
    if (offset >= VM_PROFILE_MODEL40_D4_CONTROL_WINDOW_BYTES) return TYPE_STATUS_UNSUPPORTED;
    if (offset == 0u) {
        memory->control = *(const type_unsigned_8 *)source | 0xfcu;
        memory->parity_fault_mask = 0u;
    } else if (offset == 2u) {
        memory->ram_setup = (type_unsigned_16)((memory->ram_setup & 0xff00u) |
            *(const type_unsigned_8 *)source);
    }
    return TYPE_STATUS_OK;
}

static type_status vm_profile_model40_d4_control_query(C_VOID *opaque,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access)
{
    (C_VOID)opaque;
    if (physical < VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL ||
        physical - VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL >=
            VM_PROFILE_MODEL40_D4_CONTROL_WINDOW_BYTES || bytes != 1u) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    return access == CORE_MACHINE_MEMORY_ACCESS_READ ||
        access == CORE_MACHINE_MEMORY_ACCESS_WRITE ? TYPE_STATUS_OK :
        TYPE_STATUS_UNSUPPORTED;
}
C_VOID vm_profile_model40_d4_memory_initialize(vm_profile_model40_d4_memory *memory)
{
    if (memory == STD_NULL) return;
    STD_MEMSET(memory, 0, sizeof(*memory));
    vm_profile_model40_d4_memory_reset(memory);
}

C_VOID vm_profile_model40_d4_memory_reset(vm_profile_model40_d4_memory *memory)
{
    if (memory != STD_NULL) {
        memory->control = 0xffu;
        /* 1 MiB private model: Tier-2 86Box jumper/settings visibility. */
        memory->diagnostic_high = 0xfdu;
        memory->ram_setup = 0xfc44u;
        memory->parity_fault_mask = 0u;
    }
}

type_status vm_profile_model40_d4_memory_load_compatibility(
    vm_profile_model40_d4_memory *memory,
    const vm_profile_model40_external_rom *rom)
{
    if (memory == STD_NULL || !vm_profile_model40_external_rom_is_valid(rom)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    vm_profile_model40_rom_materialize(rom, memory->compatibility);
    return TYPE_STATUS_OK;
}

type_status vm_profile_model40_d4_memory_register(core_machine *machine,
    vm_profile_model40_d4_memory *memory)
{
    static const core_machine_memory_device_callbacks memory_callbacks = {
        vm_profile_model40_d4_memory_read, vm_profile_model40_d4_memory_write,
        vm_profile_model40_d4_memory_query };
    static const core_machine_memory_device_callbacks control_callbacks = {
        vm_profile_model40_d4_control_read, vm_profile_model40_d4_control_write,
        vm_profile_model40_d4_control_query };
    type_status status;

    if (machine == STD_NULL || memory == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    memory->machine = machine;
    status = core_machine_register_memory_device(machine,
        VM_PROFILE_MODEL40_D4_REPLACEMENT_START,
        VM_PROFILE_MODEL40_D4_REPLACEMENT_BYTES, &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_register_memory_device(machine,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_START,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES, &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_register_memory_device(machine,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES, &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_register_memory_device(machine,
        VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, VM_PROFILE_MODEL40_D4_CONTROL_WINDOW_BYTES,
        &control_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_register_memory_write_observer(machine,
        vm_profile_model40_d4_memory_write_observer, memory);
}

type_status vm_profile_model40_d4_memory_materialize_plan(
    vm_profile_model40_d4_memory *memory, core_machine_plan *plan)
{
    static const core_machine_memory_device_callbacks memory_callbacks = {
        vm_profile_model40_d4_memory_read, vm_profile_model40_d4_memory_write,
        vm_profile_model40_d4_memory_query };
    static const core_machine_memory_device_callbacks control_callbacks = {
        vm_profile_model40_d4_control_read, vm_profile_model40_d4_control_write,
        vm_profile_model40_d4_control_query };
    type_status status;

    if (memory == STD_NULL || plan == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_plan_register_memory_device(plan,
        VM_PROFILE_MODEL40_D4_REPLACEMENT_START, VM_PROFILE_MODEL40_D4_REPLACEMENT_BYTES,
        &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_plan_register_memory_device(plan,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_START, VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES,
        &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_plan_register_memory_device(plan,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_HIGH_START,
        VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES, &memory_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_plan_register_memory_device(plan,
        VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL,
        VM_PROFILE_MODEL40_D4_CONTROL_WINDOW_BYTES, &control_callbacks, memory);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_plan_enable_d4_memory_parity(plan, &memory->parity_fault_mask);
}

type_status vm_profile_model40_d4_memory_enable_parity(core_machine *machine,
    vm_profile_model40_d4_memory *memory)
{
    if (machine == STD_NULL || memory == STD_NULL || memory->machine != machine) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return core_machine_enable_memory_parity(machine, 1024u * 1024u,
        vm_profile_model40_d4_parity_fault, memory);
}
