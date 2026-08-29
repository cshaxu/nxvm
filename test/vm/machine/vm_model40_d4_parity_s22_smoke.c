#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_private.h"
#include "../support/vm_model40_byob_fixture.h"

static C_INT read_byte(core_machine *machine, type_unsigned_32 physical,
    type_unsigned_8 *out_value)
{
    return core_machine_memory_read(machine, physical, out_value,
        sizeof(*out_value)) == TYPE_STATUS_OK;
}

static C_INT write_byte(core_machine *machine, type_unsigned_32 physical,
    type_unsigned_8 value)
{
    return core_machine_memory_write(machine, physical, &value,
        sizeof(value)) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    core_machine_d4_platform_observation observation;
    vm_session *session = STD_NULL;
    type_unsigned_8 value = 0u;
    type_unsigned_32 port_value = 0u;
    const type_unsigned_32 parity_physical = 0x00012345u;
    const type_unsigned_32 clear_physical = 0x00012346u;
    C_INT failed = 0;
    C_INT step = 0;

#define CHECK(expression) do { ++step; if (!(expression)) failed = step; } while (0)
    CHECK(vm_model40_fixture_create_bytes("t386-s22-even.bin", even,
        "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe",
        "t386-s22-odd.bin", odd,
        "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe",
        &session) == TYPE_STATUS_OK &&
        session != STD_NULL);
    if (!failed) {
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, &value) && value == 0x8fu);
        CHECK(write_byte(session->core_machine, parity_physical, 0x5au));
        CHECK(core_machine_bus_write(session->core_machine, 0x0070u, 0x80u) ==
            TYPE_STATUS_OK);
        /* DeskPro port 61h bit 3 disables IOCHK/NMI after reset.  Enable the
         * selected source before injecting the parity fault; the CMOS mask
         * keeps the fault latched until the following observation. */
        CHECK(core_machine_bus_write(session->core_machine, 0x0061u, 0x07u) ==
            TYPE_STATUS_OK);
        CHECK(session->core_machine->executor_memory.connect.parity != 0u);
        if (!failed) {
            ((type_unsigned_8 *)session->core_machine->executor_memory.connect.parity)
                [parity_physical] ^= 1u;
        }
        CHECK(read_byte(session->core_machine, parity_physical, &value) && value == 0x5au);
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, &value) && value == 0x8du);
        CHECK(core_machine_get_d4_platform_observation(session->core_machine,
            &observation) == TYPE_STATUS_OK && observation.iochk_latched &&
            !observation.nmi_signaled);
        CHECK(core_machine_bus_read(session->core_machine, 0x0061u, &port_value) ==
            TYPE_STATUS_OK && (port_value & 0x40u) != 0u);
        CHECK(core_machine_bus_write(session->core_machine, 0x0070u, 0u) == TYPE_STATUS_OK);
        CHECK(core_machine_get_d4_platform_observation(session->core_machine,
            &observation) == TYPE_STATUS_OK && observation.iochk_latched &&
            observation.nmi_signaled);
        CHECK(write_byte(session->core_machine, clear_physical, 0xa5u));
        CHECK(core_machine_get_d4_platform_observation(session->core_machine,
            &observation) == TYPE_STATUS_OK && !observation.iochk_latched &&
            !observation.nmi_signaled);
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, &value) && value == 0x8du);
        CHECK(write_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, 0xffu));
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, &value) && value == 0x8fu);
        CHECK(core_machine_reset(session->core_machine) == TYPE_STATUS_OK);
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL, &value) && value == 0x8fu);
        CHECK(core_machine_get_d4_platform_observation(session->core_machine,
            &observation) == TYPE_STATUS_OK && !observation.iochk_latched &&
            !observation.nmi_signaled);
    }
#undef CHECK
    vm_session_destroy(session);
    vm_model40_fixture_remove("t386-s22-even.bin", "t386-s22-odd.bin");
    if (failed) {
        STD_PRINTF("M5:T386:S22:D4-PARITY-DIAGNOSTIC:STEP-%u\n", (unsigned int)failed);
        return 1;
    }
    STD_PRINTF("M5:T386:S22:D4-PARITY-DIAGNOSTIC:OK\n");
    STD_PRINTF("M5:T386:S22:D4-IOCHK-CLEAR:OK\n");
    STD_PRINTF("M5:T386:S22:MEMORY-PARITY-OWNER:OK\n");
    return 0;
}
