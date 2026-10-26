#include "lib/types/types_interface.h"
#include "lib/types/test.h"
#include "x86-nxvm/machine/preparation_interface.h"
#include "x86-nxvm/machine/pc_at_preparation_interface.h"
#include "x86-nxvm/board-base/pc_at_rom_interface.h"

static lib_status at_prepare_status;
static lib_u32 at_prepare_calls;
static lib_i32 accept_at_descriptor(const x86_nxvm_board_pc_at_descriptor *descriptor)
{ return descriptor != LIB_NULL; }
static lib_status prepare_at_candidate(const x86_nxvm_machine_config *config,
    x86_nxvm_board_pc_at_plan_snapshot *profile, x86_nxvm_machine_construction *construction)
{
    ++at_prepare_calls;
    lib_test_assert(config != LIB_NULL && profile != LIB_NULL && construction->profile.release != LIB_NULL);
    construction->floppy_slot_count = 1u;
    construction->floppy_kind = construction->media_kind = X86_NXVM_BOARD_FLOPPY_35_1440K;
    if (at_prepare_status != LIB_STATUS_OK) return at_prepare_status;
    const x86_nxvm_board_pc_at_descriptor descriptor = {
        .identity = "preparation-at", .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .ticks_per_instruction = 1u, .default_memory_bytes = (2u * 1024u * 1024u),
        .fdc_bounce_segment = 0x600u, .kbc_aux_absent = LIB_TRUE,
        .port_leaves = x86_nxvm_board_at_port_leaves, .port_leaf_count = X86_NXVM_BOARD_AT_PORT_LEAF_COUNT,
        .routes = x86_nxvm_board_at_routes_without_aux, .route_count = 4u,
        .firmware_services = x86_nxvm_board_pc_at_firmware_services, .firmware_service_count = 13u,
        .cmos = {.floppy_type = 0x40u}, .fdc_installed_mask = 1u, .fdc_double_sided_mask = 1u,
        .fdc_cylinder_count = {80u}, .fdc_ready_mask = 1u, .validate = accept_at_descriptor
    };
    lib_status status = x86_nxvm_board_pc_at_values_create(&descriptor,
        X86_CORE_CPU_PROFILE_DEFAULT, X86_FPU_PROFILE_NONE, &profile->values);
    if (status == LIB_STATUS_OK) status = x86_nxvm_board_pc_at_snapshot_copy(profile, &descriptor,
        descriptor.identity, 0x40u);
    if (status == LIB_STATUS_OK) {
        construction->core_config = profile->values.core.configuration;
        construction->topology = profile->topology;
    }
    return status;
}

