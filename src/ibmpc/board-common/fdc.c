/* Copyright 2012-2026 Neko. */
/* PC adapter: register decode, drive mechanics/media and chip wiring. */
#include "ibmpc/board-common/fdc.h"

lib_status core_machine_fdc_create(core_machine_fdc **out_fdc)
{
    if (out_fdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_fdc = lib_allocate_zero(1u, sizeof(**out_fdc));
    return *out_fdc == LIB_NULL ? LIB_STATUS_NO_MEMORY : LIB_STATUS_OK;
}

void core_machine_fdc_destroy(core_machine_fdc *fdc)
{
    core_machine_fdc_finalize(fdc);
    lib_release(fdc);
}

lib_status core_machine_fdc_configure(core_machine_fdc *fdc,
    const core_machine_media_registry *media_registry,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_dma_request_binding *dma_request,
    core_machine_fdc_dma_request_operation request_assert,
    core_machine_fdc_dma_request_operation request_deassert,
    void *request_owner, core_machine_pic_bus *pic_master,
    core_machine_pic_bus *pic_slave, core_machine *machine,
    const core_machine_fdc_config *config,
    const core_machine_fdc_terminal_observation_provider *observation_provider)
{
    lib_status status;
    if (fdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (fdc->chip != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    status = core_machine_fdc_connect(fdc, media_registry, drives, dma_request,
        request_assert, request_deassert, request_owner, pic_master, pic_slave,
        machine, config, observation_provider);
    if (status == LIB_STATUS_OK) status = core_machine_fdc_initialize(fdc);
    if (status != LIB_STATUS_OK) core_machine_fdc_finalize(fdc);
    return status;
}

static core_machine_media_id core_machine_fdc_drive_media_id(
    const core_machine_fdc *fdc, lib_u8 drive)
{
    return fdc == LIB_NULL || drive >= CORE_MACHINE_FDC_DRIVE_COUNT ?
        CORE_MACHINE_MEDIA_ID_INVALID :
        fdc->connect.drives.media_id[drive];
}

static lib_i32 core_machine_fdc_drive_media_query(const core_machine_fdc *fdc,
    lib_u8 drive, core_machine_media_info *out_info,
    core_machine_media_result *out_result)
{
    core_machine_media_id media_id = core_machine_fdc_drive_media_id(fdc, drive);

    return fdc != LIB_NULL && fdc->connect.media_registry != LIB_NULL &&
        media_id != CORE_MACHINE_MEDIA_ID_INVALID &&
        core_machine_media_query(fdc->connect.media_registry, media_id, out_info,
            out_result) == LIB_STATUS_OK;
}

static lib_i32 core_machine_fdc_drive_media_ready(const core_machine_fdc *fdc,
    lib_u8 drive)
{
    core_machine_media_info info;
    core_machine_media_result result;

    return core_machine_fdc_drive_media_query(fdc, drive, &info, &result) &&
        result == CORE_MACHINE_MEDIA_RESULT_OK && info.present;
}

static void core_machine_fdc_update_dir(core_machine_fdc *fdc)
{
    lib_u8 drive;

    if (fdc == LIB_NULL) return;
    drive = fdc->data.dor & VFDC_DOR_DS;
    /* Disk Change is a signal from an installed mechanical unit.  An empty
     * fitted drive asserts it; an unpopulated select line cannot. */
    if ((fdc->data.dor & VFDC_DOR_ME(drive)) != 0u &&
        (fdc->connect.drives.installed_mask & (1u << drive)) != 0u &&
        (fdc->data.media_changed[drive] ||
         !core_machine_fdc_drive_media_ready(fdc, drive))) {
        fdc->data.dir |= VFDC_DIR_DC;
    } else {
        fdc->data.dir &= (lib_u8)~VFDC_DIR_DC;
    }
}

static void core_machine_fdc_observe_drive(core_machine_fdc *fdc,
    lib_u8 drive)
{
    core_machine_media_info info;
    core_machine_media_result result;

    if (!core_machine_fdc_drive_media_query(fdc, drive, &info, &result)) return;
    fdc->data.observed_media_generation[drive] = info.generation;
    fdc->data.media_changed[drive] = LIB_FALSE;
    core_machine_fdc_update_dir(fdc);
}

static void core_machine_fdc_observe_all_drives(core_machine_fdc *fdc)
{
    lib_u8 drive;

    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        core_machine_fdc_observe_drive(fdc, drive);
    }
}


static x86_fdc_pins core_machine_fdc_sample(void *context, lib_u8 unit)
{
    const core_machine_fdc *fdc = context;
    const lib_u8 drive = fdc->data.dor & VFDC_DOR_DS;
    core_machine_media_info info = {0};
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_ABSENT;
    lib_bool media_ok = core_machine_fdc_drive_media_query(fdc, unit, &info, &result) &&
        result == CORE_MACHINE_MEDIA_RESULT_OK;
    return (x86_fdc_pins) {
        .ready = (fdc->connect.config.ready_mask & (1u << unit)) != 0u,
        .track_zero = (fdc->connect.drives.installed_mask & (1u << drive)) != 0u &&
            ((fdc->drive_cylinder[drive] == 0u) !=
            ((fdc->connect.drives.track_zero_active_low_mask & (1u << drive)) != 0u)),
        .write_protected = media_ok &&
            (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) != 0u,
        .two_sided = (fdc->connect.drives.double_sided_mask & (1u << unit)) != 0u,
        .data_available = unit == drive && (fdc->data.dor & VFDC_DOR_NRS) != 0u &&
            (fdc->data.dor & VFDC_DOR_ME(drive)) != 0u &&
            (fdc->connect.drives.installed_mask & (1u << drive)) != 0u &&
            media_ok && info.present
    };
}

static void core_machine_fdc_step(void *context, lib_u8 unit, lib_bool outward)
{
    core_machine_fdc *fdc = context;
    const lib_u8 drive = fdc->data.dor & VFDC_DOR_DS;
    const lib_u16 count = fdc->connect.drives.cylinder_count[drive];
    (void)unit;
    if ((fdc->connect.drives.installed_mask & (1u << drive)) == 0u) return;
    if (outward) {
        if (fdc->drive_cylinder[drive] != 0u) --fdc->drive_cylinder[drive];
    } else if (fdc->drive_cylinder[drive] < (count == 0u ? 255u : count - 1u)) {
        ++fdc->drive_cylinder[drive];
    }
    core_machine_fdc_observe_drive(fdc, drive);
}

static lib_bool core_machine_fdc_track(void *context, lib_u8 unit, lib_bool mfm,
    x86_fdc_track *out_track)
{
    const core_machine_fdc *fdc = context;
    const lib_u8 drive = fdc->data.dor & VFDC_DOR_DS;
    static const lib_u32 rates[] = {500000u, 300000u, 250000u};
    const lib_u8 rate = fdc->connect.config.control_port == 0u ? VFDC_CCR_RATE_250 : fdc->data.ccr;
    core_machine_media_info info = {0};
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_ABSENT;
    *out_track = (x86_fdc_track) {.cylinder = fdc->drive_cylinder[drive]};
    if (!core_machine_fdc_drive_media_query(fdc, unit, &info, &result) ||
        result != CORE_MACHINE_MEDIA_RESULT_OK ||
        info.geometry.cylinders > 65535u || info.geometry.heads > 65535u ||
        info.geometry.sectors_per_track > 65535u || info.geometry.bytes_per_sector > 65535u)
        return LIB_FALSE;
    out_track->cylinders = (lib_u16)info.geometry.cylinders;
    out_track->heads = (lib_u16)info.geometry.heads;
    out_track->sectors = (lib_u16)info.geometry.sectors_per_track;
    out_track->bytes_per_sector = (lib_u16)info.geometry.bytes_per_sector;
    if (rate >= sizeof(rates) / sizeof(rates[0])) return LIB_TRUE;
    if (fdc->connect.drives.channel.sample != LIB_NULL)
        out_track->id_readable = fdc->connect.drives.channel.sample(fdc->connect.drives.channel.context,
            drive, &info.geometry, fdc->drive_cylinder[drive], rates[rate], mfm,
            &out_track->cylinder);
    else out_track->id_readable = out_track->cylinder < info.geometry.cylinders;
    return LIB_TRUE;
}

static x86_fdc_record_result core_machine_fdc_record_result(lib_status status,
    core_machine_media_result result)
{
    if (status != LIB_STATUS_OK) return X86_FDC_RECORD_FAILURE;
    switch (result) {
    case CORE_MACHINE_MEDIA_RESULT_OK: return X86_FDC_RECORD_OK;
    case CORE_MACHINE_MEDIA_RESULT_READ_ONLY: return X86_FDC_RECORD_PROTECTED;
    case CORE_MACHINE_MEDIA_RESULT_ABSENT:
    case CORE_MACHINE_MEDIA_RESULT_CHANGED:
    case CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE: return X86_FDC_RECORD_ABSENT;
    default: return X86_FDC_RECORD_FAILURE;
    }
}

static lib_bool core_machine_fdc_record_offset(const core_machine_fdc *fdc,
    lib_u8 unit, const x86_fdc_record *record, core_machine_media_info *info,
    lib_u64 *out_offset)
{
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_ABSENT;
    if (!core_machine_fdc_drive_media_query(fdc, unit, info, &result) ||
        result != CORE_MACHINE_MEDIA_RESULT_OK || record->head >= info->geometry.heads ||
        record->cylinder >= info->geometry.cylinders || record->sector == 0u ||
        record->sector > info->geometry.sectors_per_track ||
        info->geometry.bytes_per_sector != 512u || record->offset >= 512u) return LIB_FALSE;
    *out_offset = (((lib_u64)record->cylinder * info->geometry.heads +
        record->head) * info->geometry.sectors_per_track + record->sector - 1u) *
        512u + record->offset;
    return LIB_TRUE;
}

static x86_fdc_record_result core_machine_fdc_record_read(void *context, lib_u8 unit,
    const x86_fdc_record *record, lib_u8 *out_byte)
{
    core_machine_fdc *fdc = context;
    core_machine_media_info info;
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_ABSENT;
    lib_u64 offset;
    lib_status status;
    if (!core_machine_fdc_record_offset(fdc, unit, record, &info, &offset))
        return X86_FDC_RECORD_ABSENT;
    status = core_machine_media_read_bytes(fdc->connect.media_registry,
        core_machine_fdc_drive_media_id(fdc, unit), offset, out_byte, 1u, &result);
    return core_machine_fdc_record_result(status, result);
}

static x86_fdc_record_result core_machine_fdc_record_write(void *context, lib_u8 unit,
    const x86_fdc_record *record, lib_u8 byte)
{
    core_machine_fdc *fdc = context;
    core_machine_media_info info;
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_ABSENT;
    lib_u64 offset;
    lib_status status;
    if (!core_machine_fdc_record_offset(fdc, unit, record, &info, &offset))
        return X86_FDC_RECORD_ABSENT;
    status = core_machine_media_write_bytes(fdc->connect.media_registry,
        core_machine_fdc_drive_media_id(fdc, unit), offset, &byte, 1u, &result);
    return core_machine_fdc_record_result(status, result);
}

static x86_fdc_record_result core_machine_fdc_mark_read(void *context, lib_u8 unit,
    const x86_fdc_record *record, lib_bool *out_deleted)
{
    core_machine_fdc *fdc = context;
    core_machine_media_info info;
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_ABSENT;
    core_machine_media_address_mark mark = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    lib_u64 offset;
    lib_status status = LIB_STATUS_OK;
    *out_deleted = LIB_FALSE;
    if (!core_machine_fdc_record_offset(fdc, unit, record, &info, &offset))
        return X86_FDC_RECORD_ABSENT;
    if ((info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS) == 0u)
        return X86_FDC_RECORD_OK;
    status = core_machine_media_get_address_mark(fdc->connect.media_registry,
        core_machine_fdc_drive_media_id(fdc, unit), offset / 512u, &mark, &result);
    *out_deleted = mark == CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA;
    return core_machine_fdc_record_result(status, result);
}

static x86_fdc_record_result core_machine_fdc_mark_write(void *context, lib_u8 unit,
    const x86_fdc_record *record, lib_bool deleted)
{
    core_machine_fdc *fdc = context;
    core_machine_media_info info;
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_ABSENT;
    lib_u64 offset;
    lib_status status;
    if (!core_machine_fdc_record_offset(fdc, unit, record, &info, &offset))
        return X86_FDC_RECORD_ABSENT;
    status = core_machine_media_set_address_mark(fdc->connect.media_registry,
        core_machine_fdc_drive_media_id(fdc, unit), offset / 512u,
        deleted ? CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA :
            CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA, &result);
    return core_machine_fdc_record_result(status, result);
}

static x86_fdc_record_result core_machine_fdc_record_format(void *context, lib_u8 unit,
    const x86_fdc_record *record, lib_u8 fill)
{
    core_machine_fdc *fdc = context;
    core_machine_media_info info;
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_ABSENT;
    lib_u64 offset;
    lib_status status;
    if (!core_machine_fdc_record_offset(fdc, unit, record, &info, &offset))
        return X86_FDC_RECORD_ABSENT;
    status = core_machine_media_format_sectors(fdc->connect.media_registry,
        core_machine_fdc_drive_media_id(fdc, unit), offset / 512u, 1u, fill, &result);
    return core_machine_fdc_record_result(status, result);
}

static void core_machine_fdc_irq(void *context, lib_bool asserted)
{
    core_machine_fdc *fdc = context;
    if (asserted) core_machine_pic_irq_source_assert(fdc->connect.irq_source);
    else core_machine_pic_irq_source_deassert(fdc->connect.irq_source);
}

static void core_machine_fdc_drq(void *context, lib_bool asserted)
{
    core_machine_fdc *fdc = context;
    core_machine_fdc_dma_request_operation operation = asserted ?
        fdc->connect.dma_request_assert : fdc->connect.dma_request_deassert;
    if (operation != LIB_NULL) operation(fdc->connect.dma_request_owner, &fdc->connect.dma_request);
}

static void core_machine_fdc_terminal(void *context, const x86_fdc_terminal *result)
{
    core_machine_fdc *fdc = context;
    core_machine_fdc_terminal_observation observation = {
        .sequence = ++fdc->connect.observation_sequence, .command = result->command,
        .drive = result->drive, .successful = result->successful
    };
    lib_memory_copy(observation.result, result->result, sizeof(observation.result));
    if (fdc->connect.observation_provider.callback != LIB_NULL)
        fdc->connect.observation_provider.callback(fdc->connect.observation_provider.context,
            &observation);
}

static lib_u64 core_machine_fdc_duration(const core_machine_fdc *fdc, lib_u64 microseconds)
{
    lib_u64 frequency = fdc->connect.config.clock_ticks_per_second;
    lib_u64 scaled;
    if (frequency == 0u || microseconds > LIB_UINT64_MAX / frequency) return 0u;
    scaled = frequency * microseconds;
    return scaled / 1000000u + (scaled % 1000000u != 0u);
}

static x86_fdc_timing core_machine_fdc_timing(const core_machine_fdc *fdc)
{
    lib_u64 fm = 31u, mfm = 15u;
    if (fdc->data.ccr == VFDC_CCR_RATE_300) {
        fm = (fm * 5u + 2u) / 3u;
        mfm = (mfm * 5u + 2u) / 3u;
    } else if (fdc->data.ccr != VFDC_CCR_RATE_500) {
        fm = 0u;
        mfm = 0u;
    }
    return (x86_fdc_timing) {
        .reset_ticks = core_machine_fdc_duration(fdc, 1024u),
        .step_numerator = fdc->connect.config.clock_ticks_per_second,
        .step_denominator = 1000u,
        .fm_byte_ticks = core_machine_fdc_duration(fdc, fm),
        .mfm_byte_ticks = core_machine_fdc_duration(fdc, mfm)
    };
}

static void core_machine_fdc_dma_read(void *owner, t_latch *latch)
{
    core_machine_fdc *fdc = owner;
    x86_fdc_dma_read(fdc->chip, &latch->data.byte);
}

static void core_machine_fdc_dma_write(void *owner, t_latch *latch)
{
    core_machine_fdc *fdc = owner;
    x86_fdc_dma_write(fdc->chip, latch->data.byte);
}

static void core_machine_fdc_dma_terminal(void *owner, t_latch *latch)
{
    core_machine_fdc *fdc = owner;
    (void)latch;
    x86_fdc_terminal_count(fdc->chip);
}

const core_machine_dma_channel_provider *core_machine_fdc_dma_provider(void)
{
    static const core_machine_dma_channel_provider provider = {
        core_machine_fdc_dma_read, core_machine_fdc_dma_write, core_machine_fdc_dma_terminal
    };
    return &provider;
}

static lib_status core_machine_fdc_port_read(void *owner, lib_u16 id, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    core_machine_fdc *fdc = owner;
    lib_u8 value;

    if (fdc == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    value = (lib_u8)*out_value;
    if (id == fdc->connect.config.status_port) {
        value = x86_fdc_read_status(fdc->chip);
    } else if (id == fdc->connect.config.data_port) {
        x86_fdc_read_data(fdc->chip, &value);
    } else if (id == fdc->connect.config.direction_port) {
        /* Media change is sampled only when the guest observes DIR. */
        core_machine_fdc_refresh(fdc);
        value = fdc->data.dir;
    } else if (id == fdc->connect.config.diagnostic_port) {
        value = fdc->connect.config.diagnostic_read_value;
    } else {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_value = (*out_value & ~0xffu) | value;
    return LIB_STATUS_OK;
}

static lib_status core_machine_fdc_port_write(void *owner, lib_u16 id,
    lib_u32 value)
{
    core_machine_fdc *fdc = owner;

    if (fdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (id == fdc->connect.config.dor_port) {
        fdc->data.dor = (lib_u8)value;
        x86_fdc_set_service_enabled(fdc->chip, (fdc->data.dor & VFDC_DOR_ENRQ) != 0u);
        x86_fdc_set_reset(fdc->chip, (fdc->data.dor & VFDC_DOR_NRS) != 0u);
        core_machine_fdc_update_dir(fdc);
        x86_fdc_refresh(fdc->chip);
    } else if (id == fdc->connect.config.data_port) {
        x86_fdc_write_data(fdc->chip, (lib_u8)value);
    } else if (id == fdc->connect.config.control_port) {
        x86_fdc_timing timing;
        fdc->data.ccr = (lib_u8)value & VFDC_CCR_RATE_MASK;
        timing = core_machine_fdc_timing(fdc);
        (void)x86_fdc_set_timing(fdc->chip, &timing);
    } else {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_fdc_connect(core_machine_fdc *fdc,
    const core_machine_media_registry *media_registry,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_dma_request_binding *dma_request,
    core_machine_fdc_dma_request_operation dma_request_assert,
    core_machine_fdc_dma_request_operation dma_request_deassert,
    void *dma_request_owner, core_machine_pic_bus *pic_master, core_machine_pic_bus *pic_slave,
    core_machine *machine, const core_machine_fdc_config *config,
    const core_machine_fdc_terminal_observation_provider *observation_provider)
{
    lib_status status;
    if (fdc == LIB_NULL || drives == LIB_NULL || dma_request == LIB_NULL ||
        dma_request_assert == LIB_NULL || dma_request_deassert == LIB_NULL ||
        dma_request_owner == LIB_NULL || config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_pic_irq_source_bind(&fdc->connect.irq_source,
        pic_master, pic_slave, config->irq);
    if (status != LIB_STATUS_OK) return status;
    fdc->connect.media_registry = media_registry;
    fdc->connect.drives = *drives;
    fdc->connect.dma_request = *dma_request;
    fdc->connect.dma_request_assert = dma_request_assert;
    fdc->connect.dma_request_deassert = dma_request_deassert;
    fdc->connect.dma_request_owner = dma_request_owner;
    fdc->connect.machine = machine;
    fdc->connect.config = *config;
    if (observation_provider != LIB_NULL) {
        fdc->connect.observation_provider = *observation_provider;
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_fdc_initialize(core_machine_fdc *fdc)
{
    lib_status status;
    x86_fdc_connection connection;
    x86_fdc_timing timing;
    core_machine_port_route routes[7];
    lib_size route_count = 0u;
    if (fdc == LIB_NULL || fdc->connect.machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (fdc->chip != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    lib_memory_set(fdc->drive_cylinder, 0u, sizeof(fdc->drive_cylinder));
    lib_memory_set(&fdc->data, 0u, sizeof(fdc->data));
    fdc->data.ccr = VFDC_CCR_RATE_250;
    core_machine_fdc_observe_all_drives(fdc);
    fdc->data.initial_media_baseline_pending = LIB_TRUE;
    core_machine_fdc_update_dir(fdc);
    timing = core_machine_fdc_timing(fdc);
    connection = (x86_fdc_connection) {
        .sample = core_machine_fdc_sample, .step = core_machine_fdc_step,
        .track = core_machine_fdc_track, .read = core_machine_fdc_record_read,
        .write = core_machine_fdc_record_write, .read_mark = core_machine_fdc_mark_read,
        .write_mark = core_machine_fdc_mark_write, .format = core_machine_fdc_record_format,
        .irq = core_machine_fdc_irq, .drq = core_machine_fdc_drq,
        .terminal = core_machine_fdc_terminal, .context = fdc
    };
    status = x86_fdc_create(&connection, &timing, &fdc->chip);
    if (status != LIB_STATUS_OK) return status;
    routes[route_count++] = (core_machine_port_route) {
        .address = fdc->connect.config.status_port,
        .read = core_machine_fdc_port_read, .owner = fdc
    };
    routes[route_count++] = (core_machine_port_route) {
        .address = fdc->connect.config.data_port,
        .read = core_machine_fdc_port_read, .owner = fdc
    };
    if (fdc->connect.config.direction_port != 0u) {
        routes[route_count++] = (core_machine_port_route) {
            .address = fdc->connect.config.direction_port,
            .read = core_machine_fdc_port_read, .owner = fdc
        };
    }
    if (fdc->connect.config.diagnostic_port != 0u) {
        routes[route_count++] = (core_machine_port_route) {
            .address = fdc->connect.config.diagnostic_port,
            .read = core_machine_fdc_port_read, .owner = fdc
        };
    }
    routes[route_count++] = (core_machine_port_route) {
        .address = fdc->connect.config.dor_port,
        .write = core_machine_fdc_port_write, .owner = fdc
    };
    routes[route_count++] = (core_machine_port_route) {
        .address = fdc->connect.config.data_port,
        .write = core_machine_fdc_port_write, .owner = fdc
    };
    if (fdc->connect.config.control_port != 0u) {
        routes[route_count++] = (core_machine_port_route) {
            .address = fdc->connect.config.control_port,
            .write = core_machine_fdc_port_write, .owner = fdc
        };
    }
    status = core_machine_install_port_routes(fdc->connect.machine, routes, route_count);
    if (status != LIB_STATUS_OK) {
        x86_fdc_destroy(fdc->chip);
        fdc->chip = LIB_NULL;
    }
    return status;
}
void core_machine_fdc_reset(core_machine_fdc *fdc)
{
    if (fdc == LIB_NULL) return;
    x86_fdc_reset(fdc->chip);
    fdc->data.dor = 0u;
    fdc->data.dir = 0u;
    lib_memory_set(fdc->data.media_changed, 0u, sizeof(fdc->data.media_changed));
    if (fdc->data.initial_media_baseline_pending) {
        core_machine_fdc_observe_all_drives(fdc);
        fdc->data.initial_media_baseline_pending = LIB_FALSE;
    }
}

void core_machine_fdc_advance_at(core_machine_fdc *fdc, lib_u64 elapsed_ticks)
{
    if (fdc != LIB_NULL) x86_fdc_advance_at(fdc->chip, elapsed_ticks);
}

lib_status core_machine_fdc_next_due_tick(const core_machine_fdc *fdc, lib_u64 *out_due_tick)
{
    return fdc == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        x86_fdc_next_due_tick(fdc->chip, out_due_tick);
}

void core_machine_fdc_refresh(core_machine_fdc *fdc)
{
    if (fdc == LIB_NULL) return;
    for (lib_u8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        core_machine_media_info info;
        core_machine_media_result result;
        if (core_machine_fdc_drive_media_query(fdc, drive, &info, &result) &&
            fdc->data.observed_media_generation[drive] != info.generation)
            fdc->data.media_changed[drive] = LIB_TRUE;
    }
    x86_fdc_poll_ready(fdc->chip);
    core_machine_fdc_update_dir(fdc);
}

void core_machine_fdc_finalize(core_machine_fdc *fdc)
{
    if (fdc == LIB_NULL) return;
    x86_fdc_destroy(fdc->chip);
    fdc->chip = LIB_NULL;
    lib_memory_set(&fdc->data, 0u, sizeof(fdc->data));
    lib_memory_set(&fdc->connect, 0u, sizeof(fdc->connect));
    lib_memory_set(fdc->drive_cylinder, 0u, sizeof(fdc->drive_cylinder));
}
