#include "x86-nxvm/board-base/hdc.h"

static lib_status x86_nxvm_board_hdc_connect(x86_nxvm_board_hdc *hdc,
    const x86_nxvm_board_media_registry *media_registry,
    x86_nxvm_board_media_id media_id, x86_nxvm_board_media_id slave_media_id,
    x86_nxvm_board_pic_bus *pic_master, x86_nxvm_board_pic_bus *pic_slave,
    const x86_nxvm_board_hdc_config *config);
static lib_status x86_nxvm_board_hdc_initialize(x86_nxvm_board_hdc *hdc);

lib_status x86_nxvm_board_hdc_create(x86_nxvm_board_hdc **out_hdc)
{
    if (out_hdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_hdc = lib_allocate_zero(1u, sizeof(**out_hdc));
    return *out_hdc == LIB_NULL ? LIB_STATUS_NO_MEMORY : LIB_STATUS_OK;
}

void x86_nxvm_board_hdc_destroy(x86_nxvm_board_hdc *hdc)
{
    x86_nxvm_board_hdc_clear_configuration(hdc);
    lib_release(hdc);
}

lib_status x86_nxvm_board_hdc_configure(x86_nxvm_board_hdc *hdc,
    const x86_nxvm_board_media_registry *media_registry,
    x86_nxvm_board_media_id media_id, x86_nxvm_board_media_id slave_media_id,
    x86_nxvm_board_pic_bus *pic_master, x86_nxvm_board_pic_bus *pic_slave,
    const x86_nxvm_board_hdc_config *config)
{
    lib_status status;
    if (hdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (hdc->chips[0] != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    status = x86_nxvm_board_hdc_connect(hdc, media_registry, media_id,
        slave_media_id, pic_master, pic_slave, config);
    if (status == LIB_STATUS_OK) status = x86_nxvm_board_hdc_initialize(hdc);
    if (status != LIB_STATUS_OK) x86_nxvm_board_hdc_clear_configuration(hdc);
    return status;
}

static x86_hdc_geometry hdc_geometry(x86_nxvm_board_media_geometry geometry)
{
    return (x86_hdc_geometry){geometry.logical_sector_count, geometry.bytes_per_sector,
        geometry.cylinders, geometry.heads, geometry.sectors_per_track};
}

static x86_hdc_record_result hdc_result(lib_status status, x86_nxvm_board_media_result result)
{
    if (status != LIB_STATUS_OK) return X86_HDC_RECORD_FAILURE;
    switch (result) {
    case X86_NXVM_BOARD_MEDIA_RESULT_OK: return X86_HDC_RECORD_OK;
    case X86_NXVM_BOARD_MEDIA_RESULT_ABSENT: return X86_HDC_RECORD_ABSENT;
    case X86_NXVM_BOARD_MEDIA_RESULT_INVALID_RANGE: return X86_HDC_RECORD_RANGE;
    case X86_NXVM_BOARD_MEDIA_RESULT_READ_ONLY: return X86_HDC_RECORD_PROTECTED;
    case X86_NXVM_BOARD_MEDIA_RESULT_BAD_TRACK: return X86_HDC_RECORD_BAD_TRACK;
    default: return X86_HDC_RECORD_FAILURE;
    }
}

static x86_nxvm_board_media_id hdc_media(const x86_nxvm_board_hdc *hdc, lib_u8 unit)
{
    return unit == 0u ? hdc->connect.media_id : hdc->connect.slave_media_id;
}

static x86_hdc_record_result hdc_query(void *context, lib_u8 unit, x86_hdc_medium *out)
{
    x86_nxvm_board_hdc *hdc = context;
    x86_nxvm_board_media_info info = {0};
    x86_nxvm_board_media_result result = X86_NXVM_BOARD_MEDIA_RESULT_ABSENT;
    lib_status status = x86_nxvm_board_media_query(hdc->connect.media_registry,
        hdc_media(hdc, unit), &info, &result);
    *out = (x86_hdc_medium){.geometry = hdc_geometry(info.geometry),
        .present = info.present,
        .read_only = (info.capabilities & X86_NXVM_BOARD_MEDIA_CAPABILITY_READ_ONLY) != 0u,
        .geometry_known = (info.capabilities & X86_NXVM_BOARD_MEDIA_CAPABILITY_GEOMETRY_KNOWN) != 0u};
    return hdc_result(status, result);
}

static x86_hdc_record_result hdc_read(void *context, lib_u8 unit,
    lib_u64 sector, x86_hdc_record *out_record)
{
    x86_nxvm_board_hdc *hdc = context;
    x86_nxvm_board_media_result result = X86_NXVM_BOARD_MEDIA_RESULT_ABSENT;
    x86_nxvm_board_media_record record = {0};
    lib_status status = x86_nxvm_board_media_read_record(hdc->connect.media_registry,
        hdc_media(hdc, unit), sector, &record, &result);
    if (status == LIB_STATUS_OK && result == X86_NXVM_BOARD_MEDIA_RESULT_OK) {
        lib_memory_copy(out_record->data, record.data, sizeof(record.data));
        lib_memory_copy(out_record->ecc, record.ecc, sizeof(record.ecc));
        out_record->ecc_present = record.ecc_present;
    }
    return hdc_result(status, result);
}

static x86_hdc_record_result hdc_write(void *context, lib_u8 unit,
    lib_u64 sector, const x86_hdc_record *source)
{
    x86_nxvm_board_hdc *hdc = context;
    x86_nxvm_board_media_result result = X86_NXVM_BOARD_MEDIA_RESULT_ABSENT;
    x86_nxvm_board_media_record record = {.ecc_present = source->ecc_present};
    lib_status status;
    lib_memory_copy(record.data, source->data, sizeof(record.data));
    lib_memory_copy(record.ecc, source->ecc, sizeof(record.ecc));
    status = x86_nxvm_board_media_write_record(hdc->connect.media_registry,
        hdc_media(hdc, unit), sector, &record, &result);
    return hdc_result(status, result);
}

static x86_hdc_record_result hdc_format_track(void *context, lib_u8 unit,
    const x86_hdc_track *source)
{
    x86_nxvm_board_hdc *hdc = context;
    x86_nxvm_board_media_track track = {.first_sector = source->first_sector,
        .sector_count = source->sector_count, .fill = source->fill,
        .fill_payload = source->fill_payload};
    x86_nxvm_board_media_result result = X86_NXVM_BOARD_MEDIA_RESULT_ABSENT;
    lib_status status;

    lib_memory_copy(track.ids, source->ids, sizeof(track.ids));
    lib_memory_copy(track.bad, source->bad, sizeof(track.bad));
    status = x86_nxvm_board_media_format_track(hdc->connect.media_registry,
        hdc_media(hdc, unit), &track, &result);
    return hdc_result(status, result);
}

static void hdc_irq(void *context, lib_bool asserted)
{
    x86_nxvm_board_hdc *hdc = context;
    (void)asserted;
    /* Both physical Compaq controllers share the board's IRQ14 input. */
    if (x86_nxvm_board_hdc_irq_pending(hdc)) x86_nxvm_board_pic_irq_source_assert(hdc->connect.irq_source);
    else x86_nxvm_board_pic_irq_source_deassert(hdc->connect.irq_source);
}

static void hdc_drq(void *context, lib_bool asserted)
{
    x86_nxvm_board_hdc *hdc = context;
    if (asserted) {
        if (hdc->connect.dma_request_assert != LIB_NULL)
            hdc->connect.dma_request_assert(hdc->connect.dma_request_owner, &hdc->connect.dma_request);
    } else if (hdc->connect.dma_request_deassert != LIB_NULL) {
        hdc->connect.dma_request_deassert(hdc->connect.dma_request_owner, &hdc->connect.dma_request);
    }
}

static lib_status hdc_register(const x86_nxvm_board_hdc *hdc, lib_u16 port,
    x86_hdc_register *out)
{
    const x86_nxvm_board_hdc_config *config = &hdc->connect.config;
    if (config->protocol == X86_NXVM_BOARD_HDC_PROTOCOL_XEBEC_XT) {
        if (port == config->bus.xebec.data_port) *out = X86_HDC_REGISTER_DATA;
        else if (port == config->bus.xebec.hardware_status_reset_port) *out = X86_HDC_REGISTER_XEBEC_RESET;
        else if (port == config->bus.xebec.jumpers_select_port) *out = X86_HDC_REGISTER_XEBEC_SELECT;
        else if (port == config->bus.xebec.dma_irq_mask_port) *out = X86_HDC_REGISTER_XEBEC_MASK;
        else return LIB_STATUS_UNSUPPORTED;
    } else {
        const x86_nxvm_board_hdc_task_file_config *bus = &config->bus.task_file;
        const lib_u16 ports[] = {bus->data_port, bus->error_features_port,
            bus->sector_count_port, bus->sector_number_port, bus->cylinder_low_port,
            bus->cylinder_high_port, bus->drive_head_port, bus->status_command_port,
            bus->alternate_status_device_control_port, bus->drive_address_port};
        const x86_hdc_register registers[] = {X86_HDC_REGISTER_DATA,
            X86_HDC_REGISTER_ERROR_FEATURES, X86_HDC_REGISTER_SECTOR_COUNT,
            X86_HDC_REGISTER_SECTOR_NUMBER, X86_HDC_REGISTER_CYLINDER_LOW,
            X86_HDC_REGISTER_CYLINDER_HIGH, X86_HDC_REGISTER_DRIVE_HEAD,
            X86_HDC_REGISTER_STATUS_COMMAND, X86_HDC_REGISTER_CONTROL,
            X86_HDC_REGISTER_DRIVE_ADDRESS};
        for (lib_size i = 0u; i < sizeof(ports) / sizeof(ports[0]); ++i) {
            if (port == ports[i]) {
                *out = registers[i];
                return LIB_STATUS_OK;
            }
        }
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

static lib_status hdc_port_read(void *context, lib_u16 port, lib_u64 tick,
    lib_u32 *out)
{
    (void)tick;
    x86_nxvm_board_hdc *hdc = context;
    x86_hdc_register reg;
    lib_status status;
    if (hdc == LIB_NULL || out == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out = 0u;
    status = hdc_register(hdc, port, &reg);
    if (status != LIB_STATUS_OK) return status;
    if (hdc->chips[0] == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    return x86_hdc_read(hdc->chips[hdc->selected_controller], reg, out);
}

static lib_status hdc_port_write(void *context, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    (void)access_bytes;
    x86_nxvm_board_hdc *hdc = context;
    x86_hdc_register reg;
    lib_status status;
    if (hdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = hdc_register(hdc, port, &reg);
    if (status != LIB_STATUS_OK) return status;
    if (hdc->chips[0] == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    if (reg == X86_HDC_REGISTER_CONTROL) {
        /* The multipurpose controller's reset/control lines reach both drives. */
        for (lib_u32 unit = 0u; unit < 2u; ++unit) {
            if (hdc->chips[unit] == LIB_NULL) continue;
            status = x86_hdc_write(hdc->chips[unit], reg, value);
            if (status != LIB_STATUS_OK) return status;
        }
        if ((value & X86_HDC_DEVICE_CONTROL_SRST) != 0u)
            hdc->selected_controller = 0u;
        return LIB_STATUS_OK;
    }
    if (reg == X86_HDC_REGISTER_DRIVE_HEAD &&
        hdc->connect.config.protocol == X86_NXVM_BOARD_HDC_PROTOCOL_COMPAQ_WD_40MB)
        hdc->selected_controller = (lib_u8)((value >> 4u) & 1u);
    return x86_hdc_write(hdc->chips[hdc->selected_controller], reg, value);
}

static void hdc_dma_read(void *context, t_latch *latch)
{
    x86_nxvm_board_hdc *hdc = context;
    if (hdc != LIB_NULL && latch != LIB_NULL)
        x86_hdc_dma_read(hdc->chips[0], &latch->data.byte);
}

static void hdc_dma_write(void *context, t_latch *latch)
{
    x86_nxvm_board_hdc *hdc = context;
    if (hdc != LIB_NULL && latch != LIB_NULL)
        x86_hdc_dma_write(hdc->chips[0], latch->data.byte);
}

static void hdc_dma_terminal(void *context, t_latch *latch)
{
    x86_nxvm_board_hdc *hdc = context;
    (void)latch;
    if (hdc != LIB_NULL) x86_hdc_terminal_count(hdc->chips[0]);
}

static lib_status x86_nxvm_board_hdc_connect(x86_nxvm_board_hdc *hdc,
    const x86_nxvm_board_media_registry *media_registry,
    x86_nxvm_board_media_id media_id, x86_nxvm_board_media_id slave_media_id,
    x86_nxvm_board_pic_bus *pic_master, x86_nxvm_board_pic_bus *pic_slave, const x86_nxvm_board_hdc_config *config)
{
    lib_status status;
    if (hdc == LIB_NULL || config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_pic_irq_source_bind(&hdc->connect.irq_source,
        pic_master, pic_slave, config->irq);
    if (status != LIB_STATUS_OK) return status;
    hdc->connect.media_registry = media_registry;
    hdc->connect.media_id = media_id;
    hdc->connect.slave_media_id = slave_media_id;
    hdc->connect.config = *config;
    return LIB_STATUS_OK;
}

void x86_nxvm_board_hdc_bind_dma_request(x86_nxvm_board_hdc *hdc,
    const x86_nxvm_board_dma_request_binding *binding,
    void (*request_assert)(void *owner,
        const x86_nxvm_board_dma_request_binding *binding),
    void (*request_deassert)(void *owner,
        const x86_nxvm_board_dma_request_binding *binding), void *owner)
{
    if (hdc == LIB_NULL || binding == LIB_NULL || request_assert == LIB_NULL ||
        request_deassert == LIB_NULL || owner == LIB_NULL) return;
    hdc->connect.dma_request = *binding;
    hdc->connect.dma_request_assert = request_assert;
    hdc->connect.dma_request_deassert = request_deassert;
    hdc->connect.dma_request_owner = owner;
}


static lib_status x86_nxvm_board_hdc_initialize(x86_nxvm_board_hdc *hdc)
{
    x86_hdc_config config = {0};
    x86_hdc_connection connection;
    const x86_nxvm_board_hdc_config *board;
    lib_u32 rate;

    if (hdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (hdc->chips[0] != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    board = &hdc->connect.config;
    switch (board->protocol) {
    case X86_NXVM_BOARD_HDC_PROTOCOL_ATA_PIO: config.protocol = X86_HDC_PROTOCOL_ATA_PIO; break;
    case X86_NXVM_BOARD_HDC_PROTOCOL_COMPAQ_WD_40MB: config.protocol = X86_HDC_PROTOCOL_COMPAQ_WD_40MB; break;
    case X86_NXVM_BOARD_HDC_PROTOCOL_IBM_WD1003_ST506: config.protocol = X86_HDC_PROTOCOL_IBM_WD1003_ST506; break;
    case X86_NXVM_BOARD_HDC_PROTOCOL_XEBEC_XT: config.protocol = X86_HDC_PROTOCOL_XEBEC_XT; break;
    default: return LIB_STATUS_INVALID_ARGUMENT;
    }
    config.command_ticks = board->service.command_ticks;
    config.next_sector_ticks = board->service.next_sector_ticks;
    if (board->protocol == X86_NXVM_BOARD_HDC_PROTOCOL_XEBEC_XT) {
        config.xebec_geometry = hdc_geometry(board->bus.xebec.expected_media_geometry);
    } else {
        config.lba28_supported = board->bus.task_file.lba28_supported;
        rate = board->bus.task_file.clock_ticks_per_second;
        for (lib_u32 i = 0u; i < 16u; ++i) {
            lib_u32 tenths_microsecond = i == 0u ? 350u : 5000u * i;
            if (board->protocol == X86_NXVM_BOARD_HDC_PROTOCOL_COMPAQ_WD_40MB) {
                if (i == 14u) tenths_microsecond = 32u;
                else if (i == 15u) tenths_microsecond = 160u;
            }
            config.step_ticks[i] = (lib_u32)(((lib_u64)rate * tenths_microsecond +
                9999999u) / 10000000u);
        }
    }
    connection = (x86_hdc_connection){.query = hdc_query, .read = hdc_read, .write = hdc_write,
        .irq = hdc_irq, .drq = hdc_drq, .context = hdc, .format_track = hdc_format_track};
    lib_status status = x86_hdc_create(&config, &connection, &hdc->chips[0]);
    if (status == LIB_STATUS_OK &&
        board->protocol == X86_NXVM_BOARD_HDC_PROTOCOL_COMPAQ_WD_40MB)
        status = x86_hdc_create(&config, &connection, &hdc->chips[1]);
    return status;
}

void x86_nxvm_board_hdc_reset(x86_nxvm_board_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    hdc->selected_controller = 0u;
    for (lib_u32 unit = 0u; unit < 2u; ++unit) x86_hdc_reset(hdc->chips[unit]);
}

void x86_nxvm_board_hdc_advance_at(x86_nxvm_board_hdc *hdc, lib_u64 now)
{
    if (hdc == LIB_NULL) return;
    for (lib_u32 unit = 0u; unit < 2u; ++unit) x86_hdc_advance_at(hdc->chips[unit], now);
}

lib_status x86_nxvm_board_hdc_next_due_tick(const x86_nxvm_board_hdc *hdc, lib_u64 *out)
{
    lib_bool found = LIB_FALSE;
    if (hdc == LIB_NULL || out == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_u32 unit = 0u; unit < 2u; ++unit) {
        lib_u64 due;
        lib_status status;
        if (hdc->chips[unit] == LIB_NULL) continue;
        status = x86_hdc_next_due_tick(hdc->chips[unit], &due);
        if (status == LIB_STATUS_INVALID_STATE) continue;
        if (status != LIB_STATUS_OK) return status;
        if (!found || due < *out) *out = due;
        found = LIB_TRUE;
    }
    return found ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
}

void x86_nxvm_board_hdc_clear_configuration(x86_nxvm_board_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    for (lib_u32 unit = 0u; unit < 2u; ++unit) {
        x86_hdc *chip = hdc->chips[unit];
        hdc->chips[unit] = LIB_NULL;
        x86_hdc_destroy(chip);
    }
    hdc->selected_controller = 0u;
    lib_memory_set(&hdc->connect, 0, sizeof(hdc->connect));
}

const x86_core_machine_port_provider *x86_nxvm_board_hdc_port_provider(void)
{
    static const x86_core_machine_port_provider provider = {hdc_port_read, hdc_port_write};
    return &provider;
}

const x86_nxvm_board_dma_channel_provider *x86_nxvm_board_hdc_dma_provider(void)
{
    static const x86_nxvm_board_dma_channel_provider provider = {hdc_dma_read, hdc_dma_write, hdc_dma_terminal};
    return &provider;
}

lib_u8 x86_nxvm_board_hdc_irq_pending(const x86_nxvm_board_hdc *hdc)
{
    return hdc != LIB_NULL && (x86_hdc_irq_pending(hdc->chips[0]) ||
        x86_hdc_irq_pending(hdc->chips[1]));
}
