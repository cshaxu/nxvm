#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "support/rom/model40_session_assets.h"

static lib_i32 read_byte(core_machine *machine, lib_u32 physical,
    lib_u8 *out_value)
{
    return core_machine_memory_read(machine, physical, out_value,
        sizeof(*out_value)) == LIB_STATUS_OK;
}

static lib_i32 write_byte(core_machine *machine, lib_u32 physical,
    lib_u8 value)
{
    return core_machine_memory_write(machine, physical, &value,
        sizeof(value)) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    static lib_u8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static lib_u8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    core_machine_d4_platform_observation observation;
    vm_machine *session = LIB_NULL;
    lib_u8 value = 0u;
    lib_u32 port_value = 0u;
    const lib_u32 parity_physical = 0x00012345u;
    const lib_u32 clear_physical = 0x00012346u;
    lib_i32 failed = 0;
    lib_i32 step = 0;

#define CHECK(expression) do { ++step; if (!(expression)) failed = step; } while (0)
    CHECK(vm_model40_fixture_create_bytes(even, odd, &session) == LIB_STATUS_OK &&
        session != LIB_NULL);
    if (!failed) {
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, &value) && value == 0x8fu);
        CHECK(write_byte(session->core_machine, parity_physical, 0x5au));
        CHECK(core_machine_bus_write(session->core_machine, 0x0070u, 0x80u) ==
            LIB_STATUS_OK);
        /* DeskPro port 61h bit 3 disables IOCHK/NMI after reset.  Enable the
         * selected source before injecting the parity fault; the CMOS mask
         * keeps the fault latched until the following observation. */
        CHECK(core_machine_bus_write(session->core_machine, 0x0061u, 0x07u) ==
            LIB_STATUS_OK);
        CHECK(session->core_machine->executor_memory.connect.parity != 0u);
        if (!failed) {
            ((lib_u8 *)session->core_machine->executor_memory.connect.parity)
                [parity_physical] ^= 1u;
        }
        CHECK(read_byte(session->core_machine, parity_physical, &value) && value == 0x5au);
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, &value) && value == 0x8du);
        CHECK(core_machine_get_d4_platform_observation(session->core_machine,
            &observation) == LIB_STATUS_OK && observation.iochk_latched &&
            !observation.nmi_signaled);
        CHECK(core_machine_bus_read(session->core_machine, 0x0061u, &port_value) ==
            LIB_STATUS_OK && (port_value & 0x40u) != 0u);
        CHECK(core_machine_bus_write(session->core_machine, 0x0070u, 0u) == LIB_STATUS_OK);
        CHECK(core_machine_get_d4_platform_observation(session->core_machine,
            &observation) == LIB_STATUS_OK && observation.iochk_latched &&
            observation.nmi_signaled);
        CHECK(write_byte(session->core_machine, clear_physical, 0xa5u));
        CHECK(core_machine_get_d4_platform_observation(session->core_machine,
            &observation) == LIB_STATUS_OK && !observation.iochk_latched &&
            !observation.nmi_signaled);
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, &value) && value == 0x8du);
        CHECK(write_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, 0xffu));
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, &value) && value == 0x8fu);
        CHECK(core_machine_reset(session->core_machine) == LIB_STATUS_OK);
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, &value) && value == 0x8fu);
        CHECK(core_machine_get_d4_platform_observation(session->core_machine,
            &observation) == LIB_STATUS_OK && !observation.iochk_latched &&
            !observation.nmi_signaled);
    }
#undef CHECK
    vm_machine_destroy(session);
    if (failed) {
        printf("M5:T386:S22:D4-PARITY-DIAGNOSTIC:STEP-%u\n", (unsigned int)failed);
        return 1;
    }
    printf("M5:T386:S22:D4-PARITY-DIAGNOSTIC:OK\n");
    printf("M5:T386:S22:D4-IOCHK-CLEAR:OK\n");
    printf("M5:T386:S22:MEMORY-PARITY-OWNER:OK\n");
    return 0;
}