static void pc_at_preparation_contract(void)
{
    static lib_u8 bios[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES];
    static lib_u8 low[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES / 2u];
    static lib_u8 high[X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES / 2u];
    const x86_nxvm_machine_config single = {.bios_count = 1u};
    x86_nxvm_machine_assets assets = {.bios = {{bios, sizeof(bios)}}};
    x86_nxvm_machine_construction candidate = {0};
    for (lib_size i = 0u; i < sizeof(bios); ++i) bios[i] = (lib_u8)i;
    lib_test_assert(x86_nxvm_machine_pc_at_construction_create(&single, &assets, LIB_NULL, &candidate) == LIB_STATUS_INVALID_ARGUMENT);
    at_prepare_status = LIB_STATUS_UNSUPPORTED;
    lib_test_assert(x86_nxvm_machine_pc_at_construction_create(&single, &assets, prepare_at_candidate, &candidate) == LIB_STATUS_UNSUPPORTED &&
        candidate.profile.context == LIB_NULL);
    at_prepare_status = LIB_STATUS_OK;
    assets.bios[0].bytes = sizeof(bios) - 1u;
    lib_test_assert(x86_nxvm_machine_pc_at_construction_create(&single, &assets, prepare_at_candidate, &candidate) == LIB_STATUS_INVALID_ARGUMENT &&
        candidate.profile.context == LIB_NULL);
    assets.bios[0].bytes = sizeof(bios);
    lib_test_assert(x86_nxvm_machine_pc_at_construction_create(&single, &assets, prepare_at_candidate, &candidate) == LIB_STATUS_OK);
    const x86_nxvm_board_external_pc_at_rom_context *rom = candidate.firmware_context;
    lib_test_assert(rom != LIB_NULL && rom->image != bios && lib_memory_compare(rom->image, bios, sizeof(bios)) == 0);
    bios[17] ^= 0xffu;
    lib_test_assert(rom->image[17] == 17u && candidate.firmware_provider == x86_nxvm_board_external_pc_at_rom_provider());
    candidate.profile.release(candidate.profile.context);
    for (lib_size i = 0u; i < sizeof(low); ++i) { low[i] = 0x12u; high[i] = 0x34u; }
    const x86_nxvm_machine_config split = {.bios_count = 2u};
    assets = (x86_nxvm_machine_assets){.bios = {{low, sizeof(low)}, {high, sizeof(high)}}};
    lib_test_assert(x86_nxvm_machine_pc_at_construction_create(&split, &assets, prepare_at_candidate, &candidate) == LIB_STATUS_OK);
    rom = candidate.firmware_context;
    for (lib_size i = 0u; i < X86_NXVM_BOARD_EXTERNAL_PC_AT_ROM_BYTES; ++i)
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
    static lib_u8 font[X86_NXVM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES];
    lib_u8 seed[X86_NXVM_MACHINE_CMOS_SEED_BYTES];
    lib_u8 extended_seed[X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
    const x86_nxvm_machine_config config = {0};
    const x86_nxvm_machine_assets absent = {0};
    x86_nxvm_machine_assets assets = {0};
    x86_nxvm_machine_construction candidate = {0}, published;
    lib_size releases = 0u;

    for (lib_size index = 0u; index < sizeof(seed); ++index) seed[index] = (lib_u8)index;
    for (lib_size index = 0u; index < sizeof(extended_seed); ++index)
        extended_seed[index] = (lib_u8)(index ^ 0xa5u);
    for (lib_size index = 0u; index < sizeof(font); ++index)
        font[index] = (lib_u8)(index ^ (index >> 8u));
    candidate.profile = (x86_nxvm_machine_profile_binding){.context = &releases, .release = release_candidate};
    lib_test_assert(x86_nxvm_machine_construction_begin(&config, &absent, &published) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_construction_finish(&candidate, &config, &absent,
        LIB_STATUS_OK, &published) == LIB_STATUS_OK && releases == 0u);
    lib_test_assert(!published.cmos_seed_present && !published.text_glyphs.present);
    published.profile.release(published.profile.context);
    lib_test_assert(releases == 1u);

    assets.cmos_seed = (x86_nxvm_machine_asset_bytes){seed, sizeof(seed)};
    assets.font = (x86_nxvm_machine_asset_bytes){font, sizeof(font)};
    lib_test_assert(x86_nxvm_machine_construction_begin(&config, &assets, &published) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_construction_finish(&candidate, &config, &assets,
        LIB_STATUS_OK, &published) == LIB_STATUS_OK && releases == 1u);
    lib_test_assert(published.cmos_seed_present && published.text_glyphs.present);
    lib_test_assert(published.cmos_seed_bytes == sizeof(seed));
    lib_test_assert(lib_memory_compare(published.cmos_seed, seed, sizeof(seed)) == 0);
    for (lib_size character = 0u; character < X86_VIDEO_TEXT_GLYPH_COUNT; ++character) {
        lib_test_assert(lib_memory_compare(&published.text_glyphs.bytes[character * 16u],
            &font[character * 8u], 8u) == 0);
        lib_test_assert(lib_memory_compare(&published.text_glyphs.bytes[character * 16u + 8u],
            &font[X86_NXVM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES + character * 8u], 8u) == 0);
    }
    published.profile.release(published.profile.context);
    lib_test_assert(releases == 2u);

    assets.cmos_seed = (x86_nxvm_machine_asset_bytes){extended_seed, sizeof(extended_seed)};
    lib_test_assert(x86_nxvm_machine_construction_begin(&config, &assets, &published) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_construction_finish(&candidate, &config, &assets,
        LIB_STATUS_OK, &published) == LIB_STATUS_OK && releases == 2u);
    lib_test_assert(published.cmos_seed_present &&
        published.cmos_seed_bytes == sizeof(extended_seed));
    lib_test_assert(lib_memory_compare(published.cmos_seed, extended_seed,
        sizeof(extended_seed)) == 0);
    published.profile.release(published.profile.context);
    lib_test_assert(releases == 3u);
}

static void failures(void)
{
    lib_u8 byte = 0u;
    const struct {
        x86_nxvm_machine_asset_bytes seed;
        x86_nxvm_machine_asset_bytes font;
        lib_status preparation;
        lib_bool second_floppy;
    } cases[] = {
        {{LIB_NULL, 1u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{&byte, 0u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{&byte, X86_NXVM_MACHINE_CMOS_SEED_BYTES - 1u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{&byte, X86_NXVM_MACHINE_CMOS_SEED_BYTES + 1u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{&byte, X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES - 1u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{&byte, X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES + 1u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{LIB_NULL, 0u}, {LIB_NULL, 1u}, LIB_STATUS_OK, LIB_FALSE},
        {{LIB_NULL, 0u}, {&byte, 0u}, LIB_STATUS_OK, LIB_FALSE},
        {{LIB_NULL, 0u}, {&byte, X86_NXVM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES - 1u}, LIB_STATUS_OK, LIB_FALSE},
        {{LIB_NULL, 0u}, {&byte, X86_NXVM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES + 1u}, LIB_STATUS_OK, LIB_FALSE},
        {{LIB_NULL, 0u}, {LIB_NULL, 0u}, LIB_STATUS_UNSUPPORTED, LIB_FALSE},
        {{LIB_NULL, 0u}, {LIB_NULL, 0u}, LIB_STATUS_OK, LIB_TRUE}
    };
    x86_nxvm_machine_config config = {0};
    x86_nxvm_machine_assets assets = {0};
    x86_nxvm_machine_construction published = {0};
    lib_size releases = 0u;

    for (lib_size index = 0u; index < (sizeof(cases) / sizeof(cases[0u])); ++index) {
        x86_nxvm_machine_construction candidate = {.floppy_slot_count = 1u,
            .profile = {.context = &releases, .release = release_candidate}};
        config.floppy_image[1u] = cases[index].second_floppy ? "second" : LIB_NULL;
        assets.cmos_seed = cases[index].seed;
        assets.font = cases[index].font;
        published = candidate;
        lib_test_assert(x86_nxvm_machine_construction_finish(&candidate, &config, &assets,
            cases[index].preparation, &published) ==
            (cases[index].preparation == LIB_STATUS_OK ? LIB_STATUS_INVALID_ARGUMENT : cases[index].preparation));
        lib_test_assert(releases == index + 1u && published.profile.context == LIB_NULL &&
            published.profile.release == LIB_NULL && !published.cmos_seed_present && !published.text_glyphs.present);
    }
    assets = (x86_nxvm_machine_assets){0};
    config = (x86_nxvm_machine_config){.floppy_image = {LIB_NULL, "second"}};
    x86_nxvm_machine_construction candidate = {.floppy_slot_count = 2u,
        .profile = {.context = &releases, .release = release_candidate}};
    lib_test_assert(x86_nxvm_machine_construction_finish(&candidate, &config, &assets,
        LIB_STATUS_OK, &published) == LIB_STATUS_OK && releases == (sizeof(cases) / sizeof(cases[0u])));
    published.profile.release(published.profile.context);
    published = candidate;
    lib_test_assert(x86_nxvm_machine_construction_begin(LIB_NULL, &assets, &published) ==
        LIB_STATUS_INVALID_ARGUMENT && published.profile.context == LIB_NULL);
    published = candidate;
    lib_test_assert(x86_nxvm_machine_construction_begin(&config, LIB_NULL, &published) ==
        LIB_STATUS_INVALID_ARGUMENT && published.profile.context == LIB_NULL);
    config.fixed_disk_image[1u] = "second";
    lib_test_assert(x86_nxvm_machine_construction_begin(&config, &assets, &published) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_construction_begin(&config, &assets, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    published = candidate;
    lib_test_assert(x86_nxvm_machine_construction_finish(LIB_NULL, &config, &assets,
        LIB_STATUS_OK, &published) == LIB_STATUS_INVALID_ARGUMENT && published.profile.context == LIB_NULL);
    const lib_size prior = releases;
    lib_test_assert(x86_nxvm_machine_construction_finish(&candidate, LIB_NULL, &assets,
        LIB_STATUS_OK, &published) == LIB_STATUS_INVALID_ARGUMENT && releases == prior + 1u);
    lib_test_assert(x86_nxvm_machine_construction_finish(&candidate, &config, LIB_NULL,
        LIB_STATUS_OK, &published) == LIB_STATUS_INVALID_ARGUMENT && releases == prior + 2u);
    lib_test_assert(x86_nxvm_machine_construction_finish(&candidate, &config, &assets,
        LIB_STATUS_OK, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT && releases == prior + 3u);
}

static void floppy_policy(void)
{
    const x86_nxvm_board_floppy_kind kinds[] = {X86_NXVM_BOARD_FLOPPY_35_1440K,
        X86_NXVM_BOARD_FLOPPY_525_1200K, X86_NXVM_BOARD_FLOPPY_525_360K, X86_NXVM_BOARD_FLOPPY_35_720K};
    const x86_nxvm_machine_floppy_format formats[] = {X86_NXVM_MACHINE_FLOPPY_FORMAT_1440K,
        X86_NXVM_MACHINE_FLOPPY_FORMAT_1200K, X86_NXVM_MACHINE_FLOPPY_FORMAT_360K, X86_NXVM_MACHINE_FLOPPY_FORMAT_720K};
    const lib_u32 masks[] = {15u, (1u << X86_NXVM_BOARD_FLOPPY_525_1200K) |
        (1u << X86_NXVM_BOARD_FLOPPY_525_360K), 1u << X86_NXVM_BOARD_FLOPPY_525_360K};
    const x86_nxvm_board_floppy_kind defaults[] = {X86_NXVM_BOARD_FLOPPY_35_1440K,
        X86_NXVM_BOARD_FLOPPY_525_1200K, X86_NXVM_BOARD_FLOPPY_525_360K};

    for (lib_size policy = 0u; policy < (sizeof(masks) / sizeof(masks[0u])); ++policy) {
        x86_nxvm_board_floppy_kind selected;
        lib_test_assert(x86_nxvm_machine_floppy_select(LIB_NULL, defaults[policy], masks[policy],
            &selected) == LIB_STATUS_OK && selected == defaults[policy]);
        for (lib_size index = 0u; index < (sizeof(kinds) / sizeof(kinds[0u])); ++index) {
            const x86_nxvm_machine_config config = {.floppy_format = formats[index]};
            const lib_bool eligible = (masks[policy] & (1u << kinds[index])) != 0u;
            selected = (x86_nxvm_board_floppy_kind)99;
            lib_test_assert(x86_nxvm_machine_floppy_select(&config, defaults[policy], masks[policy], &selected) ==
                (eligible ? LIB_STATUS_OK : LIB_STATUS_INVALID_ARGUMENT));
            lib_test_assert(selected == (eligible ? kinds[index] : (x86_nxvm_board_floppy_kind)99));
        }
    }
    const x86_nxvm_machine_config invalid = {.floppy_format = (x86_nxvm_machine_floppy_format)99};
    x86_nxvm_board_floppy_kind selected;
    lib_test_assert(x86_nxvm_machine_floppy_select(&invalid, kinds[0], 15u, &selected) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_nxvm_machine_floppy_select(LIB_NULL, (x86_nxvm_board_floppy_kind)99, 15u, &selected) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_nxvm_machine_floppy_select(LIB_NULL, kinds[0], 0u, &selected) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_nxvm_machine_floppy_select(LIB_NULL, kinds[0], 15u, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
}

lib_i32 main(void)
{
    lib_u8 source[2u] = {1u, 2u}, copy[2u] = {0};

    lib_test_assert(x86_nxvm_machine_asset_copy(copy, sizeof(copy), (x86_nxvm_machine_asset_bytes){source, sizeof(source)}) == LIB_STATUS_OK);
    lib_test_assert(lib_memory_compare(copy, source, sizeof(copy)) == 0);
    lib_test_assert(x86_nxvm_machine_asset_copy(copy, sizeof(copy), (x86_nxvm_machine_asset_bytes){source, 1u}) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_nxvm_machine_asset_copy(LIB_NULL, sizeof(copy), (x86_nxvm_machine_asset_bytes){source, sizeof(source)}) == LIB_STATUS_INVALID_ARGUMENT);
    finishing();
    failures();
    floppy_policy();
    pc_at_preparation_contract();
    return 0;
}
