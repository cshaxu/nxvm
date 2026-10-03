#include "app-nxvm/devices/hdc.h"

static x86_hdc_geometry hdc_geometry(core_machine_media_geometry geometry)
{
    return (x86_hdc_geometry){geometry.logical_sector_count, geometry.bytes_per_sector,
        geometry.cylinders, geometry.heads, geometry.sectors_per_track};
}

static x86_hdc_record_result hdc_result(lib_status status, core_machine_media_result result)
{
    if (status != LIB_STATUS_OK) return X86_HDC_RECORD_FAILURE;
    switch (result) {
    case CORE_MACHINE_MEDIA_RESULT_OK: return X86_HDC_RECORD_OK;
    case CORE_MACHINE_MEDIA_RESULT_ABSENT: return X86_HDC_RECORD_ABSENT;
    case CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE: return X86_HDC_RECORD_RANGE;
    case CORE_MACHINE_MEDIA_RESULT_READ_ONLY: return X86_HDC_RECORD_PROTECTED;
    default: return X86_HDC_RECORD_FAILURE;
    }
}

static core_machine_media_id hdc_media(const core_machine_hdc *hdc, lib_u8 unit)
{
    return unit == 0u ? hdc->connect.media_id : hdc->connect.slave_media_id;
}

static x86_hdc_record_result hdc_query(void *context, lib_u8 unit, x86_hdc_medium *out)
{
    core_machine_hdc *hdc = context;
    core_machine_media_info info = {0};
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_ABSENT;
    lib_status status = core_machine_media_query(hdc->connect.media_registry,
        hdc_media(hdc, unit), &info, &result);
    *out = (x86_hdc_medium){.geometry = hdc_geometry(info.geometry),
        .present = info.present,
        .read_only = (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) != 0u,
        .geometry_known = (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN) != 0u};
    return hdc_result(status, result);
}

static x86_hdc_record_result hdc_read(void *context, lib_u8 unit,
    lib_u64 sector, lib_u8 *data)
{
    core_machine_hdc *hdc = context;
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_ABSENT;
    lib_status status = core_machine_media_read_bytes(hdc->connect.media_registry,
        hdc_media(hdc, unit), sector * 512u, data, 512u, &result);
    return hdc_result(status, result);
}

static x86_hdc_record_result hdc_write(void *context, lib_u8 unit,
    lib_u64 sector, const lib_u8 *data)
{
    core_machine_hdc *hdc = context;
    core_machine_media_result result = CORE_MACHINE_MEDIA_RESULT_ABSENT;
    lib_status status = core_machine_media_write_bytes(hdc->connect.media_registry,
        hdc_media(hdc, unit), sector * 512u, data, 512u, &result);
    return hdc_result(status, result);
}

static void hdc_irq(void *context, lib_bool asserted)
{
    core_machine_hdc *hdc = context;
    if (asserted) core_machine_pic_irq_source_assert(&hdc->connect.irq_source);
    else core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
}

static void hdc_drq(void *context, lib_bool asserted)
{
    core_machine_hdc *hdc = context;
    if (asserted) {
        if (hdc->connect.dma_request_assert != LIB_NULL)
            hdc->connect.dma_request_assert(hdc->connect.dma_request_owner, &hdc->connect.dma_request);
    } else if (hdc->connect.dma_request_deassert != LIB_NULL) {
        hdc->connect.dma_request_deassert(hdc->connect.dma_request_owner, &hdc->connect.dma_request);
    }
}

