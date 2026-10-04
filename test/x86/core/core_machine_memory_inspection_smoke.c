#include "lib/types/types_interface.h"
#include "x86/core/machine.h"
#include "x86/core/debug_interface.h"

typedef struct inspection_probe {
    lib_u32 reads;
    lib_u32 parity;
    lib_status status;
} inspection_probe;

static lib_status probe_read(void *opaque, lib_u32 address,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    inspection_probe *probe = opaque;
    (void)address;
    if (!observe_only) ++probe->reads;
    if (probe->status != LIB_STATUS_OK) return probe->status;
    lib_memory_set((void *)destination, 0x90u, bytes);
    return LIB_STATUS_OK;
}

static lib_status probe_write(void *opaque, lib_u32 address,
    lib_uptr source, lib_uptr bytes)
{
    (void)opaque; (void)address; (void)source; (void)bytes;
    return LIB_STATUS_OK;
}

static lib_status probe_query(void *opaque, lib_u32 address, lib_uptr bytes,
    core_machine_memory_access access)
{
    (void)opaque; (void)address; (void)bytes; (void)access;
    return LIB_STATUS_OK;
}

static void probe_parity(void *opaque, lib_u32 address)
{
    inspection_probe *probe = opaque;
    (void)address;
    ++probe->parity;
}

lib_i32 main(void)
{
    const core_machine_executor_config config = {.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386};
    core_machine *machine = LIB_NULL;
    core_machine_cpu_instruction_lexeme lexeme;
    inspection_probe probe = {0};
    lib_u8 bytes[4] = {0};
    lib_u8 expected[4] = {0x11u, 0x90u, 0x90u, 0x22u};
    t_ram *memory;
    lib_bool failed = LIB_FALSE;

    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    memory = &machine->executor_memory;
    failed |= core_machine_memory_register_device_provider(memory, 0x1001u, 2u,
        probe_read, probe_write, probe_query, &probe) != LIB_STATUS_OK;
    failed |= core_machine_memory_register_overlay_device_provider(memory,
        0xfffffff0u, 16u, probe_read, probe_write, probe_query, &probe) != LIB_STATUS_OK;
    failed |= core_machine_memory_enable_parity(memory, 0x2000u,
        probe_parity, &probe) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    if (failed) { core_machine_destroy(machine); return 1; }

    /* Corrupt backing without updating parity, as a same-owner hardware fixture. */
    ((lib_u8 *)memory->connect.backing)[0x1000u] = expected[0];
    ((lib_u8 *)memory->connect.backing)[0x1003u] = expected[3];
    ((lib_u8 *)memory->connect.parity)[0x1000u] ^= 1u;
    failed |= core_machine_memory_inspect_physical(memory, 0x1000u,
        (lib_uptr)bytes, sizeof(bytes), LIB_FALSE) != LIB_STATUS_OK;
    failed |= lib_memory_compare(bytes, expected, sizeof(bytes)) != 0 ||
        probe.reads != 0u || probe.parity != 0u;
    failed |= core_machine_memory_read_physical(memory, 0x1000u,
        (lib_uptr)bytes, sizeof(bytes)) != LIB_STATUS_OK;
    failed |= lib_memory_compare(bytes, expected, sizeof(bytes)) != 0 ||
        probe.reads != 2u || probe.parity == 0u;
    probe.reads = probe.parity = 0u;

    /* Reset preview selects high ROM without A20 wrap or callback side effects. */
    failed |= !core_machine_cpu_execution_preview_lexeme(
        machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
        lexeme.byte_count != 1u || probe.reads != 0u || probe.parity != 0u;
    failed |= core_machine_memory_read_reset_physical(memory, 0xfffffff0u,
        (lib_uptr)bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0x90u || probe.reads != 1u;
    probe.reads = 0u;
    probe.status = LIB_STATUS_IO_ERROR;
    failed |= core_machine_memory_inspect_physical(memory, 0x1001u,
        (lib_uptr)bytes, 1u, LIB_FALSE) != LIB_STATUS_IO_ERROR || probe.reads != 0u;
    failed |= core_machine_cpu_execution_preview_lexeme(
        machine->executor_cpu_execution, &lexeme) || lexeme.available ||
        probe.reads != 0u;
    probe.status = LIB_STATUS_OK;
    failed |= core_machine_memory_inspect_physical(memory, 0x101001u,
        (lib_uptr)bytes, 1u, LIB_FALSE) != LIB_STATUS_OK || bytes[0] != 0x90u;
    failed |= core_machine_memory_inspect_physical(memory, 0u,
        (lib_uptr)bytes, 0u, LIB_FALSE) != LIB_STATUS_INVALID_ARGUMENT;

    /* A paged preview must not publish accessed/dirty bits into either table. */
    {
        const lib_u32 directory = 0x4003u, page = 0x5003u;
        lib_u32 copied = 0u;
        const lib_u8 nop = 0x90u;
        const core_machine_debug_register_patch setup = {
            .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CR3),
            .values = { [CORE_MACHINE_DEBUG_CR3] = 0x3000u }
        };
        lib_u32 cr0;

        failed |= core_machine_memory_write_physical(memory, 0x3000u,
            (lib_uptr)&directory, sizeof(directory)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write_physical(memory, 0x4000u,
            (lib_uptr)&page, sizeof(page)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write_physical(memory, 0x5000u,
            (lib_uptr)&nop, sizeof(nop)) != LIB_STATUS_OK;
        failed |= core_machine_debug_patch_registers(machine, &setup) != LIB_STATUS_OK ||
            core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_CR0,
                &cr0) != LIB_STATUS_OK ||
            core_machine_debug_write_register(machine, CORE_MACHINE_DEBUG_CR0,
                cr0 | VCPU_CR0_PG | VCPU_CR0_PE) != LIB_STATUS_OK;
        core_machine_cpu_execution_invalidate_prefetch(machine->executor_cpu_execution);
        failed |= !core_machine_cpu_execution_preview_lexeme(
            machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
            lexeme.byte_count != 1u;
        failed |= core_machine_memory_inspect_physical(memory, 0x3000u,
            (lib_uptr)&copied, sizeof(copied), LIB_FALSE) != LIB_STATUS_OK ||
            copied != directory;
        failed |= core_machine_memory_inspect_physical(memory, 0x4000u,
            (lib_uptr)&copied, sizeof(copied), LIB_FALSE) != LIB_STATUS_OK ||
            copied != page;
    }
    core_machine_destroy(machine);
    return failed ? 1 : 0;
}
