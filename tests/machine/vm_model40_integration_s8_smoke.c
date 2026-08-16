#include "type.h"

#include "vm/composition/session/session.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/lifecycle.h"
#include "core/machine/fdc.h"
#include "core/machine/hdc.h"
#include "core/machine/kbc.h"
#include "core/machine/machine.h"
#include "core/machine/port.h"

static C_INT vm_model40_fdc_sense_reset(core_machine *machine,
    type_unsigned_8 expected_status)
{
    type_unsigned_8 status;
    type_unsigned_8 cylinder;

    core_machine_port_write(&machine->executor_port, 0x03f5u, 0x08u);
    core_machine_fdc_advance(&machine->fdc);
    status = (type_unsigned_8)core_machine_port_read(&machine->executor_port,
        0x03f5u);
    cylinder = (type_unsigned_8)core_machine_port_read(&machine->executor_port,
        0x03f5u);
    return status == expected_status && cylinder == 0u &&
        !machine->fdc.connect.irq_source.asserted;
}

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_profile_model40_external_rom invalid = { even, odd,
        VM_PROFILE_MODEL40_ROM_CHIP_BYTES - 1u };
    vm_profile_model40_external_rom valid = { even, odd, sizeof(even) };
    vm_session *session = STD_NULL;
    core_machine_cpu_profile cpu_profile;
    STD_SIZE_T memory_bytes;
    core_machine_d4_platform_observation d4;
    type_unsigned_32 value = 0u;
    type_unsigned_8 rom_byte = 0u;
    type_unsigned_8 sense_status;
    C_INT failed = 0;
    type_unsigned_8 fifo_count;
    type_unsigned_8 index;

    even[0x3ff8u] = 0xa5u;

    failed |= vm_session_create_model40_private(&invalid, &session) !=
        TYPE_STATUS_INVALID_ARGUMENT || session != STD_NULL;
    if (!failed) failed |= vm_session_create_model40_private(&valid, &session) !=
        TYPE_STATUS_OK || session == STD_NULL || !session->model40_private ||
        session->profile != STD_NULL ||
        core_machine_get_cpu_profile(session->core_machine, &cpu_profile) !=
            TYPE_STATUS_OK || cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
            TYPE_STATUS_OK || memory_bytes != 1024u * 1024u ||
        core_machine_get_d4_platform_observation(session->core_machine, &d4) !=
            TYPE_STATUS_OK || !d4.configured || !d4.iochk_enabled ||
        d4.failsafe_enabled ||
        core_machine_bus_read(session->core_machine, 0x07c6u, &value) !=
            TYPE_STATUS_OK || value != 0u ||
        core_machine_bus_read(session->core_machine, 0x0bc6u, &value) !=
            TYPE_STATUS_OK || value != 0x30u ||
        core_machine_bus_read(session->core_machine, 0x0fc6u, &value) !=
            TYPE_STATUS_OK || value != 0x01u ||
        core_machine_bus_read(session->core_machine, 0x0061u, &value) !=
            TYPE_STATUS_OK || value != 0x0bu ||
        core_machine_memory_read(session->core_machine, 0x000ffff0u, &rom_byte,
            sizeof(rom_byte)) != TYPE_STATUS_OK || rom_byte != 0xa5u ||
        session->core_machine->shared_kbc.connect.aux_present ||
        session->core_machine->shared_kbc.data.aux_enabled ||
        (session->core_machine->shared_kbc.data.command_byte &
            CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u;
    if (!failed) {
        core_platform_input_event event = {0};

        event.kind = CORE_PLATFORM_INPUT_RELATIVE_MOUSE;
        event.data.relative_mouse.delta_x = 1;
        event.data.relative_mouse.delta_y = 1;
        event.data.relative_mouse.buttons = 1u;
        fifo_count = session->core_machine->shared_kbc.data.fifo_count;
        failed |= vm_session_submit_host_input(session, &event) != TYPE_STATUS_OK;
        vm_platform_request_transport_observe_execution_boundary(
            &session->request_transport);
        failed |= session->core_machine->shared_kbc.data.fifo_count != fifo_count;
        core_machine_port_write(&session->core_machine->executor_port,
            0x0064u, 0xa8u);
        failed |= session->core_machine->shared_kbc.data.aux_enabled ||
            (session->core_machine->shared_kbc.data.command_byte &
                CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u;
        core_machine_port_write(&session->core_machine->executor_port,
            0x0060u, 0xf5u);
        core_machine_port_write(&session->core_machine->executor_port,
            0x0064u, 0xd4u);
        core_machine_port_write(&session->core_machine->executor_port,
            0x0060u, 0xf4u);
        failed |= session->core_machine->shared_kbc.data.scanning_enabled ||
            session->core_machine->shared_kbc.data.pending_write !=
                CORE_MACHINE_KBC_PENDING_NONE;
    }
    if (!failed) {
        vm_session_reset(session);
        failed |= !session->core_machine->auxiliary_pit_configured ||
            !session->core_machine->fdc_configured ||
            session->core_machine->fdc_topology.config.irq != 6u ||
            session->core_machine->fdc_topology.config.dma_channel != 2u ||
            !session->core_machine->hdc_configured ||
            session->core_machine->hdc_topology.config.irq != 14u ||
            session->core_machine->hdc_topology.config.protocol !=
                CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB ||
            !session->core_machine->rtc_cmos_configured ||
            session->core_machine->rtc_cmos_config.irq != 8u ||
            !core_machine_port_has_read(&session->core_machine->executor_port,
                0x03f7u) || !core_machine_port_has_write(
                &session->core_machine->executor_port, 0x004bu);
    }
    if (!failed) {
        core_machine_port_write(&session->core_machine->executor_port,
            0x03f2u, 0u);
        core_machine_port_write(&session->core_machine->executor_port,
            0x03f2u, 0x1cu);
        failed |= !session->core_machine->fdc.connect.irq_source.asserted;
        for (index = 0u; index < CORE_MACHINE_FDC_DRIVE_COUNT; ++index) {
            failed |= !vm_model40_fdc_sense_reset(session->core_machine,
                (type_unsigned_8)(core_machine_fdc_ST0_READY_CHANGE | index));
        }
        core_machine_port_write(&session->core_machine->executor_port,
            0x03f5u, 0x08u);
        core_machine_fdc_advance(&session->core_machine->fdc);
        sense_status = (type_unsigned_8)core_machine_port_read(
            &session->core_machine->executor_port, 0x03f5u);
        failed |= sense_status != 0x80u;
    }
    if (!failed) {
        core_machine_port_write(&session->core_machine->executor_port,
            0x01f6u, 0x2au);
        core_machine_port_write(&session->core_machine->executor_port,
            0x01f7u, 0x90u);
        core_machine_hdc_advance(&session->core_machine->hdc);
        failed |= session->core_machine->hdc.data.error != 0x01u ||
            !core_machine_hdc_irq_pending(&session->core_machine->hdc);
        (C_VOID)core_machine_port_read(&session->core_machine->executor_port,
            0x01f7u);
        failed |= core_machine_hdc_irq_pending(&session->core_machine->hdc);
        core_machine_port_write(&session->core_machine->executor_port,
            0x01f7u, 0xecu);
        core_machine_hdc_advance(&session->core_machine->hdc);
        failed |= (core_machine_port_read(&session->core_machine->executor_port,
            0x01f7u) & CORE_MACHINE_HDC_STATUS_ERR) == 0u ||
            core_machine_port_read(&session->core_machine->executor_port,
                0x01f1u) != CORE_MACHINE_HDC_ERROR_ABORT;
    }
    if (!failed) STD_PRINTF("M5:T386:S8:MODEL40-INTEGRATION:OK\n");
    if (!failed) STD_PRINTF("M5:T386:S8:MODEL40-CONTROLS:OK\n");
    vm_session_destroy(session);
    return failed ? 1 : 0;
}
