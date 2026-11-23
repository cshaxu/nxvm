#include "lib/types/file.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/memory_interface.h"
#include "x86-core/machine/rom_mapping_interface.h"

/* Guest loop that sums packed FAT12 entries in RAM, with code in RAM or ROM. */
static const lib_u8 guest[] = {
    0xbcu, 0x00u, 0x40u, 0xbfu, 0x00u, 0x80u, 0xbeu, 0xbcu, 0x02u,
    0x31u, 0xd2u, 0x31u, 0xdbu, 0xe8u, 0x0fu, 0x00u, 0x01u, 0xc2u,
    0x83u, 0xc3u, 0x03u, 0x81u, 0xfbu, 0xfeu, 0x01u, 0x72u, 0xf2u,
    0x4eu, 0x75u, 0xedu, 0xf4u, 0x51u, 0x26u, 0x8bu, 0x01u, 0x25u,
    0xffu, 0x0fu, 0x26u, 0x8bu, 0x49u, 0x01u, 0xc1u, 0xe9u, 0x04u,
    0x01u, 0xc8u, 0x59u, 0xc3u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u,
    0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u
};

static lib_status decline_read(void *owner, lib_u32 physical, lib_uptr data,
    lib_uptr bytes, lib_bool observe)
{
    (void)owner;
    (void)physical;
    (void)data;
    (void)bytes;
    (void)observe;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status decline_write(void *owner, lib_u32 physical, lib_uptr data,
    lib_uptr bytes)
{
    (void)owner;
    (void)physical;
    (void)data;
    (void)bytes;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status decline_query(void *owner, lib_u32 physical, lib_uptr bytes,
    x86_core_machine_memory_access access)
{
    (void)owner;
    (void)physical;
    (void)bytes;
    (void)access;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_bool train_mapping(lib_u8 mapping)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_AM386SXLV
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x1000u}
    };
    const x86_core_machine_memory_device_route overlay = {
        mapping == 2u ? 0x20000u : 0x1000u,
        4096u, {decline_read, decline_write, decline_query},
        X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT_PARTIAL, LIB_NULL
    };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    lib_u8 fat[510];
    lib_u32 value = 0u;

    for (lib_u32 byte = 0u; byte < sizeof(fat); ++byte)
        fat[byte] = (lib_u8)(37u * byte + (byte >> 4u));

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return LIB_FALSE;
    const lib_bool passed =
        (mapping == 0u ||
            (x86_core_machine_register_immutable_rom_mapping(machine,
                0x1000u, guest, sizeof(guest)) == LIB_STATUS_OK &&
             x86_core_machine_install_memory_device_routes(machine, &overlay, 1u,
                LIB_NULL, LIB_NULL, &result) == LIB_STATUS_OK)) &&
        x86_core_machine_freeze_execution_providers(machine) == LIB_STATUS_OK &&
        x86_core_machine_reset(machine) == LIB_STATUS_OK &&
        x86_core_machine_memory_write(machine, 0x1000u, guest, sizeof(guest)) == LIB_STATUS_OK &&
        x86_core_machine_memory_write(machine, 0x8000u, fat, sizeof(fat)) == LIB_STATUS_OK &&
        x86_core_machine_debug_patch_registers(machine, &entry) == LIB_STATUS_OK &&
        x86_core_machine_run(machine, (x86_core_machine_run_budget){2000000u, 0u},
            &result) == LIB_STATUS_OK &&
        x86_core_machine_debug_read_register(machine, X86_CORE_CPU_DEBUG_EDX,
            &value) == LIB_STATUS_OK &&
        result.reason == X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
        result.executed == 1549105u && result.ticks == 6668907u &&
        (value & 0xffffu) == 15552u;
    x86_core_machine_destroy(machine);
    return passed;
}

int main(void)
{
    for (lib_u8 mapping = 0u; mapping < 3u; ++mapping) {
        if (!train_mapping(mapping)) {
            lib_c_fprintf(lib_c_stderr, "Core PGO training failed for mapping %u\n",
                (unsigned)mapping);
            return 1;
        }
    }
    return 0;
}
