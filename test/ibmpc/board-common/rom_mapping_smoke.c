#include "lib/types/test.h"
#include "ibmpc/board-common/rom_mapping_interface.h"

struct core_machine_firmware_context {
    lib_size calls;
    lib_size fail_at;
    lib_u32 addresses[4u];
    lib_u32 sources[4u];
};

lib_status core_machine_firmware_register_immutable_rom(
    core_machine_firmware_context *firmware, lib_u32 address,
    const lib_u8 *image, lib_size bytes)
{
    lib_test_assert(image != LIB_NULL && bytes == 4u);
    firmware->addresses[firmware->calls++] = address;
    return firmware->calls == firmware->fail_at ? LIB_STATUS_NO_MEMORY : LIB_STATUS_OK;
}

lib_status core_machine_firmware_register_immutable_rom_alias(
    core_machine_firmware_context *firmware, lib_u32 source, lib_u32 address, lib_size bytes)
{
    lib_test_assert(bytes == 4u);
    firmware->sources[firmware->calls] = source;
    firmware->addresses[firmware->calls++] = address;
    return firmware->calls == firmware->fail_at ? LIB_STATUS_LIMIT_EXCEEDED : LIB_STATUS_OK;
}

lib_i32 main(void)
{
    const lib_u8 image[] = {1u, 2u, 3u, 4u};
    const vm_profile_rom_region regions[] = {{0xf8000u, image, sizeof(image)},
        {0xc0000u, image, sizeof(image)}};
    const vm_profile_rom_alias aliases[] = {{0xc0000u, 0xe0000u, 4u},
        {0xf8000u, 0xffff8000u, 4u}};
    for (lib_size fail = 0u; fail <= 4u; ++fail) {
        core_machine_firmware_context firmware = {.fail_at = fail};
        lib_status expected = fail == 0u ? LIB_STATUS_OK :
            (fail <= 2u ? LIB_STATUS_NO_MEMORY : LIB_STATUS_LIMIT_EXCEEDED);
        lib_test_assert(vm_profile_rom_register(&firmware, regions, 2u, aliases, 2u) == expected);
        lib_test_assert(firmware.calls == (fail == 0u ? 4u : fail));
        lib_test_assert(firmware.addresses[0] == 0xf8000u);
        if (firmware.calls > 2u) lib_test_assert(firmware.sources[2u] == 0xc0000u);
        if (firmware.calls > 3u) lib_test_assert(firmware.addresses[3u] == 0xffff8000u);
    }
    core_machine_firmware_context firmware = {0};
    lib_test_assert(vm_profile_rom_register(&firmware, LIB_NULL, 1u, aliases, 2u) ==
        LIB_STATUS_INVALID_ARGUMENT && firmware.calls == 0u);
    lib_test_assert(vm_profile_rom_register(&firmware, regions, 2u, LIB_NULL, 1u) ==
        LIB_STATUS_INVALID_ARGUMENT && firmware.calls == 0u);
    lib_test_assert(vm_profile_rom_register(&firmware, LIB_NULL, 0u, LIB_NULL, 0u) == LIB_STATUS_OK);
    return 0;
}
