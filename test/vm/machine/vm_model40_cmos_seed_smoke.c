#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "core/machine/rtc.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "../support/vm_model40_byob_fixture.h"

static type_unsigned_8 vm_model40_cmos_read(vm_session *session,
    type_unsigned_8 index)
{
    t_port *port = &session->core_machine->executor_port;

    core_machine_port_write(port, 0x0070u, index);
    return (type_unsigned_8)core_machine_port_read(port, 0x0071u);
}

static C_VOID vm_model40_cmos_write(vm_session *session, type_unsigned_8 index,
    type_unsigned_8 value)
{
    t_port *port = &session->core_machine->executor_port;

    core_machine_port_write(port, 0x0070u, index);
    core_machine_port_write(port, 0x0071u, value);
}

static C_INT vm_model40_cmos_checksum_is_valid(vm_session *session)
{
    type_unsigned_16 checksum = 0u;
    type_unsigned_8 index;

    for (index = 0x10u; index < 0x2eu; ++index) {
        checksum = (type_unsigned_16)(checksum + vm_model40_cmos_read(session, index));
    }
    return vm_model40_cmos_read(session, 0x2eu) ==
        TYPE_MASK_UNSIGNED_8(checksum >> 8u) &&
        vm_model40_cmos_read(session, 0x2fu) == TYPE_MASK_UNSIGNED_8(checksum);
}

static C_INT vm_model40_cmos_seed_matches(vm_session *session)
{
    return vm_model40_cmos_read(session, CORE_MACHINE_RTC_TYPE_DISK_FLOPPY) == 0x22u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_TYPE_DISK_FIXED) == 0x80u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_TYPE_DISK_FIXED_EXTENDED_0) == 0u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_EQUIPMENT) == 0x41u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_BASEMEM_LSB) == 0x80u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_BASEMEM_MSB) == 0x02u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_EXTMEM_LSB) == 0x00u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_EXTMEM_MSB) == 0x04u &&
        vm_model40_cmos_checksum_is_valid(session);
}

C_INT main(C_VOID)
{
    vm_session *first = STD_NULL;
    vm_session *second = STD_NULL;
    C_INT failed = 0;

    failed |= vm_model40_fixture_create("t513-model40-cmos-even.bin",
        "t513-model40-cmos-odd.bin", &first) != TYPE_STATUS_OK || first == STD_NULL;
    if (!failed) failed |= !vm_model40_cmos_seed_matches(first);
    if (!failed) {
        vm_model40_cmos_write(first, CORE_MACHINE_RTC_EQUIPMENT, 0x5au);
        failed |= vm_session_reset(first) != TYPE_STATUS_OK ||
            vm_model40_cmos_read(first, CORE_MACHINE_RTC_EQUIPMENT) != 0x5au;
    }
    failed |= vm_model40_fixture_create("t513-model40-cmos-even.bin",
        "t513-model40-cmos-odd.bin", &second) != TYPE_STATUS_OK || second == STD_NULL;
    if (!failed) failed |= !vm_model40_cmos_seed_matches(second);
    vm_session_destroy(second);
    vm_session_destroy(first);
    vm_model40_fixture_remove("t513-model40-cmos-even.bin",
        "t513-model40-cmos-odd.bin");
    if (failed) return 1;
    STD_PRINTF("M5:T513:S4:MODEL40-CMOS-SEED:OK\n");
    return 0;
}
