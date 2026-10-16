#include "x86-nxvm/machine/machine_interface.h"
#include "../support/model40.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-core/machine/memory_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "../support/model40_session_assets.h"

static x86_core_machine_memory_parity_config registered_parity;
static void *registered_owner;
static lib_size registration_count;

static lib_status d4_parity_capture_registration(x86_core_machine *machine,
    const x86_core_machine_memory_device_route *routes, lib_size count,
    x86_core_machine_memory_write_observer observer,
    const x86_core_machine_memory_parity_config *parity, void *owner)
{
    const lib_status status = x86_core_machine_install_memory_device_routes(
        machine, routes, count, observer, parity, owner);

    if (status == LIB_STATUS_OK && parity != LIB_NULL) {
        registered_parity = *parity;
        registered_owner = owner;
        ++registration_count;
    }
    return status;
}

/* Observe the actual App producer registration, not private Machine storage. */
#define x86_core_machine_install_memory_device_routes d4_parity_capture_registration
#include "app-mydeskpro386/d4_memory.c"
#undef x86_core_machine_install_memory_device_routes

static lib_i32 read_byte(x86_core_machine *machine, lib_u32 physical,
    lib_u8 *out_value)
{
    return x86_core_machine_memory_read(machine, physical, out_value,
        sizeof(*out_value)) == LIB_STATUS_OK;
}

static lib_i32 write_byte(x86_core_machine *machine, lib_u32 physical,
    lib_u8 value)
{
    return x86_core_machine_memory_write(machine, physical, &value,
        sizeof(value)) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    static lib_u8 even[MYDESKPRO386_ROM_CHIP_BYTES];
    static lib_u8 odd[MYDESKPRO386_ROM_CHIP_BYTES];
    mydeskpro386_d4_platform_observation observation;
    x86_nxvm_machine *session = LIB_NULL;
    lib_u8 value = 0u;
    lib_u32 port_value = 0u;
    const lib_u32 parity_physical = 0x00012345u;
    const lib_u32 clear_physical = 0x00012346u;
    lib_i32 failed = 0;
    lib_i32 step = 0;

#define CHECK(expression) do { ++step; if (!(expression)) { failed = step; goto done; } } while (0)
    CHECK(vm_model40_fixture_create_bytes(even, odd, &session) == LIB_STATUS_OK &&
        session != LIB_NULL);
    if (!failed) {
        CHECK(read_byte(session->x86_core_machine,
            MYDESKPRO386_D4_CONTROL_PHYSICAL, &value) && value == 0x8fu);
        CHECK(write_byte(session->x86_core_machine, parity_physical, 0x5au));
        CHECK(x86_core_machine_bus_write(session->x86_core_machine, 0x0070u, 0x80u) ==
            LIB_STATUS_OK);
        /* DeskPro port 61h bit 3 disables IOCHK/NMI after reset.  Enable the
         * selected source before injecting the parity fault; the CMOS mask
         * keeps the fault latched until the following observation. */
        CHECK(x86_core_machine_bus_write(session->x86_core_machine, 0x0061u, 0x07u) ==
            LIB_STATUS_OK);
        CHECK(registration_count == 1u && registered_owner != LIB_NULL &&
            registered_parity.bytes == 1024u * 1024u &&
            registered_parity.fault != LIB_NULL);
        /* Machine tests own corruption/detection. Exercise the exact callback
         * registered by this App, then its real diagnostic/NMI/clear routes. */
        registered_parity.fault(registered_owner, parity_physical);
        CHECK(read_byte(session->x86_core_machine, parity_physical, &value) && value == 0x5au);
        CHECK(read_byte(session->x86_core_machine,
            MYDESKPRO386_D4_CONTROL_PHYSICAL, &value) && value == 0x8du);
        CHECK(vm_test_model40_d4_observe(session,
            &observation) == LIB_STATUS_OK && observation.iochk_latched &&
            !observation.nmi_signaled);
        CHECK(x86_core_machine_bus_read(session->x86_core_machine, 0x0061u, &port_value) ==
            LIB_STATUS_OK && (port_value & 0x40u) != 0u);
        CHECK(x86_core_machine_bus_write(session->x86_core_machine, 0x0070u, 0u) == LIB_STATUS_OK);
        CHECK(vm_test_model40_d4_observe(session,
            &observation) == LIB_STATUS_OK && observation.iochk_latched &&
            observation.nmi_signaled);
        CHECK(write_byte(session->x86_core_machine, clear_physical, 0xa5u));
        CHECK(vm_test_model40_d4_observe(session,
            &observation) == LIB_STATUS_OK && !observation.iochk_latched &&
            !observation.nmi_signaled);
        CHECK(read_byte(session->x86_core_machine,
            MYDESKPRO386_D4_CONTROL_PHYSICAL, &value) && value == 0x8du);
        CHECK(write_byte(session->x86_core_machine,
            MYDESKPRO386_D4_CONTROL_PHYSICAL, 0xffu));
        CHECK(read_byte(session->x86_core_machine,
            MYDESKPRO386_D4_CONTROL_PHYSICAL, &value) && value == 0x8fu);
        CHECK(x86_core_machine_reset(session->x86_core_machine) == LIB_STATUS_OK);
        CHECK(read_byte(session->x86_core_machine,
            MYDESKPRO386_D4_CONTROL_PHYSICAL, &value) && value == 0x8fu);
        CHECK(vm_test_model40_d4_observe(session,
            &observation) == LIB_STATUS_OK && !observation.iochk_latched &&
            !observation.nmi_signaled);
    }
done:
#undef CHECK
    x86_nxvm_machine_destroy(session);
    if (failed) {
        lib_c_printf("D4-PARITY-DIAGNOSTIC:STEP-%u\n", (unsigned int)failed);
        return 1;
    }
    lib_c_printf("D4-PARITY-DIAGNOSTIC:OK\n");
    lib_c_printf("D4-IOCHK-CLEAR:OK\n");
    lib_c_printf("MEMORY-PARITY-OWNER:OK\n");
    return 0;
}
