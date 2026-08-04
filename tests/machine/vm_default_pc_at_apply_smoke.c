#include "type.h"

#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "vm/machine/cmos.h"

C_INT main(C_VOID)
{
    vm_session session = {0};
    vm_machine_cmos_defaults defaults;

    vm_session_initialize(&session);
    if (!session.active || session.profile == STD_NULL ||
        session.fdc.connect.config.dor_port != 0x03f2u ||
        session.fdc.connect.config.status_port != 0x03f4u ||
        session.fdc.connect.config.data_port != 0x03f5u ||
        session.fdc.connect.config.direction_port != 0x03f7u ||
        session.fdc.connect.config.irq != 6u ||
        session.fdc.connect.config.dma_channel != 2u) {
        vm_session_finalize(&session);
        return 1;
    }
    defaults.equipment = session.profile->cmos.equipment;
    defaults.base_memory_kib = session.profile->cmos.base_memory_kib;
    defaults.floppy_type = session.profile->cmos.floppy_type;
    defaults.fixed_disk_type = session.profile->cmos.fixed_disk_type;
    vm_machine_cmos_apply_defaults(&session.cmos, &defaults);
    if (session.cmos.connect.reg[VCMOS_EQUIPMENT] != 0x21u ||
        session.cmos.connect.reg[VCMOS_BASEMEM_LSB] != 0x7fu ||
        session.cmos.connect.reg[VCMOS_BASEMEM_MSB] != 0x02u) {
        vm_session_finalize(&session);
        return 1;
    }
    vm_session_finalize(&session);
    puts("M5:T208:S3:DEFAULT-PC-AT-APPLY:OK");
    return 0;
}
