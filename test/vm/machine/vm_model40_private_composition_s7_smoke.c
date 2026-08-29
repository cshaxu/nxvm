#include "type.h"

#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
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
    core_machine_speaker_observation speaker;
    type_unsigned_32 value = 0u;
    type_unsigned_8 rom_byte = 0u;
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    C_INT failed = 0;
    type_unsigned_8 fifo_count;

    even[0x3ff8u] = 0x26u;
    odd[0x3ff8u] = 0x90u;

    failed |= vm_session_create(&invalid_config, &session) !=
        TYPE_STATUS_INVALID_ARGUMENT || session != STD_NULL;
    if (!failed) failed |= vm_model40_fixture_create_bytes("t386-s7-even.bin", even,
        "dad19e3a911b717b0600bc2ed413d8070e46f4a9327556528c4bebcaf8bdcb89",
        "t386-s7-odd.bin", odd,
        "5e25cbc9cfc872eaff7a3426c04308a7b9f3dbb03ea130e38a017626b6d8208e",
        &session) !=
        TYPE_STATUS_OK || session == STD_NULL || !session->model40_private ||
        session->profile != STD_NULL ||
        session->core_machine->retirement_time_contract !=
            CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC ||
        session->core_machine->transaction_contract.external_cycle_timing.page_bytes != 2048u ||
        session->core_machine->transaction_contract.external_cycle_timing.page_miss_ticks != 2u ||
        session->core_machine->transaction_contract.external_cycle_timing.page_hit_ticks != 0u ||
        session->core_machine->transaction_contract.external_cycle_timing.first_eligible_address != 0u ||
        session->core_machine->transaction_contract.external_cycle_timing.last_eligible_address != 0x0009ffffu ||
        session->core_machine->transaction_contract.external_access_wait_windows[0].space !=
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT ||
        session->core_machine->transaction_contract.external_access_wait_windows[0].first_address != 0x03b4u ||
        session->core_machine->transaction_contract.external_access_wait_windows[2].last_address != 0x03dcu ||
        session->core_machine->transaction_contract.external_access_wait_windows[5].first_address != 0x0fc6u ||
        session->core_machine->transaction_contract.external_access_wait_windows[5].wait_ticks != 1u ||
        session->core_machine->transaction_contract.external_access_wait_windows[6].space != CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY ||
        session->core_machine->transaction_contract.external_access_wait_windows[6].first_address != 0x000a0000u ||
        session->core_machine->transaction_contract.external_access_wait_windows[6].last_address != 0x000affffu ||
        session->core_machine->transaction_contract.cpu_cycle_bus_ready_gate_enabled != TYPE_TRUE ||
        session->core_machine->transaction_contract.cpu_prefetch_reservation_enabled != TYPE_TRUE ||
        session->core_machine->transaction_contract.external_cycle_timing.overlap_policy !=
            CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL ||
        core_machine_get_cpu_profile(session->core_machine, &cpu_profile) !=
            TYPE_STATUS_OK || cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
            TYPE_STATUS_OK || memory_bytes != 2u * 1024u * 1024u ||
        session->core_machine->shared_rtc.registers[CORE_MACHINE_RTC_BASEMEM_LSB] != 0x80u ||
        session->core_machine->shared_rtc.registers[CORE_MACHINE_RTC_BASEMEM_MSB] != 0x02u ||
        session->core_machine->shared_rtc.registers[CORE_MACHINE_RTC_EXTMEM_LSB] != 0u ||
        session->core_machine->shared_rtc.registers[CORE_MACHINE_RTC_EXTMEM_MSB] != 0x04u ||
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
            sizeof(rom_byte)) != TYPE_STATUS_OK || rom_byte != 0x26u ||
        session->core_machine->shared_kbc.connect.aux_present ||
        core_machine_bus_write(session->core_machine, 0x0061u, 0x02u) !=
            TYPE_STATUS_OK || core_machine_get_speaker_observation(
            session->core_machine, &speaker) != TYPE_STATUS_OK ||
        !speaker.configured || speaker.timer_gate || !speaker.data_enabled ||
        !speaker.output || core_machine_bus_write(session->core_machine, 0x0061u,
            0x0fu) != TYPE_STATUS_OK ||
        STD_STRCMP(session->model40_resolved.identity,
            "compaq-deskpro-386-model-40") != 0 ||
        STD_STRCMP(session->model40_resolved.parent_identity, "pc-at-5170") != 0 ||
        session->model40_resolved.values.core.configuration.memory_bytes !=
            session->core_machine_config.memory_bytes ||
        session->model40_resolved.values.core.configuration.cpu_profile !=
            session->core_machine_config.cpu_profile ||
        session->core_machine->shared_kbc.data.aux_enabled ||
        (session->core_machine->shared_kbc.data.command_byte &
            CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u;
    if (!failed) {
        /* Model-40 selects the existing generic-AT 2-tick initial prefetch
         * locality miss in addition to the deterministic base instruction tick. */
        failed |= core_machine_run(session->core_machine, budget, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != 1u || result.ticks != 3u ||
            result.elapsed_ticks != 3u;
    }
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
    if (!failed) STD_PRINTF("M5:T386:S7:MODEL40-PRIVATE-COMPOSITION:OK\n");
    if (!failed) STD_PRINTF("M5:T421:S1:MODEL40-SPEAKER-SELECTION:OK\n");
    if (!failed) STD_PRINTF("M5:T386:S7:EXTERNAL-ROM-GUARD:OK\n");
    if (!failed) STD_PRINTF("M5:T390:S34:MODEL40-DETERMINISTIC-CONTRACT:OK\n");
    if (!failed) STD_PRINTF("M5:T477:S3:DESKPRO-SESSION-CUTOVER:OK\n");
    vm_session_destroy(session);
    vm_model40_fixture_remove("t386-s7-even.bin", "t386-s7-odd.bin");
    return failed ? 1 : 0;
}
