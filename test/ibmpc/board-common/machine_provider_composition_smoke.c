#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86/core/entry_plan_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "ibmpc/board-common/media_interface.h"
#include "x86/chips/rtc146818/rtc146818_interface.h"

typedef struct mantle_fixture {
    x86_rtc *rtc;
    core_machine_media_registry *media;
    lib_u8 media_byte;
} mantle_fixture;

static void fixture_reset(void *context)
{
    mantle_fixture *fixture = (mantle_fixture *)context;
    if (fixture != LIB_NULL) x86_rtc_reset(fixture->rtc);
}

static void fixture_advance(void *context, lib_u64 elapsed_ticks)
{
    mantle_fixture *fixture = (mantle_fixture *)context;
    if (fixture != LIB_NULL) x86_rtc_advance(fixture->rtc, elapsed_ticks);
}

static const core_machine_execution_provider fixture_execution_provider = {
    fixture_reset,
    fixture_advance
};

static core_machine_media_result fixture_media_query(void *context,
    core_machine_media_info *out_info)
{
    if (context == LIB_NULL || out_info == LIB_NULL) {
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    }
    lib_memory_set(out_info, 0, sizeof(*out_info));
    out_info->present = LIB_TRUE;
    out_info->geometry.logical_sector_count = 1u;
    out_info->geometry.bytes_per_sector = 1u;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result fixture_media_read(void *context,
    lib_u64 offset, void *buffer, lib_u32 byte_count)
{
    if (context == LIB_NULL || buffer == LIB_NULL || offset != 0u || byte_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    *(lib_u8 *)buffer = *(lib_u8 *)context;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider fixture_media_provider = {
    fixture_media_query,
    fixture_media_read,
    LIB_NULL,
    LIB_NULL,
    LIB_NULL,
    LIB_NULL,
    LIB_NULL
};

lib_i32 main(void)
{
    static const lib_u8 halt[] = { 0xf4u };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const x86_rtc_config rtc_config = {1u, 0u, 0u};
    const core_machine_entry_plan_preload preload = { 0x0200u, halt, sizeof(halt) };
    core_machine_entry_plan plan = { 0 };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result = {0};
    core_machine_media_info media_info;
    core_machine_media_result media_result;
    core_machine *machine = LIB_NULL;
    mantle_fixture fixture = { 0 };
    lib_i32 failed = 0;

    fixture.media_byte = 0xa5u;
    if (core_machine_media_registry_create(&fixture.media) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK ||
        x86_rtc_create(&rtc_config, LIB_NULL, LIB_NULL, &fixture.rtc) !=
            LIB_STATUS_OK) failed |= 0x01;
    if (!failed) {
        if (core_machine_bind_execution_provider(machine,
            &fixture_execution_provider, &fixture) != LIB_STATUS_OK) failed |= 0x02;
        if (!failed && core_machine_media_registry_bind(fixture.media, 1u,
            &fixture.media_byte, &fixture_media_provider) != LIB_STATUS_OK) failed |= 0x04;
        if (!failed && core_machine_media_registry_freeze(fixture.media) != LIB_STATUS_OK) failed |= 0x08;
        if (!failed && core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK) failed |= 0x10;
        if (!failed && core_machine_reset(machine) != LIB_STATUS_OK) failed |= 0x20;
    }
    plan.state.ip = 0x0200u;
    plan.state.sp = 0x1000u;
    plan.state.eflags = 0x0200u; /* IF: allow the halted CPU to accept interrupts. */
    plan.entry_physical = 0x0200u;
    plan.entry_route = CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM;
    plan.preloads = &preload;
    plan.preload_count = 1u;
    if (!failed) {
        if (core_machine_apply_entry_plan(machine, &plan) != LIB_STATUS_OK) failed |= 0x40;
        if (!failed && (core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)) failed |= 0x80;
        if (!failed && (core_machine_media_query(fixture.media, 1u, &media_info,
            &media_result) != LIB_STATUS_OK || media_result != CORE_MACHINE_MEDIA_RESULT_OK ||
            !media_info.present)) failed |= 0x100;
    }
    core_machine_destroy(machine);
    x86_rtc_destroy(fixture.rtc);
    core_machine_media_registry_destroy(fixture.media);
    if (failed) {
        lib_c_printf("mantle shape failed=%x reason=%u\n", failed, result.reason);
        return 1;
    }
    lib_c_printf("%s\n", "M5:T274:S2:MANTLE-SHAPE:OK");
    return 0;
}
