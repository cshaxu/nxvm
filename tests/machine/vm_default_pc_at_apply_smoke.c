#include "type.h"

#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "tests/support/vm_session_fixture.h"
#include "vm/machine/cmos.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    vm_machine_cmos_defaults defaults;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    if (!vm_session_fixture_is_active(session) || vm_session_fixture_profile(session) == STD_NULL ||
        vm_session_fixture_fdc(session)->connect.config.dor_port != 0x03f2u ||
        vm_session_fixture_fdc(session)->connect.config.status_port != 0x03f4u ||
        vm_session_fixture_fdc(session)->connect.config.data_port != 0x03f5u ||
        vm_session_fixture_fdc(session)->connect.config.direction_port != 0x03f7u ||
        vm_session_fixture_fdc(session)->connect.config.irq != 6u ||
        vm_session_fixture_fdc(session)->connect.config.dma_channel != 2u) {
        vm_session_destroy(session);
        return 1;
    }
    defaults.equipment = vm_session_fixture_profile(session)->cmos.equipment;
    defaults.base_memory_kib = vm_session_fixture_profile(session)->cmos.base_memory_kib;
    defaults.floppy_type = vm_session_fixture_profile(session)->cmos.floppy_type;
    defaults.fixed_disk_type = vm_session_fixture_profile(session)->cmos.fixed_disk_type;
    vm_machine_cmos_apply_defaults(vm_session_fixture_cmos(session), &defaults);
    if (vm_session_fixture_cmos(session)->connect.reg[VCMOS_EQUIPMENT] != 0x21u ||
        vm_session_fixture_cmos(session)->connect.reg[VCMOS_BASEMEM_LSB] != 0x7fu ||
        vm_session_fixture_cmos(session)->connect.reg[VCMOS_BASEMEM_MSB] != 0x02u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    puts("M5:T208:S3:DEFAULT-PC-AT-APPLY:OK");
    return 0;
}
