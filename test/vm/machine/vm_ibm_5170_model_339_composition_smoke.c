#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/port.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "vm/profile/default_profile/pc_at_profile.h"

#include "../support/rom/session_assets.h"

static type_status vm_test_create_5170(const vm_session_config *config,
    vm_session **out_session)
{
    type_unsigned_8 even[VM_SESSION_PC_AT_ROM_CHIP_BYTES];
    type_unsigned_8 odd[VM_SESSION_PC_AT_ROM_CHIP_BYTES];
    vm_session_assets assets;

    vm_test_ibm_5170_assets(&assets, even, odd);
    return vm_session_create_from_assets(config, &assets, out_session);
}

static type_status vm_test_create_default(const vm_session_config *config,
    vm_session **out_session)
{
    type_unsigned_8 rom[VM_SESSION_PC_AT_ROM_BYTES];
    vm_session_assets assets;

    vm_test_default_pc_at_assets(&assets, rom);
    return vm_session_create_from_assets(config, &assets, out_session);
}

static C_INT vm_model_339_selected_contract(C_VOID)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_ibm_5170_model_339_descriptor_get();
    const vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339,
        .bios_count = 2u
    };
    core_machine_cpu_profile cpu_profile;
    core_machine_planar_parity_observation parity;
    core_machine_speaker_observation speaker;
    core_machine_memory_route memory_route;
    vm_session *session = STD_NULL;
    STD_SIZE_T memory_bytes = 0u;
    type_unsigned_8 before = 0u;
    type_unsigned_8 after = 0u;
    type_unsigned_8 option_rom_probe = 0u;
    C_INT failed = 0;

    if (profile == STD_NULL || !vm_profile_default_pc_at_descriptor_is_valid(profile) ||
        !profile->hdc_present || !profile->planar_parity_present ||
        !profile->monochrome_aperture_absent || profile->diskette_drive_a_field_upgrade ||
        profile->clock_plan.pit.numerator != 596591u ||
        profile->clock_plan.pit.denominator != 4000000u ||
        profile->pic_irq_timing.unmask_delivery_ticks[1u] != 120u ||
        profile->cmos.floppy_type != 0x20u || profile->cmos.fixed_disk_type != 0x30u ||
        vm_profile_default_pc_at_port_leaf_find(profile,
            VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f0u) == STD_NULL ||
        vm_test_create_5170(&config, &session) != TYPE_STATUS_OK || session == STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }
    failed |= (STD_STRCMP(session->profile->identity, "pc-at-5170") != 0 ||
        !vm_profile_default_pc_at_descriptor_is_valid(session->profile)) ? 0x0001 : 0;
    failed |= (session->core_machine->transaction_contract.cpu_cycle_bus_ready_gate_enabled ||
        session->core_machine->transaction_contract.cpu_prefetch_reservation_enabled ||
        session->core_machine->transaction_contract.external_cycle_timing.page_bytes != 0u ||
        session->core_machine->transaction_contract.external_cycle_timing.page_miss_ticks != 0u ||
        session->core_machine->transaction_contract.external_cycle_timing.page_hit_ticks != 0u ||
        session->core_machine->transaction_contract.external_cycle_timing.overlap_policy !=
            CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED ||
        session->core_machine->transaction_contract.external_access_wait_windows[0].wait_ticks != 0u) ?
        0x0002 : 0;
    failed |= (core_machine_bus_write(session->core_machine, CORE_MACHINE_PC_AT_PORT_B,
        0x02u) != TYPE_STATUS_OK || core_machine_get_speaker_observation(
        session->core_machine, &speaker) != TYPE_STATUS_OK || !speaker.configured ||
        speaker.timer_gate || !speaker.data_enabled || !speaker.output) ? 0x0004 : 0;
    failed |= (core_machine_get_cpu_profile(session->core_machine, &cpu_profile) !=
        TYPE_STATUS_OK || cpu_profile != CORE_MACHINE_CPU_PROFILE_80286) ? 0x0008 : 0;
    failed |= (session->core_machine->shared_kbc.connect.aux_present ||
        session->core_machine->shared_kbc.data.aux_enabled ||
        (session->core_machine->shared_kbc.data.command_byte &
            CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u) ? 0x0010 : 0;
    failed |= (session->core_machine->shared_kbc.data.output_port != 0x03u ||
        !session->core_machine->executor_memory.data.flagA20) ? 0x0011 : 0;
    failed |= session->core_machine->shared_pic_master.data.unmask_delivery_ticks[1u] !=
        120u ? 0x0012 : 0;
    failed |= (core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
        TYPE_STATUS_OK || memory_bytes != 512u * 1024u) ? 0x0020 : 0;
    failed |= (core_machine_get_planar_parity_observation(session->core_machine,
        &parity) != TYPE_STATUS_OK || !parity.configured) ? 0x0040 : 0;
    failed |= core_machine_set_a20(session->core_machine, 1) != TYPE_STATUS_OK ? 0x0080 : 0;
    failed |= (core_machine_memory_query(session->core_machine, 0x00100003u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &memory_route) != TYPE_STATUS_OK ||
        memory_route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER) ? 0x0100 : 0;
    failed |= (core_machine_memory_read(session->core_machine, 0x00100003u, &before,
        sizeof(before)) != TYPE_STATUS_OK || before != 0xffu) ? 0x0200 : 0;
    failed |= (core_machine_memory_write(session->core_machine, 0x00100003u,
        &(type_unsigned_8){0x5au}, sizeof(type_unsigned_8)) != TYPE_STATUS_OK) ? 0x0400 : 0;
    failed |= (core_machine_memory_read(session->core_machine, 0x00100003u, &after,
        sizeof(after)) != TYPE_STATUS_OK || after != 0xffu) ? 0x0800 : 0;
    failed |= (core_machine_memory_read(session->core_machine, 0x000c0000u,
        &option_rom_probe, sizeof(option_rom_probe)) != TYPE_STATUS_OK ||
        option_rom_probe != 0xffu) ? 0x0900 : 0;
    failed |= (!core_machine_port_has_read(&session->core_machine->executor_port, 0x01f0u) ||
        !core_machine_port_has_write(&session->core_machine->executor_port, 0x01f0u) ||
        core_machine_port_read(&session->core_machine->executor_port, 0x03f1u) != 0x50u ||
        session->core_machine->hdc.connect.config.service.command_ticks != 16000u ||
        session->core_machine->hdc.connect.config.service.next_sector_ticks != 7840u) ? 0x1000 : 0;
    vm_session_destroy(session);
    return failed;
}

