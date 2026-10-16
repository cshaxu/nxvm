#include "app-mydeskpro386/machine_observation_interface.h"
#include "../support/profile.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "../../x86-nxvm/machine/support/media.h"
#include "../../x86-nxvm/board-base/support/composition_fixture.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "app-mydeskpro386/machine_config.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "../support/model40_session_assets.h"

static lib_bool refresh_count_matches(x86_core_machine *machine, lib_u16 expected)
{
    lib_u32 low, high;
    return x86_core_machine_bus_write(machine, 0x0043u, 0x40u) == LIB_STATUS_OK &&
        x86_core_machine_bus_read(machine, 0x0041u, &low) == LIB_STATUS_OK &&
        x86_core_machine_bus_read(machine, 0x0041u, &high) == LIB_STATUS_OK &&
        (lib_u16)(low | (high << 8u)) == expected;
}

static lib_bool advance_to_first_pit_edge(x86_core_machine *machine, lib_u64 target)
{
    x86_core_machine_time_observation time = {0};

    if (x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
        time.elapsed_ticks != 0u) return LIB_FALSE;
    while (time.elapsed_ticks < target) {
        lib_u8 advanced = LIB_FALSE;
        const lib_u64 before = time.elapsed_ticks;

        if (!time.next_deadline_valid || time.next_deadline_tick <= before ||
            time.next_deadline_tick > target ||
            x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
            !advanced || x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
            time.elapsed_ticks <= before || time.elapsed_ticks > target) return LIB_FALSE;
    }
    return time.elapsed_ticks == target;
}

static lib_bool short_video_copy_is_bounded(void)
{
    static lib_u8 even[MYDESKPRO386_ROM_CHIP_BYTES];
    static lib_u8 odd[MYDESKPRO386_ROM_CHIP_BYTES];
    lib_u8 video[MYDESKPRO386_VIDEO_ROM_BYTES];
    const x86_nxvm_machine_config config = {.bios_count = 2u};
    x86_nxvm_machine_assets assets = {.bios = {{even, sizeof(even)}, {odd, sizeof(odd)}},
        .video = {video, 512u}};
    x86_nxvm_machine_construction construction = {0};
    lib_bool valid;

    lib_memory_set(video, 0xcc, sizeof(video));
    lib_memory_set(video, 0, assets.video.bytes);
    video[0] = 0x55u;
    video[1] = 0xaau;
    video[2] = 1u;
    if (mydeskpro386_machine_plan_create(&config, &assets, &construction) != LIB_STATUS_OK)
        return LIB_FALSE;
    const mydeskpro386_external_rom *rom = vm_test_profile_model40_rom(&construction);
    valid = rom != LIB_NULL && rom->video_byte_count == assets.video.bytes &&
        lib_memory_compare(rom->video_bytes, video, assets.video.bytes) == 0;
    /* The real candidate owns the maximum-size zeroed video buffer. Bytes
     * beyond the declared source must not be read/copied into its tail. */
    for (lib_size index = assets.video.bytes; valid && index < sizeof(video); ++index)
        valid = rom->video_bytes[index] == 0u;
    construction.profile.release(construction.profile.context);
    return valid;
}

static lib_bool observation_argument_contract(const x86_nxvm_machine *machine)
{
    x86_nxvm_machine_construction unrelated = {0};
    mydeskpro386_observation observation = { .fdc_terminal_valid = LIB_TRUE };

    return machine != LIB_NULL &&
        mydeskpro386_observe(&unrelated, &observation) == LIB_STATUS_UNSUPPORTED &&
        observation.fdc_terminal_valid &&
        mydeskpro386_observe(LIB_NULL, &observation) == LIB_STATUS_INVALID_ARGUMENT &&
        mydeskpro386_observe(vm_test_profile_construction(machine), LIB_NULL) ==
            LIB_STATUS_INVALID_ARGUMENT;
}

