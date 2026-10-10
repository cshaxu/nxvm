#include "../../support/profile.h"
/* Repository guest firmware and synthetic media only; no external asset files. */
#include "core/machine/input_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "core/board-base/media_interface.h"
#include "lib/types/file.h"

extern const vm_machine_assets vm_app_firmware;

typedef struct disk {
    lib_u8 bytes[80u * 2u * 18u * 512u];
    lib_bool present;
    lib_bool readonly;
} disk;

static core_machine_media_result query(void *context, core_machine_media_info *info)
{
    disk *media = context;
    *info = (core_machine_media_info) {
        .present = media->present, .generation = 1u,
        .capabilities = CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
            CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS |
            (media->readonly ? CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY : 0u),
        .geometry = {2880u, 512u, 80u, 2u, 18u}
    };
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result read_bytes(void *context, lib_u64 offset,
    void *buffer, lib_u32 count)
{
    disk *media = context;
    if (!media->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (offset > sizeof(media->bytes) || count > sizeof(media->bytes) - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    lib_memory_copy(buffer, media->bytes + (lib_size)offset, count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result write_bytes(void *context, lib_u64 offset,
    const void *buffer, lib_u32 count)
{
    disk *media = context;
    if (media->readonly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (!media->present) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (offset > sizeof(media->bytes) || count > sizeof(media->bytes) - offset)
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    lib_memory_copy(media->bytes + (lib_size)offset, buffer, count);
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static void word(lib_u8 *bytes, lib_size *offset, lib_u16 value)
{
    bytes[(*offset)++] = (lib_u8)value;
    bytes[(*offset)++] = (lib_u8)(value >> 8u);
}

static core_machine_media_result set_mark(void *context, lib_u64 sector,
    core_machine_media_address_mark mark)
{
    disk *media = context;
    if (media->readonly) return CORE_MACHINE_MEDIA_RESULT_READ_ONLY;
    if (sector >= 2880u) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    return mark == CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA ?
        CORE_MACHINE_MEDIA_RESULT_OK : CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED;
}

static core_machine_media_result get_mark(void *context, lib_u64 sector,
    core_machine_media_address_mark *mark)
{
    (void)context;
    if (sector >= 2880u) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    *mark = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static void instruction(lib_u8 *bytes, lib_size *offset, lib_u8 opcode, lib_u16 value)
{
    bytes[(*offset)++] = opcode;
    word(bytes, offset, value);
}

static void request(lib_u8 *bytes, lib_size *offset, lib_u16 ax, lib_u16 cx,
    lib_u16 dx, lib_u16 output)
{
    static const lib_u8 registers[] = {0u, 3u, 1u, 2u, 6u, 7u, 5u};
    instruction(bytes, offset, 0xb8u, 0x1234u); /* DS remains caller-owned. */
    bytes[(*offset)++] = 0x8eu; bytes[(*offset)++] = 0xd8u;
    instruction(bytes, offset, 0xb8u, 0x1000u);
    bytes[(*offset)++] = 0x8eu; bytes[(*offset)++] = 0xc0u;
    instruction(bytes, offset, 0xb8u, ax);
    instruction(bytes, offset, 0xbbu, 0xfff0u);
    instruction(bytes, offset, 0xb9u, cx);
    instruction(bytes, offset, 0xbau, dx);
    instruction(bytes, offset, 0xbeu, 0x1357u);
    instruction(bytes, offset, 0xbfu, 0x2468u);
    instruction(bytes, offset, 0xbdu, 0x369cu);
    bytes[(*offset)++] = 0xcdu; bytes[(*offset)++] = 0x13u;
    /* CS is zero at the boot entry: copied results do not depend on DS/ES. */
    for (lib_size index = 0u; index < sizeof(registers); ++index) {
        bytes[(*offset)++] = 0x2eu; bytes[(*offset)++] = 0x89u;
        bytes[(*offset)++] = (lib_u8)(0x06u | (registers[index] << 3u));
        word(bytes, offset, (lib_u16)(output + index * 2u));
    }
    bytes[(*offset)++] = 0x9cu; bytes[(*offset)++] = 0x58u;
    bytes[(*offset)++] = 0x2eu;
    instruction(bytes, offset, 0xa3u, output + 14u);
    bytes[(*offset)++] = 0x2eu; bytes[(*offset)++] = 0x8cu;
    bytes[(*offset)++] = 0x1eu; word(bytes, offset, output + 16u);
    bytes[(*offset)++] = 0x2eu; bytes[(*offset)++] = 0x8cu;
    bytes[(*offset)++] = 0x06u; word(bytes, offset, output + 18u);
}

static lib_bool run_to(core_machine *machine, lib_u32 pc, lib_u32 limit)
{
    for (lib_u32 used = 0u; used < limit; ++used) {
        core_machine_run_result result = {0};
        if (core_machine_run(machine, (core_machine_run_budget){1u, 0u}, &result) !=
                LIB_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) {
            lib_c_fprintf(lib_c_stderr, "Guest run failed: reason=%u PC=%x\n",
                result.reason, result.linear_pc);
            return LIB_FALSE;
        }
        if (result.linear_pc == pc) return LIB_TRUE;
    }
    core_machine_cpu_state state;
    if (core_machine_get_cpu_state(machine, &state) == LIB_STATUS_OK)
        lib_c_fprintf(lib_c_stderr, "Guest budget: %04x:%x, wanted=%x\n", state.cs, state.eip, pc);
    return LIB_FALSE;
}

static lib_bool check(lib_u16 ax, lib_u16 cx, lib_u16 dx, lib_u8 expected,
    lib_bool readonly, lib_bool absent, lib_bool recalibrate)
{
    const core_machine_media_provider provider = {
        .query = query, .read_bytes = read_bytes, .write_bytes = write_bytes,
        .set_address_mark = set_mark, .get_address_mark = get_mark
    };
    vm_machine_config config = {.bios_count = 1u};
    vm_machine_construction profile = {0};
    core_machine_plan *plan = LIB_NULL;
    core_machine_media_registry *registry = LIB_NULL;
    core_machine_display_provider_slot *display = LIB_NULL;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    disk *media = lib_allocate_zero(1u, sizeof(*media));
    disk *empty = lib_allocate_zero(1u, sizeof(*empty));
    lib_u8 payload[1024], actual[1024];
    lib_u16 result[10] = {0};
    lib_size offset = 0u;
    lib_bool ok = LIB_FALSE;

    if (media == LIB_NULL || empty == LIB_NULL) goto done;
    media->present = LIB_TRUE;
    for (lib_size index = 512u; index < sizeof(media->bytes); ++index)
        media->bytes[index] = (lib_u8)(index ^ (index >> 9u));
    for (lib_size index = 0u; index < sizeof(payload); ++index)
        payload[index] = (lib_u8)(index ^ 0xa5u);
    if (recalibrate) {
        request(media->bytes, &offset, 0x0201u, 0x4f01u, 0u, 0x620u);
        request(media->bytes, &offset, 0u, 0u, 0u, 0x640u);
    }
    if (expected == 0x80u) {
        /* Guest masks IRQ6: BIOS reset must time out, not wait forever. */
        static const lib_u8 mask[] = {0xe4u, 0x21u, 0x0cu, 0x40u, 0xe6u, 0x21u};
        lib_memory_copy(media->bytes + offset, mask, sizeof(mask));
        offset += sizeof(mask);
    }
    request(media->bytes, &offset, ax, cx, dx, 0x600u);
    if (expected != 0u) request(media->bytes, &offset, 0x0100u, cx, dx, 0x660u);
    if (expected == 0x80u) {
        static const lib_u8 unmask[] = {0xe4u, 0x21u, 0x24u, 0xbfu, 0xe6u, 0x21u};
        lib_memory_copy(media->bytes + offset, unmask, sizeof(unmask));
        offset += sizeof(unmask);
        request(media->bytes, &offset, 0x0201u, 0x0101u, 0u, 0x680u);
    }
    media->bytes[offset++] = 0xfau; media->bytes[offset++] = 0xf4u;
    if (offset > 510u) goto done;
    media->bytes[510u] = 0x55u; media->bytes[511u] = 0xaau;
    if (vm_test_profile_construction_create(VM_MACHINE_PROFILE_DEFAULT_PC_AT,
        &config, &vm_app_firmware, &profile) != LIB_STATUS_OK ||
        core_machine_plan_create(&profile.core_config, &plan) != LIB_STATUS_OK ||
        plan == LIB_NULL ||
        core_machine_plan_set_controller_timing_rules(plan,
            &profile.timing_rules) != LIB_STATUS_OK ||
        core_machine_plan_set_topology(plan, &profile.topology) != LIB_STATUS_OK ||
        core_machine_media_registry_create(&registry) != LIB_STATUS_OK ||
        registry == LIB_NULL ||
        core_machine_media_registry_bind(registry, 1u, media, &provider) != LIB_STATUS_OK ||
        core_machine_media_registry_bind(registry, 2u, empty, &provider) != LIB_STATUS_OK ||
        core_machine_media_registry_freeze(registry) != LIB_STATUS_OK ||
        core_machine_display_provider_slot_create(&display) != LIB_STATUS_OK ||
        display == LIB_NULL ||
        core_machine_plan_bind_display_provider(plan, display) != LIB_STATUS_OK ||
        core_machine_plan_bind_media_registry(plan, registry) != LIB_STATUS_OK ||
        profile.profile.configure(profile.profile.context, plan) != LIB_STATUS_OK ||
        core_machine_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK ||
        machine == LIB_NULL || board == LIB_NULL ||
        core_machine_bind_firmware_provider(machine,
            profile.firmware_provider,
            profile.firmware_context) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        !run_to(machine, 0x7c00u, 1000000u)) goto done;
    media->readonly = readonly;
    media->present = !absent;
    if (core_machine_memory_write(machine, 0x1fff0u, payload, sizeof(payload)) != LIB_STATUS_OK ||
        !run_to(machine, 0x7c00u + (lib_u32)offset, 6000000u) ||
        core_machine_memory_read(machine, 0x600u, result, sizeof(result)) != LIB_STATUS_OK) goto done;
    ok = (result[0] >> 8u) == expected && (result[7] & 1u) == (expected != 0u) &&
        result[1] == 0xfff0u && result[2] == cx && result[3] == dx &&
        result[4] == 0x1357u && result[5] == 0x2468u && result[6] == 0x369cu &&
        result[8] == 0x1234u && result[9] == 0x1000u;
    if (ok && expected == 0u && (ax >> 8u) == 2u) {
        lib_size lba = ((cx >> 8u) * 2u + (dx >> 8u)) * 18u + (cx & 0xffu) - 1u;
        ok = (result[0] & 0xffu) == (ax & 0xffu) &&
            core_machine_memory_read(machine, 0x1fff0u, actual, (ax & 0xffu) * 512u) == LIB_STATUS_OK &&
            lib_memory_compare(actual, media->bytes + lba * 512u, (ax & 0xffu) * 512u) == 0;
    }
    if (ok && expected == 0u && (ax >> 8u) == 3u) {
        lib_size lba = ((cx >> 8u) * 2u + (dx >> 8u)) * 18u + (cx & 0xffu) - 1u;
        ok = lib_memory_compare(payload, media->bytes + lba * 512u, (ax & 0xffu) * 512u) == 0;
    }
    if (ok && recalibrate) {
        lib_u16 prior[10], reset[10];
        ok = core_machine_memory_read(machine, 0x620u, prior, sizeof(prior)) == LIB_STATUS_OK &&
            core_machine_memory_read(machine, 0x640u, reset, sizeof(reset)) == LIB_STATUS_OK &&
            prior[0] == 1u && (prior[7] & 1u) == 0u && reset[6] == 0x369cu &&
            reset[0] == 0u && (reset[7] & 1u) == 0u;
    }
    if (ok && expected != 0u) {
        lib_u16 status[10];
        ok = core_machine_memory_read(machine, 0x660u, status, sizeof(status)) == LIB_STATUS_OK &&
            (status[0] >> 8u) == expected && (status[7] & 1u) != 0u;
    }
    if (ok && expected == 0x80u) {
        lib_u16 recovered[10];
        ok = core_machine_memory_read(machine, 0x680u, recovered, sizeof(recovered)) == LIB_STATUS_OK &&
            recovered[0] == 1u && (recovered[7] & 1u) == 0u &&
            core_machine_memory_read(machine, 0x1fff0u, actual, 512u) == LIB_STATUS_OK &&
            lib_memory_compare(actual, media->bytes + 36u * 512u, 512u) == 0;
    }
done:
    if (machine == LIB_NULL) lib_c_fprintf(lib_c_stderr, "Fixture construction failed\n");
    if (!ok) lib_c_fprintf(lib_c_stderr, "BIOS floppy AX=%04x CX=%04x DX=%04x: result=%04x flags=%04x\n",
        ax, cx, dx, result[0], result[7]);
    core_machine_destroy(machine);
    core_machine_display_provider_slot_destroy(display);
    core_machine_media_registry_destroy(registry);
    core_machine_plan_destroy(plan);
    if (profile.profile.release != LIB_NULL) profile.profile.release(profile.profile.context);
    lib_release(empty); lib_release(media);
    return ok;
}

lib_i32 main(void)
{
    if (!check(0x0202u, 0x0112u, 0u, 0u, LIB_FALSE, LIB_FALSE, LIB_FALSE) ||
        !check(0x0302u, 0x0112u, 0u, 0u, LIB_FALSE, LIB_FALSE, LIB_FALSE) ||
        !check(0x0201u, 0x0101u, 0u, 0u, LIB_FALSE, LIB_FALSE, LIB_TRUE) ||
        !check(0x0200u, 1u, 0u, 4u, LIB_FALSE, LIB_FALSE, LIB_FALSE) ||
        !check(0x0201u, 0u, 0u, 4u, LIB_FALSE, LIB_FALSE, LIB_FALSE) ||
        !check(0x0201u, 0x5001u, 0u, 4u, LIB_FALSE, LIB_FALSE, LIB_FALSE) ||
        !check(0x0201u, 1u, 1u, 1u, LIB_FALSE, LIB_FALSE, LIB_FALSE) ||
        !check(0x9900u, 1u, 0u, 1u, LIB_FALSE, LIB_FALSE, LIB_FALSE) ||
        !check(0u, 1u, 0u, 0x80u, LIB_FALSE, LIB_FALSE, LIB_FALSE) ||
        !check(0x0301u, 2u, 0u, 3u, LIB_TRUE, LIB_FALSE, LIB_FALSE) ||
        !check(0x0201u, 2u, 0u, 4u, LIB_FALSE, LIB_TRUE, LIB_FALSE)) return 1;
    lib_c_printf("Project BIOS floppy read/write/CHS/buffer/reset/error contracts: OK\n");
    return 0;
}
