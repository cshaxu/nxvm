#include "ibmpc/machine/input_interface.h"

lib_i32 main(void)
{
    const lib_u8 bytes[] = {0x55u, 0xaau};
    const vm_machine_assets assets = {
        .bios = {{bytes, sizeof(bytes)}}
    };
    const vm_machine_config config = {
        .memory_bytes = 1024u * 1024u,
        .floppy_format = VM_MACHINE_FLOPPY_FORMAT_1200K,
        .floppy_mode = {LIB_STORAGE_MEDIUM_READONLY, LIB_STORAGE_MEDIUM_OVERLAY},
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const vm_machine_assets copied_assets = assets;
    const vm_machine_config copied_config = config;

    return copied_assets.bios[0].data != bytes ||
        copied_assets.bios[0].bytes != sizeof(bytes) ||
        copied_config.memory_bytes != 1024u * 1024u ||
        copied_config.floppy_format != VM_MACHINE_FLOPPY_FORMAT_1200K ||
        copied_config.floppy_mode[0] != LIB_STORAGE_MEDIUM_READONLY ||
        copied_config.floppy_mode[1] != LIB_STORAGE_MEDIUM_OVERLAY ||
        copied_config.cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        copied_config.fpu_profile != X86_FPU_PROFILE_NONE;
}
