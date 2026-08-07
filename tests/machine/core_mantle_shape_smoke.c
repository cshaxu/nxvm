#include "type.h"

#include "core/machine/entry_plan_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/media_interface.h"
#include "core/machine/rtc.h"
#include "core/platform/backing_resource_interface.h"

typedef struct mantle_fixture {
    core_machine_rtc rtc;
    core_machine_media_registry media;
    uint8_t media_byte;
    uint8_t backing_byte;
} mantle_fixture;

static C_VOID fixture_reset(C_VOID *context)
{
    mantle_fixture *fixture = (mantle_fixture *)context;
    if (fixture != STD_NULL) core_machine_rtc_reset(&fixture->rtc);
}

static C_VOID fixture_advance(C_VOID *context, uint64_t elapsed_ticks)
{
    mantle_fixture *fixture = (mantle_fixture *)context;
    if (fixture != STD_NULL) core_machine_rtc_advance(&fixture->rtc, elapsed_ticks);
}

static const core_machine_execution_provider fixture_execution_provider = {
    fixture_reset,
    STD_NULL,
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
    uint64_t offset, C_VOID *buffer, uint32_t byte_count)
{
    if (context == STD_NULL || buffer == STD_NULL || offset != 0u || byte_count != 1u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    *(uint8_t *)buffer = *(uint8_t *)context;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider fixture_media_provider = {
    fixture_media_query,
    fixture_media_read,
    STD_NULL,
    STD_NULL,
    STD_NULL
};

static core_platform_backing_resource_result fixture_backing_size(C_VOID *context,
    uint64_t *out_size)
{
    if (context == STD_NULL || out_size == STD_NULL) {
        return CORE_PLATFORM_BACKING_RESOURCE_PERMANENT;
    }
    *out_size = 1u;
    return CORE_PLATFORM_BACKING_RESOURCE_OK;
}

static core_platform_backing_resource_result fixture_backing_read(C_VOID *context,
    uint64_t offset, C_VOID *buffer, uint32_t requested, uint32_t *out_transferred)
{
    if (context == STD_NULL || buffer == STD_NULL || out_transferred == STD_NULL ||
        offset != 0u || requested != 1u) {
        return CORE_PLATFORM_BACKING_RESOURCE_INVALID_RANGE;
    }
    *(uint8_t *)buffer = *(uint8_t *)context;
    *out_transferred = 1u;
    return CORE_PLATFORM_BACKING_RESOURCE_OK;
}

static const core_platform_backing_resource_provider fixture_backing_provider = {
    fixture_backing_size,
    fixture_backing_read,
    STD_NULL,
    STD_NULL,
    STD_NULL
};

C_INT main(C_VOID)
{
    static const uint8_t halt[] = { 0xf4u };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_rtc_config rtc_config = { 8u, 1u };
    const core_machine_entry_plan_preload preload = { 0x0200u, halt, sizeof(halt) };
    core_machine_entry_plan plan = { 0 };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_platform_backing_resource backing;
    core_platform_backing_resource_result backing_result;
    core_machine_media_info media_info;
    core_machine_media_result media_result;
    core_machine *machine = STD_NULL;
    mantle_fixture fixture = { 0 };
    uint8_t observed = 0u;
    uint32_t transferred = 0u;
    C_INT failed = 0;

    fixture.media_byte = 0xa5u;
    fixture.backing_byte = 0x5au;
    core_machine_media_registry_initialize(&fixture.media);
    core_platform_backing_resource_initialize(&backing, &fixture.backing_byte,
        &fixture_backing_provider);
    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        core_machine_rtc_initialize(&fixture.rtc,
            core_machine_configuration_shared_pic_master_borrow(machine),
            core_machine_configuration_shared_pic_slave_borrow(machine), &rtc_config);
        if (core_machine_bind_execution_provider(machine,
            &fixture_execution_provider, &fixture) != TYPE_STATUS_OK) failed |= 0x02;
        if (core_machine_media_registry_bind(&fixture.media, 1u,
            &fixture.media_byte, &fixture_media_provider) != TYPE_STATUS_OK) failed |= 0x04;
        if (core_machine_media_registry_freeze(&fixture.media) != TYPE_STATUS_OK) failed |= 0x08;
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
        if (core_machine_media_query(&fixture.media, 1u, &media_info,
            &media_result) != TYPE_STATUS_OK || media_result != CORE_MACHINE_MEDIA_RESULT_OK ||
            !media_info.present) failed |= 0x100;
        if (core_platform_backing_resource_read(&backing, 0u, &observed,
            1u, &transferred, &backing_result) != TYPE_STATUS_OK ||
            backing_result != CORE_PLATFORM_BACKING_RESOURCE_OK ||
            transferred != 1u || observed != 0x5au) failed |= 0x200;
    }
    core_machine_destroy(machine);
    core_machine_rtc_finalize(&fixture.rtc);
    core_machine_media_registry_finalize(&fixture.media);
    if (failed) {
        STD_PRINTF("mantle shape failed=%x reason=%u\n", failed, result.reason);
        return 1;
    }
    puts("M5:T274:S2:MANTLE-SHAPE:OK");
    return 0;
}
