#include "../../../support/profile.h"
#include "app-nxvm/profiles/machine_factory_interface.h"
#include "../../../support/media.h"
#include "../../../../x86/core/composition_fixture.h"
#include "../../../../x86/core/time_fixture.h"
#include "../../../../x86/ibmpc-common/composition_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86/ibmpc-common/machine_board_interface.h"
#include "x86/product/machine/machine_private.h"
#include "app-nxvm/profiles/model40/model40_private.h"
#include "x86/product/machine/machine_interface.h"
#include "support/rom/model40_session_assets.h"

static lib_bool refresh_count_matches(core_machine *machine, lib_u16 expected)
{
    lib_u32 low, high;
    return core_machine_bus_write(machine, 0x0043u, 0x40u) == LIB_STATUS_OK &&
        core_machine_bus_read(machine, 0x0041u, &low) == LIB_STATUS_OK &&
        core_machine_bus_read(machine, 0x0041u, &high) == LIB_STATUS_OK &&
        (lib_u16)(low | (high << 8u)) == expected;
}

lib_i32 main(void)
{
    lib_u8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES] = {0};
    lib_u8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    lib_u8 video[VM_PROFILE_MODEL40_VIDEO_ROM_BYTES] = {0};
    lib_u8 cmos_seed[VM_MACHINE_CMOS_SEED_BYTES];
    vm_machine_config config = {0};
    vm_machine_assets assets = {0};
    vm_machine *session = LIB_NULL;
    vm_machine_reset_vector reset_vector = {0};
    core_machine_run_result result = {0};
    core_machine_time_observation time_observation = {0};
    lib_size memory_bytes = 0u;
    lib_size retained_memory_bytes;
    lib_u8 observed_memory = 0u;
    lib_u8 video_body_byte = 0u;
    lib_i32 failed = 1;

    lib_memory_set(odd, 1, sizeof(odd));
    video[0u] = 0x55u;
    video[1u] = 0xaau;
    video[2u] = 0x20u;
    video[sizeof(video) - 1u] = 0xe1u;
    config.profile_kind = VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.bios_count = 2u;
    vm_model40_fixture_cmos_seed(cmos_seed);
    assets.bios[0u] = (vm_machine_asset_bytes) { even, sizeof(even) };
    assets.bios[1u] = (vm_machine_asset_bytes) { odd, sizeof(odd) };
    assets.video = (vm_machine_asset_bytes) { video, sizeof(video) };
    assets.cmos_seed = (vm_machine_asset_bytes) { cmos_seed, sizeof(cmos_seed) };
    if (vm_test_machine_create_from_assets(&config, &assets, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || !vm_profile_machine_plan_is_model40(vm_test_profile_plan(session))) goto done;
    const test_board_plan_observation plan = test_board_capture_plan(session->core_machine_plan);
    if (plan.memory_bytes != 2u * 1024u * 1024u ||
        plan.retirement_time_contract !=
            CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC ||
        plan.l1_compatibility_policy !=
            CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS ||
        plan.cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        plan.fpu_profile != X86_FPU_PROFILE_NONE || !plan.cpu_80386_cr_mov_ignores_mod)
        goto done;
    const vm_profile_model40_external_rom *rom =
        vm_profile_machine_plan_model40_rom_get(vm_test_profile_plan(session));
    if (rom == LIB_NULL || rom->even_bytes == LIB_NULL || rom->even_bytes[0] != 0u ||
        rom->odd_bytes == LIB_NULL || rom->odd_bytes[0] != 1u ||
        rom->video_bytes == LIB_NULL || rom->video_bytes[0u] != 0x55u) goto done;
    if (core_machine_capture_time_observation(session->core_machine,
            &time_observation) != LIB_STATUS_OK || !time_observation.pacing_time_available ||
        time_observation.pacing_ticks_per_second != 16000000u || time_observation.physical_time_available ||
        time_observation.physical_ticks_per_second != 0u ||
        vm_test_fdd_info(session->fdd).geometry.sectors_per_track != 15u ||
        core_machine_memory_read(session->core_machine,
            VM_PROFILE_MODEL40_VIDEO_ROM_PHYSICAL_START, &observed_memory,
            sizeof(observed_memory)) != LIB_STATUS_OK || observed_memory != 0x55u ||
        !test_core_has_rom_mapping_start(session->core_machine,
        VM_PROFILE_MODEL40_VIDEO_ROM_COMPATIBILITY_ALIAS_START +
            VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES)) goto done;
    if (core_machine_memory_read(session->core_machine,
        VM_PROFILE_MODEL40_VIDEO_ROM_PHYSICAL_START +
            VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES, &video_body_byte,
        sizeof(video_body_byte)) != LIB_STATUS_OK || core_machine_memory_read(session->core_machine,
        VM_PROFILE_MODEL40_VIDEO_ROM_COMPATIBILITY_ALIAS_START +
            VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES, &observed_memory,
        sizeof(observed_memory)) != LIB_STATUS_OK || observed_memory != video_body_byte ||
        vm_machine_get_reset_vector(session, &reset_vector) != LIB_STATUS_OK ||
        reset_vector.cs != 0xf000u || reset_vector.ip != 0xfff0u) goto done;
    retained_memory_bytes = session->construction.core_config.memory_bytes;
    if (vm_machine_reconfigure_memory(session, 2u * 1024u * 1024u) !=
        LIB_STATUS_INVALID_STATE || core_machine_get_memory_bytes(session->core_machine,
        &memory_bytes) != LIB_STATUS_OK || memory_bytes != 2u * 1024u * 1024u ||
        session->construction.core_config.memory_bytes != retained_memory_bytes ||
        core_machine_run(session->core_machine,
        (core_machine_run_budget) {1u, 0u}, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        core_machine_reset(session->core_machine) != LIB_STATUS_OK ||
        test_core_machine_advance_time(session->core_machine, 1u) != LIB_STATUS_OK ||
        !refresh_count_matches(session->core_machine, 18u) ||
        vm_machine_get_reset_vector(session, &reset_vector) != LIB_STATUS_OK ||
        reset_vector.cs != 0xf000u || reset_vector.ip != 0xfff0u) goto done;
    even[0u] = 2u;
    if (rom->even_bytes[0] != 0u || rom->odd_bytes[0] != 1u) goto done;
    vm_machine_destroy(session);
    session = LIB_NULL;
    config.memory_bytes = 2u * 1024u * 1024u;
    if (vm_test_machine_create_from_assets(&config, &assets, &session) != LIB_STATUS_INVALID_ARGUMENT ||
        session != LIB_NULL) goto done;
    config.memory_bytes = 0u;
    failed = 0;
done:
    vm_machine_destroy(session);
    if (!failed) printf("M5:T386:S20:MODEL40-BYOB-MANIFEST:OK\nM5:T386:S20:MODEL40-BYOB-VALIDATION:OK\nM5:T386:S20:MODEL40-PUBLIC-COMPOSITION:OK\nM5:T424:S1:MODEL40-BYOB-RESET-LIFECYCLE:OK\nM5:T440:S1:MODEL40-IMMUTABLE-CONFIGURATION:OK\n");
    return failed;
}
