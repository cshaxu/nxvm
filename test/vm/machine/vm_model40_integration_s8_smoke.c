#include "type.h"

#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/lifecycle.h"
#include "core/machine/fdc.h"
#include "core/machine/hdc.h"
#include "core/machine/kbc.h"
#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "../support/vm_model40_byob_fixture.h"

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_session_config invalid_config = {
        .profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40
    };
    vm_session *session = STD_NULL;
    core_machine_cpu_profile cpu_profile;
    STD_SIZE_T memory_bytes;
    core_machine_d4_platform_observation d4;
    type_unsigned_32 value = 0u;
    type_unsigned_8 rom_byte = 0u;
    type_unsigned_8 sense_status;
    C_INT failed = 0;
    type_unsigned_8 fifo_count;

    even[0x3ff8u] = 0xa5u;

    failed |= vm_session_create(&invalid_config, &session) !=
        TYPE_STATUS_INVALID_ARGUMENT || session != STD_NULL;
    if (!failed) failed |= vm_model40_fixture_create_bytes("t386-s8-even.bin", even,
        "a5059e5343dc620dfdc23ebb0477362bffa4296960c052305c64258b1a4da245",
        "t386-s8-odd.bin", odd,
        "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe",
        &session) !=
        TYPE_STATUS_OK || session == STD_NULL || !session->model40_private ||
        session->profile != STD_NULL ||
        core_machine_get_cpu_profile(session->core_machine, &cpu_profile) !=
            TYPE_STATUS_OK || cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
            TYPE_STATUS_OK || memory_bytes != 2u * 1024u * 1024u ||
        core_machine_get_d4_platform_observation(session->core_machine, &d4) !=
            TYPE_STATUS_OK || !d4.configured || d4.iochk_enabled ||
        d4.failsafe_enabled ||
        core_machine_bus_read(session->core_machine, 0x07c6u, &value) !=
            TYPE_STATUS_OK || value != 0u ||
        core_machine_bus_read(session->core_machine, 0x0bc6u, &value) !=
            TYPE_STATUS_OK || value != 0x30u ||
        core_machine_bus_read(session->core_machine, 0x0fc6u, &value) !=
            TYPE_STATUS_OK || value != 0x01u ||
        core_machine_bus_read(session->core_machine, 0x0061u, &value) !=
            TYPE_STATUS_OK || value != 0x1fu ||
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
            session->request_transport);
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
        core_machine_fdc_advance_at(&session->core_machine->fdc,
            session->core_machine->fdc.data.reset_due_tick);
        failed |= session->core_machine->fdc.connect.irq_source.asserted;
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
    vm_model40_fixture_remove("t386-s8-even.bin", "t386-s8-odd.bin");
    return failed ? 1 : 0;
}