static lib_status hdc_register(const core_machine_hdc *hdc, lib_u16 port,
    x86_hdc_register *out)
{
    const core_machine_hdc_config *config = &hdc->connect.config;
    if (config->protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT) {
        if (port == config->bus.xebec.data_port) *out = X86_HDC_REGISTER_DATA;
        else if (port == config->bus.xebec.hardware_status_reset_port) *out = X86_HDC_REGISTER_XEBEC_RESET;
        else if (port == config->bus.xebec.jumpers_select_port) *out = X86_HDC_REGISTER_XEBEC_SELECT;
        else if (port == config->bus.xebec.dma_irq_mask_port) *out = X86_HDC_REGISTER_XEBEC_MASK;
        else return LIB_STATUS_UNSUPPORTED;
    } else {
        const core_machine_hdc_task_file_config *bus = &config->bus.task_file;
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
    core_machine_hdc *hdc = context;
    x86_hdc_register reg;
    lib_status status;
    if (hdc == LIB_NULL || out == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out = 0u;
    status = hdc_register(hdc, port, &reg);
    return status == LIB_STATUS_OK ? x86_hdc_read(hdc->chip, reg, out) : status;
}

static lib_status hdc_port_write(void *context, lib_u16 port, lib_u32 value)
{
    core_machine_hdc *hdc = context;
    x86_hdc_register reg;
    lib_status status;
    if (hdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = hdc_register(hdc, port, &reg);
    return status == LIB_STATUS_OK ? x86_hdc_write(hdc->chip, reg, value) : status;
}

static void hdc_dma_read(void *context, t_latch *latch)
{
    core_machine_hdc *hdc = context;
    if (hdc != LIB_NULL && latch != LIB_NULL)
        x86_hdc_dma_read(hdc->chip, &latch->data.byte);
}

static void hdc_dma_write(void *context, t_latch *latch)
{
    core_machine_hdc *hdc = context;
    if (hdc != LIB_NULL && latch != LIB_NULL)
        x86_hdc_dma_write(hdc->chip, latch->data.byte);
}

static void hdc_dma_terminal(void *context, t_latch *latch)
{
    core_machine_hdc *hdc = context;
    (void)latch;
    if (hdc != LIB_NULL) x86_hdc_terminal_count(hdc->chip);
}

void core_machine_hdc_connect(core_machine_hdc *hdc,
    const core_machine_media_registry *media_registry,
    core_machine_media_id media_id, core_machine_media_id slave_media_id,
    core_machine_pic_bus *pic_master, core_machine_pic_bus *pic_slave, const core_machine_hdc_config *config)
{
    if (hdc == LIB_NULL || config == LIB_NULL) return;
    hdc->connect.media_registry = media_registry;
    hdc->connect.media_id = media_id;
    hdc->connect.slave_media_id = slave_media_id;
    core_machine_pic_irq_source_bind(&hdc->connect.irq_source, pic_master,
        pic_slave, config->irq);
    hdc->connect.config = *config;
}

void core_machine_hdc_bind_dma_request(core_machine_hdc *hdc,
    const core_machine_dma_request_binding *binding,
    void (*request_assert)(void *owner,
        const core_machine_dma_request_binding *binding),
    void (*request_deassert)(void *owner,
        const core_machine_dma_request_binding *binding), void *owner)
{
    if (hdc == LIB_NULL || binding == LIB_NULL || request_assert == LIB_NULL ||
        request_deassert == LIB_NULL || owner == LIB_NULL) return;
    hdc->connect.dma_request = *binding;
    hdc->connect.dma_request_assert = request_assert;
    hdc->connect.dma_request_deassert = request_deassert;
    hdc->connect.dma_request_owner = owner;
}


lib_status core_machine_hdc_initialize(core_machine_hdc *hdc)
{
    x86_hdc_config config = {0};
    x86_hdc_connection connection;
    const core_machine_hdc_config *board;
    lib_u32 rate;

    if (hdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (hdc->chip != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    board = &hdc->connect.config;
    switch (board->protocol) {
    case CORE_MACHINE_HDC_PROTOCOL_ATA_PIO: config.protocol = X86_HDC_PROTOCOL_ATA_PIO; break;
    case CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB: config.protocol = X86_HDC_PROTOCOL_COMPAQ_WD_40MB; break;
    case CORE_MACHINE_HDC_PROTOCOL_IBM_WD1003_ST506: config.protocol = X86_HDC_PROTOCOL_IBM_WD1003_ST506; break;
    case CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT: config.protocol = X86_HDC_PROTOCOL_XEBEC_XT; break;
    default: return LIB_STATUS_INVALID_ARGUMENT;
    }
    config.command_ticks = board->service.command_ticks;
    config.next_sector_ticks = board->service.next_sector_ticks;
    if (board->protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT) {
        config.xebec_geometry = hdc_geometry(board->bus.xebec.expected_media_geometry);
    } else {
        config.lba28_supported = board->bus.task_file.lba28_supported;
        rate = board->bus.task_file.clock_ticks_per_second;
        config.step_ticks[0] = (rate / 1000000u) * 35u;
        for (lib_u32 i = 1u; i < 16u; ++i) config.step_ticks[i] = (rate / 2000u) * i;
    }
    connection = (x86_hdc_connection){.query = hdc_query, .read = hdc_read, .write = hdc_write,
        .irq = hdc_irq, .drq = hdc_drq, .context = hdc};
    return x86_hdc_create(&config, &connection, &hdc->chip);
}

void core_machine_hdc_reset(core_machine_hdc *hdc)
{
    if (hdc != LIB_NULL) x86_hdc_reset(hdc->chip);
}

void core_machine_hdc_advance_at(core_machine_hdc *hdc, lib_u64 now)
{
    if (hdc != LIB_NULL) x86_hdc_advance_at(hdc->chip, now);
}

lib_status core_machine_hdc_next_due_tick(const core_machine_hdc *hdc, lib_u64 *out)
{
    return hdc == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : x86_hdc_next_due_tick(hdc->chip, out);
}

void core_machine_hdc_finalize(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    x86_hdc_destroy(hdc->chip);
    hdc->chip = LIB_NULL;
    lib_memory_set(&hdc->connect, 0, sizeof(hdc->connect));
}

const core_machine_port_provider *core_machine_hdc_port_provider(void)
{
    static const core_machine_port_provider provider = {hdc_port_read, hdc_port_write};
    return &provider;
}

const core_machine_dma_channel_provider *core_machine_hdc_dma_provider(void)
{
    static const core_machine_dma_channel_provider provider = {hdc_dma_read, hdc_dma_write, hdc_dma_terminal};
    return &provider;
}

lib_u8 core_machine_hdc_irq_pending(const core_machine_hdc *hdc)
{
    return hdc != LIB_NULL && x86_hdc_irq_pending(hdc->chip);
}
