#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86/chips/hdc/hdc_interface.h"

typedef struct fixture {
    lib_bool irq, drq;
    lib_u32 reads, writes;
    const x86_hdc_geometry *geometry;
    lib_u64 first_sector;
} fixture;

static const x86_hdc_geometry geometry = {
    .logical_sector_count = 2u, .bytes_per_sector = 512u,
    .cylinders = 1u, .heads = 1u, .sectors_per_track = 2u
};

static x86_hdc_record_result query(void *context, lib_u8 unit, x86_hdc_medium *out)
{
    fixture *f = context;
    *out = (x86_hdc_medium){.geometry = f->geometry != LIB_NULL ? *f->geometry : geometry,
        .present = unit == 0u,
        .geometry_known = LIB_TRUE};
    return out->present ? X86_HDC_RECORD_OK : X86_HDC_RECORD_ABSENT;
}

static x86_hdc_record_result read_sector(void *context, lib_u8 unit,
    lib_u64 sector, lib_u8 *data)
{
    fixture *f = context;
    if (unit != 0u || sector < f->first_sector || sector - f->first_sector >= 2u)
        return X86_HDC_RECORD_RANGE;
    ++f->reads;
    lib_memory_set(data, 0x5a, 512u);
    return X86_HDC_RECORD_OK;
}

static x86_hdc_record_result write_sector(void *context, lib_u8 unit,
    lib_u64 sector, const lib_u8 *data)
{
    fixture *f = context;
    if (unit != 0u || sector < f->first_sector || sector - f->first_sector >= 2u)
        return X86_HDC_RECORD_RANGE;
    for (lib_u32 i = 0u; i < 512u; ++i)
        if (data[i] != 0x5au) return X86_HDC_RECORD_FAILURE;
    ++f->writes;
    return X86_HDC_RECORD_OK;
}

static void irq(void *context, lib_bool asserted) { ((fixture *)context)->irq = asserted; }
static void drq(void *context, lib_bool asserted) { ((fixture *)context)->drq = asserted; }

static lib_status create(x86_hdc_protocol protocol, fixture *f, x86_hdc **out)
{
    const x86_hdc_config config = {.protocol = protocol, .lba28_supported = LIB_TRUE,
        .xebec_geometry = f->geometry != LIB_NULL ? *f->geometry : geometry};
    const x86_hdc_connection connection = {.query = query, .read = read_sector,
        .write = write_sector, .irq = irq, .drq = drq, .context = f};
    return x86_hdc_create(&config, &connection, out);
}

static lib_bool send_dcb(x86_hdc *hdc, lib_bool write)
{
    const lib_u8 dcb[] = {write ? 0x0au : 0x08u, 0u, 0u, 0u, 1u, 0u};
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u) != LIB_STATUS_OK)
        return LIB_FALSE;
    for (lib_size i = 0u; i < sizeof(dcb); ++i)
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, dcb[i]) != LIB_STATUS_OK)
            return LIB_FALSE;
    return LIB_TRUE;
}

