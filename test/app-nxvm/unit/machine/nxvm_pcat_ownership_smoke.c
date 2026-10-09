#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "core/x86/machine_interface.h"
#include "core/x86/port_interface.h"
#include "core/x86/debug_interface.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "../support/rom/session_assets.h"

static lib_bool fdc_result_is_local(core_machine *machine, core_machine *other)
{
    lib_u32 value;
    const core_machine_run_budget budget = { 1u, 0u };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = { [CORE_MACHINE_DEBUG_EIP] = 0x1000u }
    };
    const lib_u8 loop[] = { 0x90u, 0xebu, 0xfdu };
    core_machine_run_result result;
    if (core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x1000u, loop, sizeof(loop)) != LIB_STATUS_OK)
        return LIB_FALSE;
    if (core_machine_bus_write(machine, 0x03f5u, 0xffu) != LIB_STATUS_OK) return LIB_FALSE;
    for (lib_u32 event = 0u; event < 128u; ++event) {
        if (core_machine_bus_read(machine, 0x03f4u, &value) != LIB_STATUS_OK) return LIB_FALSE;
        if ((value & 0xc0u) == 0xc0u) {
            if (other != LIB_NULL &&
                (core_machine_bus_read(other, 0x03f4u, &value) != LIB_STATUS_OK ||
                    (value & 0x40u) != 0u)) return LIB_FALSE;
            return core_machine_bus_read(machine, 0x03f5u, &value) == LIB_STATUS_OK &&
                value == 0x80u;
        }
        /* An immediate command phase must settle through normal Core
         * execution before a future deadline can be selected. */
        if (core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET) return LIB_FALSE;
    }
    return LIB_FALSE;
}

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    vm_machine *other = LIB_NULL;
    lib_i32 masked = LIB_FALSE;
    lib_i32 failed = 1;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || !session->active || session->core_machine == LIB_NULL ||
        vm_test_default_pc_at_session_create(LIB_NULL, &other) != LIB_STATUS_OK ||
        other == LIB_NULL || !other->active || other->core_machine == LIB_NULL)
        goto done;
    failed = core_machine_bus_write(session->core_machine, 0x03f2u, 0x0cu) != LIB_STATUS_OK ||
        core_machine_bus_write(other->core_machine, 0x03f2u, 0x0cu) != LIB_STATUS_OK ||
        !fdc_result_is_local(session->core_machine, other->core_machine);
    failed = failed || core_machine_bus_write(session->core_machine, 0x0070u, 0x80u) != LIB_STATUS_OK ||
        core_machine_get_nmi_mask(session->core_machine, &masked) != LIB_STATUS_OK || !masked;
    failed = failed || core_machine_bus_write(session->core_machine, 0x0070u, 0u) != LIB_STATUS_OK ||
        core_machine_get_nmi_mask(session->core_machine, &masked) != LIB_STATUS_OK || masked;
    vm_machine_destroy(session);
    session = LIB_NULL;
    /* Destroying one composition must not revoke the other Core's routes. */
    failed = failed || !fdc_result_is_local(other->core_machine, LIB_NULL);
done:
    vm_machine_destroy(session);
    vm_machine_destroy(other);
    if (failed) return 1;
    puts("M5:T264:S3:PCAT-OWNERSHIP:OK");
    return 0;
}
