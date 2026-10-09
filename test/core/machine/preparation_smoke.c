#include "lib/types/types_interface.h"
#include "lib/types/test.h"
#include "core/machine/preparation_interface.h"
#include "core/machine/pc_at_preparation_interface.h"
#include "core/board-base/pc_at_rom_interface.h"

static lib_status at_prepare_status;
static lib_u32 at_prepare_calls;
static lib_i32 accept_at_descriptor(const vm_profile_default_pc_at_descriptor *descriptor)
{ return descriptor != LIB_NULL; }
static lib_status prepare_at_candidate(const vm_machine_config *config,
    vm_profile_default_pc_at_plan_snapshot *profile, vm_machine_construction *construction)
{
    ++at_prepare_calls;
    lib_test_assert(config != LIB_NULL && profile != LIB_NULL && construction->profile.release != LIB_NULL);
    construction->floppy_slot_count = 1u;
    construction->floppy_kind = construction->media_kind = VM_PROFILE_FLOPPY_35_1440K;
    if (at_prepare_status != LIB_STATUS_OK) return at_prepare_status;
    const vm_profile_default_pc_at_descriptor descriptor = {
        .identity = "preparation-at", .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .ticks_per_instruction = 1u, .default_memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .fdc_bounce_segment = 0x600u, .kbc_aux_absent = LIB_TRUE,
        .port_leaves = vm_at_port_leaves, .port_leaf_count = VM_AT_PORT_LEAF_COUNT,
        .routes = vm_at_routes_without_aux, .route_count = 4u,
        .firmware_services = vm_pc_at_firmware_services, .firmware_service_count = 13u,
        .cmos = {.floppy_type = 0x40u}, .fdc_installed_mask = 1u, .fdc_double_sided_mask = 1u,
        .fdc_cylinder_count = {80u}, .fdc_ready_mask = 1u, .validate = accept_at_descriptor
    };
    lib_status status = vm_profile_default_pc_at_values_create(&descriptor,
        CORE_MACHINE_CPU_PROFILE_DEFAULT, X86_FPU_PROFILE_NONE, &profile->values);
    if (status == LIB_STATUS_OK) status = vm_profile_default_pc_at_snapshot_copy(profile, &descriptor,
        descriptor.identity, 0x40u);
    if (status == LIB_STATUS_OK) {
        construction->core_config = profile->values.core.configuration;
        construction->topology = profile->topology;
    }
    return status;
}

static void pc_at_preparation_contract(void)
{
    static lib_u8 bios[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES];
    static lib_u8 low[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES / 2u];
    static lib_u8 high[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES / 2u];
    const vm_machine_config single = {.bios_count = 1u};
    vm_machine_assets assets = {.bios = {{bios, sizeof(bios)}}};
    vm_machine_construction candidate = {0};
    for (lib_size i = 0u; i < sizeof(bios); ++i) bios[i] = (lib_u8)i;
    lib_test_assert(vm_pc_at_construction_create(&single, &assets, LIB_NULL, &candidate) == LIB_STATUS_INVALID_ARGUMENT);
    at_prepare_status = LIB_STATUS_UNSUPPORTED;
    lib_test_assert(vm_pc_at_construction_create(&single, &assets, prepare_at_candidate, &candidate) == LIB_STATUS_UNSUPPORTED &&
        candidate.profile.context == LIB_NULL);
    at_prepare_status = LIB_STATUS_OK;
    assets.bios[0].bytes = sizeof(bios) - 1u;
    lib_test_assert(vm_pc_at_construction_create(&single, &assets, prepare_at_candidate, &candidate) == LIB_STATUS_INVALID_ARGUMENT &&
        candidate.profile.context == LIB_NULL);
    assets.bios[0].bytes = sizeof(bios);
    lib_test_assert(vm_pc_at_construction_create(&single, &assets, prepare_at_candidate, &candidate) == LIB_STATUS_OK);
    const vm_profile_external_pc_at_rom_context *rom = candidate.firmware_context;
    lib_test_assert(rom != LIB_NULL && rom->image != bios && lib_memory_compare(rom->image, bios, sizeof(bios)) == 0);
    bios[17] ^= 0xffu;
    lib_test_assert(rom->image[17] == 17u && candidate.firmware_provider == vm_profile_external_pc_at_rom_provider());
    candidate.profile.release(candidate.profile.context);
    for (lib_size i = 0u; i < sizeof(low); ++i) { low[i] = 0x12u; high[i] = 0x34u; }
    const vm_machine_config split = {.bios_count = 2u};
    assets = (vm_machine_assets){.bios = {{low, sizeof(low)}, {high, sizeof(high)}}};
    lib_test_assert(vm_pc_at_construction_create(&split, &assets, prepare_at_candidate, &candidate) == LIB_STATUS_OK);
    rom = candidate.firmware_context;
    for (lib_size i = 0u; i < VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES; ++i)
        lib_test_assert(rom->image[i] == (i % 2u == 0u ? 0x12u : 0x34u));
    candidate.profile.release(candidate.profile.context);
    lib_test_assert(at_prepare_calls == 4u);
}

