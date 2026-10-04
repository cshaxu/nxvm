#ifndef TEST_X86_CORE_MEMORY_ALIAS_FIXTURE_H
#define TEST_X86_CORE_MEMORY_ALIAS_FIXTURE_H

#include "x86/core/machine_interface.h"
#include "x86/core/memory_interface.h"

static inline lib_status test_core_machine_fixture_register_reset_mapping(
    core_machine *machine, lib_u32 linear, lib_u32 physical,
    lib_size bytes)
{
    lib_status status;
    lib_size mapped_bytes;
    core_machine_cpu_profile profile;
    core_machine_memory_alias_config alias;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    /* Instruction refresh can prefetch up to 15 bytes.  A reset fixture which
     * supplies a shorter program must still map that complete window; otherwise
     * the trailing fetch can escape a high-ROM alias before the first opcode. */
    mapped_bytes = bytes < 15u ? 15u : bytes;
    status = core_machine_get_cpu_profile(machine, &profile);
    if (status != LIB_STATUS_OK) return status;
    alias = (core_machine_memory_alias_config){linear, physical, mapped_bytes};
    status = core_machine_install_memory_aliases(machine, &alias, 1u, LIB_FALSE);
    /* The corpus names every reset fixture through the 80386 alias.  Each
     * earlier CPU fetches the same bytes through its narrower physical bus. */
    if (status == LIB_STATUS_OK &&
        profile <= CORE_MACHINE_CPU_PROFILE_80286 &&
        linear == 0xfffffff0u) {
        alias.physical_start = profile <= CORE_MACHINE_CPU_PROFILE_80186 ?
            0x000ffff0u : 0x00fffff0u;
        status = core_machine_install_memory_aliases(machine, &alias, 1u, LIB_FALSE);
    }
    return status;
}

#endif