lib_i32 main(void)
{
    lib_u8 even[MYDESKPRO386_ROM_CHIP_BYTES] = {0};
    lib_u8 odd[MYDESKPRO386_ROM_CHIP_BYTES];
    lib_u8 video[MYDESKPRO386_VIDEO_ROM_BYTES] = {0};
    lib_u8 cmos_seed[X86_NXVM_MACHINE_CMOS_SEED_BYTES];
    x86_nxvm_machine_config config = {0};
    x86_nxvm_machine_assets assets = {0};
    x86_nxvm_machine *session = LIB_NULL;
    x86_nxvm_machine_reset_vector reset_vector = {0};
    x86_core_machine_run_result result = {0};
    x86_core_machine_time_observation time_observation = {0};
    lib_size memory_bytes = 0u;
    lib_size retained_memory_bytes;
    lib_u8 observed_memory = 0u;
    lib_u8 video_body_byte = 0u;
    const lib_u8 rejected_video_write = 0xa5u;
    lib_i32 failed = 1;

    lib_memory_set(odd, 1, sizeof(odd));
    video[0u] = 0x55u;
    video[1u] = 0xaau;
    video[2u] = 0x20u;
    video[MYDESKPRO386_VIDEO_ROM_ALIAS_SKIP_BYTES] = 0x5au;
    video[sizeof(video) - 1u] = 0x87u;
    config.bios_count = 2u;
    vm_model40_fixture_cmos_seed(cmos_seed);
    assets.bios[0u] = (x86_nxvm_machine_asset_bytes) { even, sizeof(even) };
    assets.bios[1u] = (x86_nxvm_machine_asset_bytes) { odd, sizeof(odd) };
    assets.video = (x86_nxvm_machine_asset_bytes) { video, sizeof(video) };
    assets.cmos_seed = (x86_nxvm_machine_asset_bytes) { cmos_seed, sizeof(cmos_seed) };
    if (vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        &config, &assets, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || !vm_test_profile_is_model40(vm_test_profile_construction(session))) goto done;
    if (!observation_argument_contract(session)) goto done;
    const test_board_plan_observation plan = test_board_capture_plan(session->x86_nxvm_board_plan);
    if (plan.memory_bytes != 2u * 1024u * 1024u ||
        plan.retirement_time_contract !=
            X86_CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC ||
        plan.l1_compatibility_policy !=
            X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS ||
        plan.cpu_profile != X86_CORE_CPU_PROFILE_80386 ||
        plan.fpu_profile != X86_FPU_PROFILE_NONE || !plan.cpu_80386_cr_mov_ignores_mod)
        goto done;
    const mydeskpro386_external_rom *rom =
        vm_test_profile_model40_rom(vm_test_profile_construction(session));
    if (rom == LIB_NULL || rom->even_bytes == LIB_NULL || rom->even_bytes[0] != 0u ||
        rom->odd_bytes == LIB_NULL || rom->odd_bytes[0] != 1u ||
        rom->video_bytes == LIB_NULL || rom->video_bytes[0u] != 0x55u) goto done;
    if (x86_core_machine_capture_time_observation(session->x86_core_machine,
            &time_observation) != LIB_STATUS_OK || !time_observation.pacing_time_available ||
        time_observation.pacing_ticks_per_second != 16000000u || time_observation.physical_time_available ||
        time_observation.physical_ticks_per_second != 0u ||
        vm_test_fdd_info(session->floppy[0u]).geometry.sectors_per_track != 15u ||
        x86_core_machine_memory_read(session->x86_core_machine,
            MYDESKPRO386_VIDEO_ROM_PHYSICAL_START, &observed_memory,
        sizeof(observed_memory)) != LIB_STATUS_OK || observed_memory != 0x55u) goto done;
    if (x86_core_machine_memory_read(session->x86_core_machine,
        MYDESKPRO386_VIDEO_ROM_PHYSICAL_START +
            MYDESKPRO386_VIDEO_ROM_ALIAS_SKIP_BYTES, &video_body_byte,
        sizeof(video_body_byte)) != LIB_STATUS_OK || x86_core_machine_memory_read(session->x86_core_machine,
        MYDESKPRO386_VIDEO_ROM_COMPATIBILITY_ALIAS_START +
            MYDESKPRO386_VIDEO_ROM_ALIAS_SKIP_BYTES, &observed_memory,
        sizeof(observed_memory)) != LIB_STATUS_OK || video_body_byte != 0x5au ||
        observed_memory != video_body_byte ||
        x86_core_machine_memory_write(session->x86_core_machine,
            MYDESKPRO386_VIDEO_ROM_COMPATIBILITY_ALIAS_START +
                MYDESKPRO386_VIDEO_ROM_ALIAS_SKIP_BYTES,
            &rejected_video_write, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(session->x86_core_machine,
            MYDESKPRO386_VIDEO_ROM_COMPATIBILITY_ALIAS_START +
                MYDESKPRO386_VIDEO_ROM_ALIAS_SKIP_BYTES,
            &observed_memory, 1u) != LIB_STATUS_OK || observed_memory != 0x5au ||
        x86_nxvm_machine_get_reset_vector(session, &reset_vector) != LIB_STATUS_OK ||
        reset_vector.cs != 0xf000u || reset_vector.ip != 0xfff0u) goto done;
    retained_memory_bytes = session->construction.core_config.memory_bytes;
    const x86_core_machine_clock_ratio refresh_clock =
        session->construction.core_config.clock_plan.pit;
    const lib_u64 first_pit_tick =
        ((lib_u64)refresh_clock.denominator + refresh_clock.numerator - 1u) /
            refresh_clock.numerator;
    if (x86_nxvm_machine_reconfigure_memory(session, 2u * 1024u * 1024u) !=
        LIB_STATUS_INVALID_STATE || x86_core_machine_get_memory_bytes(session->x86_core_machine,
        &memory_bytes) != LIB_STATUS_OK || memory_bytes != 2u * 1024u * 1024u ||
        session->construction.core_config.memory_bytes != retained_memory_bytes ||
        x86_core_machine_run(session->x86_core_machine,
        (x86_core_machine_run_budget) {1u, 0u}, &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        x86_core_machine_reset(session->x86_core_machine) != LIB_STATUS_OK ||
        !advance_to_first_pit_edge(session->x86_core_machine, first_pit_tick) ||
        !refresh_count_matches(session->x86_core_machine, 18u) ||
        x86_nxvm_machine_get_reset_vector(session, &reset_vector) != LIB_STATUS_OK ||
        reset_vector.cs != 0xf000u || reset_vector.ip != 0xfff0u) goto done;
    even[0u] = 2u;
    if (rom->even_bytes[0] != 0u || rom->odd_bytes[0] != 1u) goto done;
    x86_nxvm_machine_destroy(session);
    session = LIB_NULL;
    config.memory_bytes = 2u * 1024u * 1024u;
    if (vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        &config, &assets, &session) != LIB_STATUS_INVALID_ARGUMENT ||
        session != LIB_NULL) goto done;
    config.memory_bytes = 0u;
    failed = !short_video_copy_is_bounded();
done:
    x86_nxvm_machine_destroy(session);
    if (!failed) lib_c_printf("MODEL40-BYOB-MANIFEST:OK\nMODEL40-BYOB-VALIDATION:OK\nMODEL40-PUBLIC-COMPOSITION:OK\nMODEL40-BYOB-RESET-LIFECYCLE:OK\nMODEL40-IMMUTABLE-CONFIGURATION:OK\n");
    return failed;
}