static void release_candidate(void *context)
{
    lib_size *releases = context;

    ++*releases;
}

static void finishing(void)
{
    static lib_u8 font[VM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES];
    lib_u8 seed[VM_MACHINE_CMOS_SEED_BYTES];
    const vm_machine_config config = {0};
    const vm_machine_assets absent = {0};
    vm_machine_assets assets = {0};
    vm_machine_construction candidate = {0}, published;
    lib_size releases = 0u;

    for (lib_size index = 0u; index < sizeof(seed); ++index) seed[index] = (lib_u8)index;
    for (lib_size index = 0u; index < sizeof(font); ++index)
        font[index] = (lib_u8)(index ^ (index >> 8u));
    candidate.profile = (vm_machine_profile_binding){.context = &releases, .release = release_candidate};
    lib_test_assert(vm_machine_construction_begin(&config, &absent, &published) == LIB_STATUS_OK);
    lib_test_assert(vm_machine_construction_finish(&candidate, &config, &absent,
        LIB_STATUS_OK, &published) == LIB_STATUS_OK && releases == 0u);
    lib_test_assert(!published.cmos_seed_present && !published.text_glyphs.present);
    published.profile.release(published.profile.context);
    lib_test_assert(releases == 1u);

    assets.cmos_seed = (vm_machine_asset_bytes){seed, sizeof(seed)};
    assets.font = (vm_machine_asset_bytes){font, sizeof(font)};
    lib_test_assert(vm_machine_construction_begin(&config, &assets, &published) == LIB_STATUS_OK);
    lib_test_assert(vm_machine_construction_finish(&candidate, &config, &assets,
        LIB_STATUS_OK, &published) == LIB_STATUS_OK && releases == 1u);
    lib_test_assert(published.cmos_seed_present && published.text_glyphs.present);
    lib_test_assert(lib_memory_compare(published.cmos_seed, seed, sizeof(seed)) == 0);
    for (lib_size character = 0u; character < X86_VIDEO_TEXT_GLYPH_COUNT; ++character) {
        lib_test_assert(lib_memory_compare(&published.text_glyphs.bytes[character * 16u],
            &font[character * 8u], 8u) == 0);
        lib_test_assert(lib_memory_compare(&published.text_glyphs.bytes[character * 16u + 8u],
            &font[VM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES + character * 8u], 8u) == 0);
    }
    published.profile.release(published.profile.context);
    lib_test_assert(releases == 2u);
}

