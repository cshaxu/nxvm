#include "app-mydeskpro386/profiles/observation_interface.h"
#include "../../../app-nxvm/unit/support/profile.h"
#include "ibmpc/machine/machine_interface.h"
#include "../../../app-nxvm/unit/support/ibmpc/machine/support/media.h"
#include "../../../ibmpc/core/composition_fixture.h"
#include "../../../ibmpc/core/time_fixture.h"
#include "../../../app-nxvm/unit/support/ibmpc/board-common/composition_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/machine/machine_private.h"
#include "app-mydeskpro386/profiles/model40_private.h"
#include "ibmpc/machine/machine_interface.h"
#include "../../support/rom/model40_session_assets.h"

static lib_bool refresh_count_matches(core_machine *machine, lib_u16 expected)
{
    lib_u32 low, high;
    return core_machine_bus_write(machine, 0x0043u, 0x40u) == LIB_STATUS_OK &&
        core_machine_bus_read(machine, 0x0041u, &low) == LIB_STATUS_OK &&
        core_machine_bus_read(machine, 0x0041u, &high) == LIB_STATUS_OK &&
        (lib_u16)(low | (high << 8u)) == expected;
}

static lib_bool short_video_copy_is_bounded(void)
{
    static lib_u8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static lib_u8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    lib_u8 video[VM_PROFILE_MODEL40_VIDEO_ROM_BYTES];
    const vm_machine_config config = {.bios_count = 2u};
    vm_machine_assets assets = {.bios = {{even, sizeof(even)}, {odd, sizeof(odd)}},
        .video = {video, 512u}};
    vm_machine_construction construction = {0};
    lib_bool valid;

    lib_memory_set(video, 0xcc, sizeof(video));
    lib_memory_set(video, 0, assets.video.bytes);
    video[0] = 0x55u;
    video[1] = 0xaau;
    video[2] = 1u;
    if (vm_profile_machine_plan_create_model40(&config, &assets, &construction) != LIB_STATUS_OK)
        return LIB_FALSE;
    const vm_profile_model40_external_rom *rom = vm_test_profile_model40_rom(&construction);
    valid = rom != LIB_NULL && rom->video_byte_count == assets.video.bytes &&
        lib_memory_compare(rom->video_bytes, video, assets.video.bytes) == 0;
    /* The real candidate owns the maximum-size zeroed video buffer. Bytes
     * beyond the declared source must not be read/copied into its tail. */
    for (lib_size index = assets.video.bytes; valid && index < sizeof(video); ++index)
        valid = rom->video_bytes[index] == 0u;
    construction.profile.release(construction.profile.context);
    return valid;
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
    config.bios_count = 2u;
    vm_model40_fixture_cmos_seed(cmos_seed);
    assets.bios[0u] = (vm_machine_asset_bytes) { even, sizeof(even) };
    assets.bios[1u] = (vm_machine_asset_bytes) { odd, sizeof(odd) };
    assets.video = (vm_machine_asset_bytes) { video, sizeof(video) };
    assets.cmos_seed = (vm_machine_asset_bytes) { cmos_seed, sizeof(cmos_seed) };
    if (vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        &config, &assets, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || !vm_test_profile_is_model40(vm_test_profile_construction(session))) goto done;
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
        vm_test_profile_model40_rom(vm_test_profile_construction(session));
    if (rom == LIB_NULL || rom->even_bytes == LIB_NULL || rom->even_bytes[0] != 0u ||
        rom->odd_bytes == LIB_NULL || rom->odd_bytes[0] != 1u ||
        rom->video_bytes == LIB_NULL || rom->video_bytes[0u] != 0x55u) goto done;
    if (core_machine_capture_time_observation(session->core_machine,
            &time_observation) != LIB_STATUS_OK || !time_observation.pacing_time_available ||
        time_observation.pacing_ticks_per_second != 16000000u || time_observation.physical_time_available ||
        time_observation.physical_ticks_per_second != 0u ||
        vm_test_fdd_info(session->floppy[0u]).geometry.sectors_per_track != 15u ||
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
    if (vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        &config, &assets, &session) != LIB_STATUS_INVALID_ARGUMENT ||
        session != LIB_NULL) goto done;
    config.memory_bytes = 0u;
    failed = !short_video_copy_is_bounded();
done:
    vm_machine_destroy(session);
    if (!failed) printf("M5:T386:S20:MODEL40-BYOB-MANIFEST:OK\nM5:T386:S20:MODEL40-BYOB-VALIDATION:OK\nM5:T386:S20:MODEL40-PUBLIC-COMPOSITION:OK\nM5:T424:S1:MODEL40-BYOB-RESET-LIFECYCLE:OK\nM5:T440:S1:MODEL40-IMMUTABLE-CONFIGURATION:OK\n");
    return failed;
}
