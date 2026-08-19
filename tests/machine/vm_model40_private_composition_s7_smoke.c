#include "type.h"

#include "vm/composition/session/session.h"
#include "vm/composition/session/session_interface.h"
#include "core/machine/kbc.h"
#include "core/machine/machine.h"
#include "core/machine/port.h"

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
    core_machine_speaker_observation speaker;
    type_unsigned_32 value = 0u;
    type_unsigned_8 rom_byte = 0u;
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    C_INT failed = 0;
    type_unsigned_8 fifo_count;

    even[0x3ff8u] = 0x26u;
    odd[0x3ff8u] = 0x90u;

    failed |= vm_session_create_model40_private(&invalid, &session) !=
        TYPE_STATUS_INVALID_ARGUMENT || session != STD_NULL;
    if (!failed) failed |= vm_session_create_model40_private(&valid, &session) !=
        TYPE_STATUS_OK || session == STD_NULL || !session->model40_private ||
        session->profile != STD_NULL ||
        session->core_machine->retirement_time_contract !=
            CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC ||
        session->core_machine->external_cycle_timing.page_bytes != 2048u ||
        session->core_machine->external_cycle_timing.page_miss_ticks != 2u ||
        session->core_machine->external_cycle_timing.page_hit_ticks != 0u ||
        session->core_machine->cpu_cycle_bus_ready_gate_enabled != TYPE_TRUE ||
        session->core_machine->cpu_prefetch_reservation_enabled != TYPE_TRUE ||
        session->core_machine->external_cycle_timing.overlap_policy !=
            CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL ||
        core_machine_get_cpu_profile(session->core_machine, &cpu_profile) !=
            TYPE_STATUS_OK || cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
            TYPE_STATUS_OK || memory_bytes != 1024u * 1024u ||
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
    if (!failed) STD_PRINTF("M5:T386:S7:MODEL40-PRIVATE-COMPOSITION:OK\n");
    if (!failed) STD_PRINTF("M5:T421:S1:MODEL40-SPEAKER-SELECTION:OK\n");
    if (!failed) STD_PRINTF("M5:T386:S7:EXTERNAL-ROM-GUARD:OK\n");
    if (!failed) STD_PRINTF("M5:T390:S34:MODEL40-DETERMINISTIC-CONTRACT:OK\n");
    vm_session_destroy(session);
    return failed ? 1 : 0;
}
