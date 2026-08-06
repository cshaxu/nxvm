#include "type.h"

#include "core/machine/pic.h"
#include "core/machine/port.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "vm/machine/fdc.h"
#include "vm/machine/fdd.h"

C_INT main(C_VOID)
{
    vm_session *session;
    t_port *port;
    C_INT failed = 0;

    session = STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    port = session->fdc.connect.port;
    if (!session->active || port == STD_NULL) failed = 1;
    vm_machine_fdd_create_for(&session->fdd);
    vm_machine_fdc_refresh(&session->fdc);
    core_machine_port_write(port, 0x03f2u, 0x0cu);

    /* READ TRACK is the corpus-selected, currently unsupported command. */
    core_machine_port_write(port, 0x03f5u, 0x42u);
    failed |= (core_machine_port_read(port, 0x03f4u) &
        (VFDC_MSR_RQM | VFDC_MSR_DIO | VFDC_MSR_CB)) !=
        (VFDC_MSR_RQM | VFDC_MSR_DIO | VFDC_MSR_CB);
    failed |= core_machine_port_read(port, 0x03f5u) != 0x80u;
    failed |= (core_machine_port_read(port, 0x03f4u) & VFDC_MSR_CB) != 0u;
    failed |= core_machine_pic_scan_interrupt(session->fdc.connect.irq_source.master,
        session->fdc.connect.irq_source.slave);

    vm_session_finalize(session);
    STD_FREE(session);
    if (failed) return 1;
    STD_PRINTF("M5:T242:S1:FDC:PORT:OK\n");
    return 0;
}
