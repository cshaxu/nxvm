#include "type.h"

#include "core/machine/hdc.h"
#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "vm/composition/session/session_private.h"
#include "vm/machine/hdd.h"
#include "../support/rom/model40_session_assets.h"

#define MODEL40_HDC_BYTES (925u * 5u * 17u * 512u)

static C_INT read_first_sector(vm_session *session, type_unsigned_8 drive_head,
    type_unsigned_16 expected_word)
{
    core_machine_hdc *hdc;
    type_unsigned_32 value;
    type_unsigned_16 index;

    if (session == STD_NULL || session->core_machine == STD_NULL) return 0;
    hdc = &session->core_machine->hdc;
    core_machine_port_write(&session->core_machine->executor_port, 0x01f2u, 1u);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f3u, 1u);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f4u, 0u);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f5u, 0u);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f6u, drive_head);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f7u, 0x20u);
    core_machine_hdc_advance(hdc);
    value = core_machine_port_read(&session->core_machine->executor_port, 0x03f6u);
    if ((value & CORE_MACHINE_HDC_STATUS_DRQ) == 0u || !core_machine_hdc_irq_pending(hdc)) return 0;
    value = core_machine_port_read(&session->core_machine->executor_port, 0x01f7u);
    if ((value & CORE_MACHINE_HDC_STATUS_DRQ) == 0u || core_machine_hdc_irq_pending(hdc) ||
        core_machine_port_read(&session->core_machine->executor_port, 0x01f0u) != expected_word) return 0;
    for (index = 1u; index < 256u; ++index) {
        (C_VOID)core_machine_port_read(&session->core_machine->executor_port, 0x01f0u);
    }
    core_machine_hdc_advance(hdc);
    if (!core_machine_hdc_irq_pending(hdc)) return 0;
    (C_VOID)core_machine_port_read(&session->core_machine->executor_port, 0x01f7u);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f7u, 0xecu);
    core_machine_hdc_advance(hdc);
    value = core_machine_port_read(&session->core_machine->executor_port, 0x01f7u);
    if ((value & CORE_MACHINE_HDC_STATUS_ERR) == 0u || core_machine_hdc_irq_pending(hdc)) return 0;
    core_machine_port_write(&session->core_machine->executor_port, 0x03f6u,
        CORE_MACHINE_HDC_DEVICE_CONTROL_SRST);
    core_machine_port_write(&session->core_machine->executor_port, 0x03f6u, 0u);
    return !core_machine_hdc_irq_pending(hdc) &&
        core_machine_port_read(&session->core_machine->executor_port, 0x01f1u) ==
            CORE_MACHINE_HDC_ERROR_DIAGNOSTIC_OK;
}

C_INT main(C_VOID)
{
    type_unsigned_8 *image = (type_unsigned_8 *)STD_CALLOC(1u, MODEL40_HDC_BYTES);
    vm_session *session = STD_NULL;
    C_INT failed = image == STD_NULL;

    if (!failed) {
        image[0u] = 0xa5u;
        image[1u] = 0x5au;
        failed = vm_model40_fixture_create(&session) != TYPE_STATUS_OK || session == STD_NULL ||
            vm_machine_hdd_replace_bytes(&session->hdd, image, MODEL40_HDC_BYTES) ||
            vm_machine_hdd_set_geometry(&session->hdd, 925u, 5u, 17u) ||
            !session->hdd.connect.flagDiskExist ||
            session->core_machine->hdc.connect.config.service.command_ticks != 0u ||
            session->core_machine->hdc.connect.config.service.next_sector_ticks != 0u ||
            session->hdd.data.ncyl != 925u || session->hdd.data.nhead != 5u ||
            session->hdd.data.nsector != 17u || session->hdd.data.nbyte != 512u ||
            session->core_machine->hdc.connect.slave_media_id != CORE_MACHINE_MEDIA_ID_INVALID ||
            !read_first_sector(session, 0x20u, 0x5aa5u) ||
            !read_first_sector(session, 0xa0u, 0x5aa5u);
    }
    vm_session_destroy(session);
    STD_FREE(image);
    if (failed) return 1;
    STD_PRINTF("M5:T386:S26:MODEL40-HDC-MEMORY-MEDIA:OK\n");
    return 0;
}
