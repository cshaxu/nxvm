#include "type.h"

#include "core/machine/entry_plan_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/media_interface.h"
#include "core/machine/rtc.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct mantle_fixture {
    core_machine_rtc rtc;
    core_machine_media_registry *media;
    type_unsigned_8 media_byte;
} mantle_fixture;

static C_VOID fixture_reset(C_VOID *context)
{
    mantle_fixture *fixture = (mantle_fixture *)context;
    if (fixture != STD_NULL) core_machine_rtc_reset(&fixture->rtc);
}

static C_VOID fixture_advance(C_VOID *context, type_unsigned_64 elapsed_ticks)
{
    mantle_fixture *fixture = (mantle_fixture *)context;
    if (fixture != STD_NULL) core_machine_rtc_advance(&fixture->rtc, elapsed_ticks);
}

static const core_machine_execution_provider fixture_execution_provider = {
    fixture_reset,
    fixture_advance
};

static core_machine_media_result fixture_media_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    if (context == STD_NULL || out_info == STD_NULL) {
        return CORE_MACHINE_MEDIA_RESULT_PERMANENT;
    }
    STD_MEMSET(out_info, 0, sizeof(*out_info));
    out_info->present = TYPE_TRUE;
    out_info->geometry.logical_sector_count = 1u;
    out_info->geometry.bytes_per_sector = 1u;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result fixture_media_read(C_VOID *context,
    type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 byte_count)
{
    if (context == STD_NULL || buffer == STD_NULL || offset != 0u || byte_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    *(type_unsigned_8 *)buffer = *(type_unsigned_8 *)context;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider fixture_media_provider = {
    fixture_media_query,
    fixture_media_read,
    STD_NULL,
    STD_NULL,
    STD_NULL,
    STD_NULL,
    STD_NULL
};

C_INT main(C_VOID)
{
    static const type_unsigned_8 halt[] = { 0xf4u };
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
    core_machine *machine = STD_NULL;
    mantle_fixture fixture = { 0 };
    C_INT failed = 0;

    fixture.media_byte = 0xa5u;
    if (core_machine_media_registry_create(&fixture.media) != TYPE_STATUS_OK ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        test_core_machine_fixture_initialize_rtc_with_shared_pic(machine,
            &fixture.rtc, &rtc_config);
        if (core_machine_bind_execution_provider(machine,
            &fixture_execution_provider, &fixture) != TYPE_STATUS_OK) failed |= 0x02;
        if (core_machine_media_registry_bind(fixture.media, 1u,
            &fixture.media_byte, &fixture_media_provider) != TYPE_STATUS_OK) failed |= 0x04;
        if (core_machine_media_registry_freeze(fixture.media) != TYPE_STATUS_OK) failed |= 0x08;
        if (core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK) failed |= 0x10;
        if (core_machine_reset(machine) != TYPE_STATUS_OK) failed |= 0x20;
    }
    plan.state.ip = 0x0200u;
    plan.state.sp = 0x1000u;
    plan.state.eflags = VCPU_EFLAGS_IF;
    plan.entry_physical = 0x0200u;
    plan.entry_route = CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM;
    plan.preloads = &preload;
    plan.preload_count = 1u;
    if (!failed) {
        if (core_machine_apply_entry_plan(machine, &plan) != TYPE_STATUS_OK) failed |= 0x40;
        if (core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) failed |= 0x80;
        if (core_machine_media_query(fixture.media, 1u, &media_info,
            &media_result) != TYPE_STATUS_OK || media_result != CORE_MACHINE_MEDIA_RESULT_OK ||
            !media_info.present) failed |= 0x100;
    }
    core_machine_destroy(machine);
    core_machine_rtc_finalize(&fixture.rtc);
    core_machine_media_registry_destroy(fixture.media);
    if (failed) {
        STD_PRINTF("mantle shape failed=%x reason=%u\n", failed, result.reason);
        return 1;
    }
    puts("M5:T274:S2:MANTLE-SHAPE:OK");
    return 0;
}