static C_INT vm_model_339_floppy_contract(C_VOID)
{
    const vm_session_config native = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339, .bios_count = 2u
    };
    const vm_session_config compatible = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339, .bios_count = 2u,
        .floppy_format = VM_SESSION_FLOPPY_FORMAT_360K
    };
    const vm_session_config rejected = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339, .bios_count = 2u,
        .floppy_format = VM_SESSION_FLOPPY_FORMAT_720K
    };
    vm_session *session = STD_NULL;
    C_INT failed = vm_test_create_5170(&native, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || session->floppy_kind != VM_PROFILE_FLOPPY_525_1200K ||
        session->fdd.data.ncyl != 80u || session->fdd.data.nhead != 2u ||
        session->fdd.data.nsector != 15u;

    vm_session_destroy(session);
    session = STD_NULL;
    failed |= vm_test_create_5170(&compatible, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || session->floppy_kind != VM_PROFILE_FLOPPY_525_1200K ||
        session->fdd_media_kind != VM_PROFILE_FLOPPY_525_360K ||
        session->fdd.data.ncyl != 40u || session->fdd.data.nhead != 2u ||
        session->fdd.data.nsector != 9u || session->profile->cmos.floppy_type != 0x20u ||
        session->core_machine->fdc.connect.drives.cylinder_count[0u] != 80u;
    vm_session_destroy(session);
    session = STD_NULL;
    failed |= vm_test_create_5170(&rejected, &session) == TYPE_STATUS_OK || session != STD_NULL;
    vm_session_destroy(session);
    return failed;
}

/* The 5170 BIOS POST polls Port B bit 4 while it waits for the system PIT
 * refresh counter.  Keep that board-level edge observable through the same
 * CPU retirement path which real firmware uses; a direct advance is not
 * permitted on this profile's physical Core axis. */
static C_INT vm_model_339_refresh_polling_is_live(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0xb4u, 0x10u, 0xe4u, 0x61u, 0x24u, 0x10u,
        0x3au, 0xc4u, 0x74u, 0xf8u, 0xf4u
    };
    const vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339, .bios_count = 2u
    };
    core_machine_run_result result = {0};
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    if (vm_test_create_5170(&config, &session) != TYPE_STATUS_OK || session == STD_NULL ||
        core_machine_memory_write(session->core_machine, 0x0500u, program,
            sizeof(program)) != TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return 1;
    }
    session->core_machine->executor_cpu.data.cs.selector = 0u;
    session->core_machine->executor_cpu.data.cs.base = 0u;
    session->core_machine->executor_cpu.data.ds.selector = 0u;
    session->core_machine->executor_cpu.data.ds.base = 0u;
    session->core_machine->executor_cpu.data.es.selector = 0u;
    session->core_machine->executor_cpu.data.es.base = 0u;
    session->core_machine->executor_cpu.data.ss.selector = 0u;
    session->core_machine->executor_cpu.data.ss.base = 0u;
    session->core_machine->executor_cpu.data.eip = 0x0500u;
    session->core_machine->executor_cpu.data.sp = 0xfffeu;
    session->core_machine->executor_cpu.data.flagHalt = TYPE_FALSE;
    failed = core_machine_run(session->core_machine, (core_machine_run_budget) {1000u, 0u},
        &result) != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    vm_session_destroy(session);
    return failed;
}