static lib_i32 run_taskfile(x86_hdc_protocol protocol, lib_bool write)
{
    fixture f = {0};
    x86_hdc *hdc = LIB_NULL;
    x86_hdc_observation observation;
    lib_u64 due = 99u;
    lib_u32 value = 0u;
    lib_i32 failed = 0;
    if (create(protocol, &f, &hdc) != LIB_STATUS_OK) return 1;
    if (x86_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, write ? 0x30u : 0x20u) !=
            LIB_STATUS_OK || x86_hdc_next_due_tick(hdc, &due) != LIB_STATUS_OK || due != 0u)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    for (lib_u32 sector = 0u; sector < 2u; ++sector) {
        if (x86_hdc_read(hdc, X86_HDC_REGISTER_CONTROL, &value) != LIB_STATUS_OK &&
            protocol != X86_HDC_PROTOCOL_IBM_WD1003_ST506) failed = 1;
        if (x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value) != LIB_STATUS_OK ||
            (value & X86_HDC_STATUS_DRQ) == 0u ||
            x86_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK) failed = 1;
        for (lib_u32 word = 0u; word < 256u; ++word) {
            lib_status status = write ? x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0x5a5au) :
                x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
            if (status != LIB_STATUS_OK || (!write && value != 0x5a5au)) failed = 1;
        }
        if (x86_hdc_next_due_tick(hdc, &due) != LIB_STATUS_OK || due != 0u) failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        x86_hdc_advance_at(hdc, 0u);
        if (x86_hdc_capture(hdc, &observation) != LIB_STATUS_OK ||
            observation.sectors_remaining != 1u - sector || observation.elapsed_ticks != 0u ||
            x86_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK) failed = 1;
    }
    if (f.reads != (write ? 0u : 2u) || f.writes != (write ? 2u : 0u)) failed = 1;
    x86_hdc_advance_at(hdc, 100u);
    if (protocol != X86_HDC_PROTOCOL_IBM_WD1003_ST506) {
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_CONTROL, X86_HDC_DEVICE_CONTROL_SRST) !=
                LIB_STATUS_OK || x86_hdc_write(hdc, X86_HDC_REGISTER_CONTROL, 0u) !=
                LIB_STATUS_OK) failed = 1;
    }
    x86_hdc_advance_at(hdc, 99u);
    if (x86_hdc_capture(hdc, &observation) != LIB_STATUS_OK ||
        observation.elapsed_ticks != 100u || f.drq) failed = 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x20u) != LIB_STATUS_OK ||
        x86_hdc_next_due_tick(hdc, &due) != LIB_STATUS_OK || due != 100u) failed = 1;
    x86_hdc_reset(hdc);
    if (x86_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK ||
        x86_hdc_capture(hdc, &observation) != LIB_STATUS_OK ||
        observation.elapsed_ticks != 0u || f.irq) failed = 1;
    x86_hdc_destroy(hdc);
    if (f.irq || f.drq) failed = 1;
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC taskfile protocol=%u write=%u failed\n",
        (lib_u32)protocol, (lib_u32)write);
    return failed;
}

static lib_i32 run_xebec(lib_bool write, lib_bool destroy)
{
    fixture f = {0};
    x86_hdc *hdc = LIB_NULL;
    lib_u64 due = 99u;
    lib_i32 failed = 0;
    if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u) != LIB_STATUS_OK ||
        !send_dcb(hdc, write) || x86_hdc_next_due_tick(hdc, &due) != LIB_STATUS_OK || due != 0u)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (!f.drq || x86_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK) failed = 1;
    if (destroy) x86_hdc_destroy(hdc);
    else {
        x86_hdc_reset(hdc);
        if (f.drq || f.irq || x86_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK) failed = 1;
        x86_hdc_destroy(hdc);
    }
    if (f.drq || f.irq) failed = 1;
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC Xebec write=%u destroy=%u failed\n",
        (lib_u32)write, (lib_u32)destroy);
    return failed;
}

static lib_i32 run_xebec_record_bounds(lib_bool write, lib_u32 bytes, lib_u64 sectors)
{
    /* The provider accepts sector zero even for an incompatible description;
     * record-size and capacity admission belong to HDC. */
    x86_hdc_geometry empty = geometry;
    fixture f = {.geometry = &empty};
    x86_hdc *hdc = LIB_NULL;
    lib_u32 response = 0u;
    lib_i32 failed = 0;
    empty.bytes_per_sector = bytes;
    empty.logical_sector_count = sectors;
    if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u) != LIB_STATUS_OK ||
        !send_dcb(hdc, write)) failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (write) {
        /* Existing Write Data admits DMA before media completion is checked. */
        if (!f.drq) failed = 1;
        for (lib_u32 byte = 0u; byte < 512u; ++byte)
            x86_hdc_dma_write(hdc, 0x5au);
    }
    if (f.drq || !f.irq || f.reads != 0u || f.writes != 0u ||
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &response) != LIB_STATUS_OK ||
        response != 0x02u) failed = 1;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC Xebec bounds write=%u bytes=%u failed\n",
        (lib_u32)write, bytes);
    return failed;
}

