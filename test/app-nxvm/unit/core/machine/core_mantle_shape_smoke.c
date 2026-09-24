#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/entry_plan_interface.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/devices/rtc.h"
#include "../devices/support/core_machine_cpu_fixture.h"

typedef struct mantle_fixture {
    core_machine_rtc rtc;
    core_machine_media_registry *media;
    lib_u8 media_byte;
} mantle_fixture;

static void fixture_reset(void *context)
{
    mantle_fixture *fixture = (mantle_fixture *)context;
    if (fixture != LIB_NULL) core_machine_rtc_reset(&fixture->rtc);
}

static void fixture_advance(void *context, lib_u64 elapsed_ticks)
{
    mantle_fixture *fixture = (mantle_fixture *)context;
    if (fixture != LIB_NULL) core_machine_rtc_advance(&fixture->rtc, elapsed_ticks);
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
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_rtc_config rtc_config = { .irq = 8u, .ticks_per_second = 1u };
    const core_machine_entry_plan_preload preload = { 0x0200u, halt, sizeof(halt) };
    core_machine_entry_plan plan = { 0 };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_media_info media_info;
    core_machine_media_result media_result;
    core_machine *machine = LIB_NULL;
    mantle_fixture fixture = { 0 };
    lib_i32 failed = 0;

    fixture.media_byte = 0xa5u;
    if (core_machine_media_registry_create(&fixture.media) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine) != LIB_STATUS_OK) failed |= 0x01;
    if (!failed) {
        test_core_machine_fixture_initialize_rtc_with_shared_pic(machine,
            &fixture.rtc, &rtc_config);
        if (core_machine_bind_execution_provider(machine,
            &fixture_execution_provider, &fixture) != LIB_STATUS_OK) failed |= 0x02;
        if (core_machine_media_registry_bind(fixture.media, 1u,
            &fixture.media_byte, &fixture_media_provider) != LIB_STATUS_OK) failed |= 0x04;
        if (core_machine_media_registry_freeze(fixture.media) != LIB_STATUS_OK) failed |= 0x08;
        if (core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK) failed |= 0x10;
        if (core_machine_reset(machine) != LIB_STATUS_OK) failed |= 0x20;
    }
    plan.state.ip = 0x0200u;
    plan.state.sp = 0x1000u;
    plan.state.eflags = VCPU_EFLAGS_IF;
    plan.entry_physical = 0x0200u;
    plan.entry_route = CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM;
    plan.preloads = &preload;
    plan.preload_count = 1u;
    if (!failed) {
        if (core_machine_apply_entry_plan(machine, &plan) != LIB_STATUS_OK) failed |= 0x40;
        if (core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) failed |= 0x80;
        if (core_machine_media_query(fixture.media, 1u, &media_info,
            &media_result) != LIB_STATUS_OK || media_result != CORE_MACHINE_MEDIA_RESULT_OK ||
            !media_info.present) failed |= 0x100;
    }
    core_machine_destroy(machine);
    core_machine_rtc_finalize(&fixture.rtc);
    core_machine_media_registry_destroy(fixture.media);
    if (failed) {
        printf("mantle shape failed=%x reason=%u\n", failed, result.reason);
        return 1;
    }
    puts("M5:T274:S2:MANTLE-SHAPE:OK");
    return 0;
}