/* The IBM Rev-3 POST measures the same port-61h refresh waveform rather than
 * merely waiting for one low sample.  Keep the public 80286 execution path
 * and the copied 5170 board clock together: a direct PIT advance would evade
 * the CPU/PIT ratio that this firmware test actually exercises. */
static C_INT vm_model_339_refresh_post_loop_is_calibrated(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0x32u, 0xdbu, 0x33u, 0xc9u, 0x90u,
        0xe4u, 0x61u, 0xa8u, 0x10u, 0xe1u, 0xfau,
        0xe4u, 0x61u, 0xa8u, 0x10u, 0xe0u, 0xfau,
        0xfeu, 0xcbu, 0x75u, 0xf0u, 0xf4u
    };
    const vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339, .bios_count = 2u
    };
    core_machine_run_result result = {0};
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    if (vm_test_create_5170(&config, &session) != TYPE_STATUS_OK || session == STD_NULL ||
        core_machine_memory_write(session->core_machine, 0x0500u, program,
            sizeof(program)) != TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return 1;
    }
    session->core_machine->executor_cpu.data.cs.selector = 0u;
    session->core_machine->executor_cpu.data.cs.base = 0u;
    session->core_machine->executor_cpu.data.ds.selector = 0u;
    session->core_machine->executor_cpu.data.ds.base = 0u;
    session->core_machine->executor_cpu.data.es.selector = 0u;
    session->core_machine->executor_cpu.data.es.base = 0u;
    session->core_machine->executor_cpu.data.ss.selector = 0u;
    session->core_machine->executor_cpu.data.ss.base = 0u;
    session->core_machine->executor_cpu.data.eip = 0x0500u;
    session->core_machine->executor_cpu.data.sp = 0xfffeu;
    session->core_machine->executor_cpu.data.flagHalt = TYPE_FALSE;
    failed = core_machine_run(session->core_machine,
        (core_machine_run_budget) {200000u, 0u}, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        session->core_machine->executor_cpu.data.cx < 0xf600u;
    if (failed) STD_PRINTF("M5:T516:S2:MODEL339-REFRESH:CX=%04X:EIP=%04X:reason=%u\n",
        session->core_machine->executor_cpu.data.cx,
        session->core_machine->executor_cpu.data.eip, result.reason);
    vm_session_destroy(session);
    return failed;
}