static lib_i32 run_wd_commands(x86_hdc_protocol protocol)
{
    const struct {
        lib_u8 command, status, error;
    } cases[] = {
        {0x10u, X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC, 0u},
        {0x91u, X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC, 0u},
        {0x90u, X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC, X86_HDC_ERROR_DIAGNOSTIC_OK},
        {0xecu, X86_HDC_STATUS_DRDY | X86_HDC_STATUS_ERR, X86_HDC_ERROR_ABORT}
    };
    fixture f = {0};
    x86_hdc *hdc = LIB_NULL;
    x86_hdc_observation observation;
    lib_u32 value = 0u;
    lib_i32 failed = 0;
    if (create(protocol, &f, &hdc) != LIB_STATUS_OK) return 1;
    for (lib_size i = 0u; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        x86_hdc_reset(hdc);
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_CYLINDER_LOW, 0x7fu) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_CYLINDER_HIGH, 0x03u) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, cases[i].command) !=
                LIB_STATUS_OK) failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        if (!f.irq || x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value) !=
                LIB_STATUS_OK || value != cases[i].status || f.irq ||
            x86_hdc_read(hdc, X86_HDC_REGISTER_ERROR_FEATURES, &value) != LIB_STATUS_OK ||
            value != cases[i].error || x86_hdc_capture(hdc, &observation) != LIB_STATUS_OK)
            failed = 1;
        if (cases[i].command == 0x10u &&
            (observation.cylinder_low != 0u || observation.cylinder_high != 0u)) failed = 1;
    }
    /* 22h is a valid WD1003 read form, but not the Compaq personality's form. */
    x86_hdc_reset(hdc);
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x22u) != LIB_STATUS_OK)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value) != LIB_STATUS_OK ||
        value != (protocol == X86_HDC_PROTOCOL_IBM_WD1003_ST506 ?
            X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC | X86_HDC_STATUS_DRQ :
            X86_HDC_STATUS_DRDY | X86_HDC_STATUS_ERR)) failed = 1;
    if (protocol == X86_HDC_PROTOCOL_COMPAQ_WD_40MB &&
        (x86_hdc_read(hdc, X86_HDC_REGISTER_ERROR_FEATURES, &value) != LIB_STATUS_OK ||
            value != X86_HDC_ERROR_ABORT)) failed = 1;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC WD commands protocol=%u failed\n",
        (lib_u32)protocol);
    return failed;
}

static lib_i32 run_large_lba(lib_u32 sector, lib_bool write)
{
    x86_hdc_geometry large = geometry;
    fixture f = {.geometry = &large, .first_sector = sector};
    x86_hdc *hdc = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = 0;
    large.logical_sector_count = 0x10000000u;
    if (create(X86_HDC_PROTOCOL_ATA_PIO, &f, &hdc) != LIB_STATUS_OK) return 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER, sector & 0xffu) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_CYLINDER_LOW, (sector >> 8u) & 0xffu) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_CYLINDER_HIGH, (sector >> 16u) & 0xffu) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0x40u | (sector >> 24u)) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, write ? 0x30u : 0x20u) != LIB_STATUS_OK)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value) != LIB_STATUS_OK ||
        (value & X86_HDC_STATUS_DRQ) == 0u) failed = 1;
    for (lib_u32 word = 0u; word < 256u; ++word) {
        lib_status status = write ? x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0x5a5au) :
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
        if (status != LIB_STATUS_OK || (!write && value != 0x5a5au)) failed = 1;
    }
    x86_hdc_advance_at(hdc, 0u);
    if (f.reads != (write ? 0u : 1u) || f.writes != (write ? 1u : 0u) ||
        x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value) != LIB_STATUS_OK ||
        value != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC)) failed = 1;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC LBA=%x write=%u failed\n",
        sector, (lib_u32)write);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;
    for (lib_u32 protocol = X86_HDC_PROTOCOL_ATA_PIO;
        protocol <= X86_HDC_PROTOCOL_IBM_WD1003_ST506; ++protocol) {
        failed |= run_taskfile((x86_hdc_protocol)protocol, LIB_FALSE);
        failed |= run_taskfile((x86_hdc_protocol)protocol, LIB_TRUE);
    }
    for (lib_u32 write = 0u; write < 2u; ++write)
        for (lib_u32 destroy = 0u; destroy < 2u; ++destroy)
            failed |= run_xebec((lib_bool)write, (lib_bool)destroy);
    for (lib_u32 write = 0u; write < 2u; ++write) {
        failed |= run_xebec_record_bounds((lib_bool)write, 512u, 0u);
        failed |= run_xebec_record_bounds((lib_bool)write, 256u, 2u);
        failed |= run_xebec_record_bounds((lib_bool)write, 1024u, 2u);
    }
    failed |= run_wd_commands(X86_HDC_PROTOCOL_COMPAQ_WD_40MB);
    failed |= run_wd_commands(X86_HDC_PROTOCOL_IBM_WD1003_ST506);
    for (lib_u32 write = 0u; write < 2u; ++write) {
        failed |= run_large_lba(0x00800000u, (lib_bool)write);
        failed |= run_large_lba(0x0fffffffu, (lib_bool)write);
    }
    return failed;
}