static void failures(void)
{
    lib_u8 byte = 0u;
    const struct {
        vm_machine_asset_bytes seed;
        vm_machine_asset_bytes font;
        lib_status preparation;
        lib_bool second_floppy;
    } cases[] = {
        {{LIB_NULL, 1u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{&byte, 0u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{&byte, VM_MACHINE_CMOS_SEED_BYTES - 1u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{&byte, VM_MACHINE_CMOS_SEED_BYTES + 1u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{LIB_NULL, 0u}, {LIB_NULL, 1u}, LIB_STATUS_OK, LIB_FALSE},
        {{LIB_NULL, 0u}, {&byte, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{LIB_NULL, 0u}, {&byte, VM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES - 1u}, LIB_STATUS_OK, LIB_FALSE},
        {{LIB_NULL, 0u}, {&byte, VM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES + 1u}, LIB_STATUS_OK, LIB_FALSE},
        {{LIB_NULL, 0u}, {LIB_NULL, 0u}, LIB_STATUS_UNSUPPORTED, LIB_FALSE},
        {{LIB_NULL, 0u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_TRUE}
    };
    vm_machine_config config = {0};
    vm_machine_assets assets = {0};
    vm_machine_construction published = {0};
    lib_size releases = 0u;

    for (lib_size index = 0u; index < (sizeof(cases) / sizeof(cases[0u])); ++index) {
        vm_machine_construction candidate = {.floppy_slot_count = 1u,
            .profile = {.context = &releases, .release = release_candidate}};
        config.floppy_image[1u] = cases[index].second_floppy ? "second" : LIB_NULL;
        assets.cmos_seed = cases[index].seed;
        assets.font = cases[index].font;
        published = candidate;
        lib_test_assert(vm_machine_construction_finish(&candidate, &config, &assets,
            cases[index].preparation, &published) ==
            (cases[index].preparation == LIB_STATUS_OK ? LIB_STATUS_INVALID_ARGUMENT : cases[index].preparation));
        lib_test_assert(releases == index + 1u && published.profile.context == LIB_NULL &&
            published.profile.release == LIB_NULL && !published.cmos_seed_present && !published.text_glyphs.present);
    }
    assets = (vm_machine_assets){0};
    config = (vm_machine_config){.floppy_image = {LIB_NULL, "second"}};
    vm_machine_construction candidate = {.floppy_slot_count = 2u,
        .profile = {.context = &releases, .release = release_candidate}};
    lib_test_assert(vm_machine_construction_finish(&candidate, &config, &assets,
        LIB_STATUS_OK, &published) == LIB_STATUS_OK && releases == (sizeof(cases) / sizeof(cases[0u])));
    published.profile.release(published.profile.context);
    published = candidate;
    lib_test_assert(vm_machine_construction_begin(LIB_NULL, &assets, &published) ==
        LIB_STATUS_INVALID_ARGUMENT && published.profile.context == LIB_NULL);
    published = candidate;
    lib_test_assert(vm_machine_construction_begin(&config, LIB_NULL, &published) ==
        LIB_STATUS_INVALID_ARGUMENT && published.profile.context == LIB_NULL);
    config.fixed_disk_image[1u] = "second";
    lib_test_assert(vm_machine_construction_begin(&config, &assets, &published) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(vm_machine_construction_begin(&config, &assets, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    published = candidate;
    lib_test_assert(vm_machine_construction_finish(LIB_NULL, &config, &assets,
        LIB_STATUS_OK, &published) == LIB_STATUS_INVALID_ARGUMENT && published.profile.context == LIB_NULL);
    const lib_size prior = releases;
    lib_test_assert(vm_machine_construction_finish(&candidate, LIB_NULL, &assets,
        LIB_STATUS_OK, &published) == LIB_STATUS_INVALID_ARGUMENT && releases == prior + 1u);
    lib_test_assert(vm_machine_construction_finish(&candidate, &config, LIB_NULL,
        LIB_STATUS_OK, &published) == LIB_STATUS_INVALID_ARGUMENT && releases == prior + 2u);
    lib_test_assert(vm_machine_construction_finish(&candidate, &config, &assets,
        LIB_STATUS_OK, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT && releases == prior + 3u);
}

static void floppy_policy(void)
{
    const vm_profile_floppy_kind kinds[] = {VM_PROFILE_FLOPPY_35_1440K,
        VM_PROFILE_FLOPPY_525_1200K, VM_PROFILE_FLOPPY_525_360K, VM_PROFILE_FLOPPY_35_720K};
    const vm_machine_floppy_format formats[] = {VM_MACHINE_FLOPPY_FORMAT_1440K,
        VM_MACHINE_FLOPPY_FORMAT_1200K, VM_MACHINE_FLOPPY_FORMAT_360K, VM_MACHINE_FLOPPY_FORMAT_720K};
    const lib_u32 masks[] = {15u, (1u << VM_PROFILE_FLOPPY_525_1200K) |
        (1u << VM_PROFILE_FLOPPY_525_360K), 1u << VM_PROFILE_FLOPPY_525_360K};
    const vm_profile_floppy_kind defaults[] = {VM_PROFILE_FLOPPY_35_1440K,
        VM_PROFILE_FLOPPY_525_1200K, VM_PROFILE_FLOPPY_525_360K};

    for (lib_size policy = 0u; policy < (sizeof(masks) / sizeof(masks[0u])); ++policy) {
        vm_profile_floppy_kind selected;
        lib_test_assert(vm_machine_floppy_select(LIB_NULL, defaults[policy], masks[policy],
            &selected) == LIB_STATUS_OK && selected == defaults[policy]);
        for (lib_size index = 0u; index < (sizeof(kinds) / sizeof(kinds[0u])); ++index) {
            const vm_machine_config config = {.floppy_format = formats[index]};
            const lib_bool eligible = (masks[policy] & (1u << kinds[index])) != 0u;
            selected = (vm_profile_floppy_kind)99;
            lib_test_assert(vm_machine_floppy_select(&config, defaults[policy], masks[policy], &selected) ==
                (eligible ? LIB_STATUS_OK : LIB_STATUS_INVALID_ARGUMENT));
            lib_test_assert(selected == (eligible ? kinds[index] : (vm_profile_floppy_kind)99));
        }
    }
    const vm_machine_config invalid = {.floppy_format = (vm_machine_floppy_format)99};
    vm_profile_floppy_kind selected;
    lib_test_assert(vm_machine_floppy_select(&invalid, kinds[0], 15u, &selected) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(vm_machine_floppy_select(LIB_NULL, (vm_profile_floppy_kind)99, 15u, &selected) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(vm_machine_floppy_select(LIB_NULL, kinds[0], 0u, &selected) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(vm_machine_floppy_select(LIB_NULL, kinds[0], 15u, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
}

lib_i32 main(void)
{
    lib_u8 source[2u] = {1u, 2u}, copy[2u] = {0};

    lib_test_assert(vm_machine_asset_copy(copy, sizeof(copy), (vm_machine_asset_bytes){source, sizeof(source)}) == LIB_STATUS_OK);
    lib_test_assert(lib_memory_compare(copy, source, sizeof(copy)) == 0);
    lib_test_assert(vm_machine_asset_copy(copy, sizeof(copy), (vm_machine_asset_bytes){source, 1u}) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(vm_machine_asset_copy(LIB_NULL, sizeof(copy), (vm_machine_asset_bytes){source, sizeof(source)}) == LIB_STATUS_INVALID_ARGUMENT);
    finishing();
    failures();
    floppy_policy();
    pc_at_preparation_contract();
    return 0;
}