/* The IBM 5170 POST's 106 check writes AA55h to the adjacent byte-wide DMA
 * page latches at 82h/83h, then reads them back as individual bytes.  This
 * is a CPU-to-Core board path check, not a firmware shortcut. */
static C_INT vm_model_339_dma_page_word_io_is_converted(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0xb8u, 0x55u, 0xaau, 0xe7u, 0x82u,
        0xe4u, 0x82u, 0x86u, 0xc4u, 0xe4u, 0x83u, 0xf4u
    };
    const vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339, .bios_count = 2u
    };
    core_machine_run_result result = {0};
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    if (vm_test_create_5170(&config, &session) != TYPE_STATUS_OK || session == STD_NULL ||
        core_machine_memory_write(session->core_machine, 0x0500u, program,
            sizeof(program)) != TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return 1;
    }
    session->core_machine->executor_cpu.data.cs.selector = 0u;
    session->core_machine->executor_cpu.data.cs.base = 0u;
    session->core_machine->executor_cpu.data.eip = 0x0500u;
    session->core_machine->executor_cpu.data.flagHalt = TYPE_FALSE;
    failed = core_machine_run(session->core_machine, (core_machine_run_budget) {64u, 0u},
        &result) != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        session->core_machine->executor_cpu.data.ax != 0x55aau;
    vm_session_destroy(session);
    return failed;
}

static C_INT vm_model_339_external_rom_route(C_VOID)
{
    type_unsigned_8 even[VM_SESSION_PC_AT_ROM_CHIP_BYTES];
    type_unsigned_8 odd[VM_SESSION_PC_AT_ROM_CHIP_BYTES];
    type_unsigned_8 video[512] = {0x55u, 0xaau, 1u};
    vm_session_assets assets;
    const vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339, .bios_count = 2u
    };
    vm_session *session = STD_NULL;
    type_unsigned_8 observed[2] = {0};
    C_INT failed;

    vm_test_ibm_5170_assets(&assets, even, odd);
    even[0u] = 0x12u;
    odd[0u] = 0x34u;
    assets.video = (vm_session_asset_bytes) { video, sizeof(video) };
    failed = vm_session_create_from_assets(&config, &assets, &session) !=
            TYPE_STATUS_OK || session == STD_NULL || !session->pc_at_rom_external ||
        core_machine_memory_read(session->core_machine, 0x000f0000u, observed,
            sizeof(observed)) != TYPE_STATUS_OK || observed[0u] != 0x12u ||
            observed[1u] != 0x34u ||
        core_machine_memory_read(session->core_machine, 0x000c0000u, observed,
            sizeof(observed)) != TYPE_STATUS_OK || observed[0u] != 0x55u ||
            observed[1u] != 0xaau;
    vm_session_destroy(session);
    return failed;
}

C_INT main(C_VOID)
{
    const vm_session_config default_config = {
        .profile_kind = VM_SESSION_PROFILE_DEFAULT_PC_AT, .bios_count = 1u
    };
    vm_session *default_session = STD_NULL;
    const C_INT selected = vm_model_339_selected_contract();
    const C_INT floppy = vm_model_339_floppy_contract();
    const C_INT refresh = vm_model_339_refresh_polling_is_live();
    const C_INT refresh_post = vm_model_339_refresh_post_loop_is_calibrated();
    const C_INT dma_word_io = vm_model_339_dma_page_word_io_is_converted();
    const C_INT rom = vm_model_339_external_rom_route();
    const C_INT default_create = vm_test_create_default(&default_config,
        &default_session) != TYPE_STATUS_OK || default_session == STD_NULL ||
        !default_session->profile->hdc_present;
    C_INT failed = selected || floppy || refresh || refresh_post || dma_word_io || rom || default_create;

    vm_session_destroy(default_session);
    if (failed) return 1;
    STD_PRINTF("M5:T515:UNIT:MODEL339-COMPOSITION:OK\n");
    return 0;
}
