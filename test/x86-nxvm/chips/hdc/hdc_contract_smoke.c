#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86-nxvm/chips/hdc/hdc.h"

typedef struct fixture {
    lib_bool irq, drq;
    lib_u32 reads, writes;
    lib_u32 queries;
    const x86_hdc_geometry *geometry;
    lib_u64 first_sector;
    lib_u64 sector_sum;
    lib_u32 records;
    lib_u8 unit_mask;
    x86_hdc_record_result forced_read, forced_write;
    lib_u32 formats;
    x86_hdc_track formatted;
    x86_hdc_record_result forced_format;
    lib_u32 fail_format_call;
    lib_u64 first_format_sector, format_sector_sum;
    lib_bool read_only;
    lib_bool stored_records;
    x86_hdc_record saved[2];
} fixture;

static const x86_hdc_geometry geometry = {
    .logical_sector_count = 2u, .bytes_per_sector = 512u,
    .cylinders = 1u, .heads = 1u, .sectors_per_track = 2u
};

static x86_hdc_record_result query(void *context, lib_u8 unit, x86_hdc_medium *out)
{
    fixture *f = context;
    ++f->queries;
    *out = (x86_hdc_medium){.geometry = f->geometry != LIB_NULL ? *f->geometry : geometry,
        .present = unit < 2u &&
            ((f->unit_mask != 0u ? f->unit_mask : 1u) & (1u << unit)) != 0u,
        .geometry_known = LIB_TRUE, .read_only = f->read_only};
    return out->present ? X86_HDC_RECORD_OK : X86_HDC_RECORD_ABSENT;
}

static x86_hdc_record_result read_sector(void *context, lib_u8 unit,
    lib_u64 sector, x86_hdc_record *record)
{
    fixture *f = context;
    if (f->forced_read != X86_HDC_RECORD_OK) {
        record->data[0] = 0xedu;
        record->ecc_present = LIB_TRUE;
        return f->forced_read;
    }
    if (unit >= 2u ||
        ((f->unit_mask != 0u ? f->unit_mask : 1u) & (1u << unit)) == 0u ||
        sector < f->first_sector ||
        sector - f->first_sector >= (f->records != 0u ? f->records : 2u))
        return X86_HDC_RECORD_RANGE;
    ++f->reads;
    f->sector_sum += sector;
    if (f->stored_records || (sector - f->first_sector < 2u &&
            f->saved[sector - f->first_sector].ecc_present)) {
        if (sector - f->first_sector >= 2u) return X86_HDC_RECORD_RANGE;
        *record = f->saved[sector - f->first_sector];
    } else {
        lib_memory_set(record->data, 0x5a, sizeof(record->data));
        record->ecc_present = LIB_FALSE;
    }
    return X86_HDC_RECORD_OK;
}

static x86_hdc_record_result write_sector(void *context, lib_u8 unit,
    lib_u64 sector, const x86_hdc_record *record)
{
    fixture *f = context;
    if (f->forced_write != X86_HDC_RECORD_OK) return f->forced_write;
    if (unit != 0u || sector < f->first_sector || sector - f->first_sector >= 2u)
        return X86_HDC_RECORD_RANGE;
    if (f->stored_records) {
        f->saved[sector - f->first_sector] = *record;
        ++f->writes;
        return X86_HDC_RECORD_OK;
    }
    for (lib_u32 i = 0u; i < 512u; ++i)
        if (record->data[i] != 0x5au) return X86_HDC_RECORD_FAILURE;
    f->saved[sector - f->first_sector] = *record;
    ++f->writes;
    return X86_HDC_RECORD_OK;
}

static x86_hdc_record_result format_track(void *context, lib_u8 unit,
    const x86_hdc_track *track)
{
    fixture *f = context;
    ++f->formats;
    if (f->forced_format != X86_HDC_RECORD_OK &&
        (f->fail_format_call == 0u || f->formats == f->fail_format_call))
        return f->forced_format;
    if (unit != 0u) return X86_HDC_RECORD_ABSENT;
    if (f->formats == 1u) f->first_format_sector = track->first_sector;
    f->format_sector_sum += track->first_sector;
    f->formatted = *track;
    return X86_HDC_RECORD_OK;
}

static void irq(void *context, lib_bool asserted) { ((fixture *)context)->irq = asserted; }
static void drq(void *context, lib_bool asserted) { ((fixture *)context)->drq = asserted; }

static lib_status create(x86_hdc_protocol protocol, fixture *f, x86_hdc **out)
{
    const x86_hdc_config config = {.protocol = protocol, .lba28_supported = LIB_TRUE,
        .xebec_geometry = f->geometry != LIB_NULL ? *f->geometry : geometry};
    const x86_hdc_connection connection = {.query = query, .read = read_sector,
        .write = write_sector, .irq = irq, .drq = drq, .context = f,
        .format_track = format_track};
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

static lib_i32 run_xebec_dma(lib_bool write, lib_bool abort)
{
    fixture f = {0};
    x86_hdc *hdc = LIB_NULL;
    lib_u8 byte = 0xa5u;
    lib_u32 response = 99u;
    lib_i32 failed = 0;
    if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
    x86_hdc_dma_read(hdc, &byte);
    x86_hdc_terminal_count(hdc);
    if (byte != 0xa5u || f.drq || f.irq ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u) != LIB_STATUS_OK ||
        !send_dcb(hdc, write)) failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (!f.drq || f.irq) failed = 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 1u) != LIB_STATUS_OK ||
        !f.drq || f.irq || f.writes != 0u)
        failed = 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u) != LIB_STATUS_OK ||
        !f.drq || f.irq || f.writes != 0u)
        failed = 1;
    for (lib_u32 i = 0u; i < (abort ? 17u : 512u); ++i) {
        if (!f.drq) failed = 1;
        if (write) x86_hdc_dma_write(hdc, 0x5au);
        else {
            byte = 0u;
            x86_hdc_dma_read(hdc, &byte);
            if (byte != 0x5au) failed = 1;
        }
    }
    if (abort) x86_hdc_terminal_count(hdc);
    if (f.drq || !f.irq || f.reads != (write ? 0u : 1u) ||
        f.writes != (write && !abort ? 1u : 0u) ||
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &response) != LIB_STATUS_OK ||
        response != (abort ? 2u : 0u)) failed = 1;
    byte = 0xa5u;
    x86_hdc_dma_read(hdc, &byte);
    if (byte != 0xa5u) failed = 1; /* No bytes leak after response. */
    x86_hdc_destroy(hdc);
    return failed || f.drq || f.irq;
}

static lib_i32 run_xebec_record_bounds(lib_bool write, lib_u32 bytes, lib_u64 sectors,
    lib_u32 cylinders, lib_u32 heads, lib_u32 tracks)
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
    empty.cylinders = cylinders;
    empty.heads = heads;
    empty.sectors_per_track = tracks;
    if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u) != LIB_STATUS_OK ||
        !send_dcb(hdc, write)) failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    /* Incompatible records are rejected before accepting a DMA payload. */
    if (f.drq || !f.irq || f.reads != 0u || f.writes != 0u ||
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &response) != LIB_STATUS_OK ||
        response != 0x02u) failed = 1;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC Xebec bounds write=%u bytes=%u failed\n",
        (lib_u32)write, bytes);
    return failed;
}

static lib_i32 run_xebec_commands(void)
{
    static const struct {
        lib_u8 command, drive_head, sector, cylinder, count, unit_mask;
        lib_u8 response, sense;
        lib_u32 reads;
    } cases[] = {
        {0x00u, 0u, 63u, 255u, 0u, 1u, 0u, 0u, 0u},
        {0x00u, 0x20u, 0u, 0u, 0u, 1u, 0x22u, 0x04u, 0u},
        {0x00u, 0x20u, 0u, 0u, 0u, 3u, 0x20u, 0u, 0u},
        {0x01u, 0u, 63u, 255u, 0u, 1u, 0u, 0u, 0u},
        {0x01u, 0x20u, 0u, 0u, 0u, 1u, 0x22u, 0x04u, 0u},
        {0x0bu, 0u, 0u, 1u, 0u, 1u, 0u, 0u, 0u},
        {0x0bu, 0u, 1u, 0u, 0u, 1u, 2u, 0xa1u, 0u},
        {0x0bu, 2u, 0u, 0u, 0u, 1u, 2u, 0xa1u, 0u},
        {0x0bu, 0u, 0u, 2u, 0u, 1u, 2u, 0xa1u, 0u},
        {0x05u, 0u, 0u, 0u, 1u, 1u, 0u, 0u, 1u},
        {0x05u, 0u, 1u, 0u, 3u, 1u, 0u, 0u, 3u},
        {0x05u, 0x20u, 1u, 0u, 3u, 3u, 0x20u, 0u, 3u},
        {0x05u, 0u, 0u, 0u, 0u, 1u, 2u, 0xa0u, 0u},
        {0x05u, 0u, 0u, 0u, 9u, 1u, 2u, 0xa1u, 8u},
        {0xe3u, 0x1fu, 63u, 255u, 0u, 1u, 0u, 0u, 4u},
        {0xe3u, 0x3fu, 63u, 255u, 0u, 3u, 0x20u, 0u, 4u},
        {0xe3u, 0x20u, 0u, 0u, 0u, 1u, 0x22u, 0x84u, 0u},
        {0x02u, 0u, 0u, 0u, 0u, 1u, 2u, 0x20u, 0u}
    };
    const x86_hdc_geometry media = {.logical_sector_count = 8u,
        .bytes_per_sector = 512u, .cylinders = 2u, .heads = 2u,
        .sectors_per_track = 2u};
    lib_i32 failed = 0;

    for (lib_size i = 0u; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        fixture f = {.geometry = &media, .records = 8u,
            .unit_mask = cases[i].unit_mask};
        x86_hdc *hdc = LIB_NULL;
        lib_u32 value = 0u;
        lib_u64 due;
        lib_u8 dcb[6] = {cases[i].command, cases[i].drive_head,
            cases[i].sector, cases[i].cylinder, cases[i].count, 0u};

        if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u) != LIB_STATUS_OK)
            failed = 1;
        for (lib_size byte = 0u; byte < sizeof(dcb); ++byte)
            if (x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, dcb[byte]) != LIB_STATUS_OK)
                failed = 1;
        if (f.irq || f.drq || x86_hdc_next_due_tick(hdc, &due) != LIB_STATUS_OK)
            failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 1u) != LIB_STATUS_OK ||
            f.irq || !hdc->data.irq_pending || f.drq)
            failed = 1;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u) != LIB_STATUS_OK ||
            !f.irq || f.drq || f.reads != cases[i].reads)
            failed = 1;
        if (!f.irq || f.drq || f.reads != cases[i].reads || f.writes != 0u ||
            x86_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK ||
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
            value != cases[i].response || f.irq) failed = 1;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 1u) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u) != LIB_STATUS_OK ||
            f.irq || hdc->data.irq_pending) failed = 1;
        if (cases[i].command == 0xe3u && cases[i].reads != 0u &&
            f.sector_sum != 12u) failed = 1;
        dcb[0] = 0x03u;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u) != LIB_STATUS_OK)
            failed = 1;
        for (lib_size byte = 0u; byte < sizeof(dcb); ++byte)
            if (x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, dcb[byte]) != LIB_STATUS_OK)
                failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        if (f.irq || f.drq) failed = 1;
        for (lib_size byte = 0u; byte < 4u; ++byte) {
            lib_u8 expected = 0u;
            if (byte == 0u) expected = cases[i].sense;
            else if (cases[i].sense != 0u && cases[i].command != 0x02u) {
                if (byte == 1u) expected = cases[i].drive_head;
                if (byte == 2u) expected = cases[i].sector;
                if (byte == 3u) expected = cases[i].cylinder;
                if (cases[i].count == 9u) expected = byte == 3u ? 2u : 0u;
                if (cases[i].command == 0xe3u)
                    expected = byte == 1u ? cases[i].drive_head & 0x20u : 0u;
            }
            if (x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK)
                failed = 1;
            if (value != expected) failed = 1;
        }
        if (!f.irq) failed = 1;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 1u) != LIB_STATUS_OK ||
            f.irq || !hdc->data.irq_pending) failed = 1;
        if (x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
            value != (cases[i].drive_head & 0x20u) || f.irq) failed = 1;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u) != LIB_STATUS_OK ||
            f.irq || hdc->data.irq_pending) failed = 1;
        x86_hdc_destroy(hdc);
    }
    return failed;
}

static lib_i32 run_xebec_initialize(void)
{
    const x86_hdc_geometry media = {.logical_sector_count = 1200u,
        .bytes_per_sector = 512u, .cylinders = 300u, .heads = 2u,
        .sectors_per_track = 2u};
    const lib_u8 parameters[] = {1u, 1u, 1u, 0u, 128u, 0u, 64u, 11u};
    const lib_u8 initialize[] = {0x0cu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu};
    fixture f = {.geometry = &media, .records = 1200u, .unit_mask = 3u};
    x86_hdc *hdc = LIB_NULL;
    lib_u32 value;
    lib_u64 due;
    lib_i32 failed = 0;
    if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
    x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
    for (lib_size i = 0u; i < sizeof(initialize); ++i)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, initialize[i]);
    for (lib_size i = 0u; i < sizeof(parameters) - 1u; ++i)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, parameters[i]);
    failed |= f.irq || f.drq || hdc->xebec.cylinders != 0u ||
        x86_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK;
    x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, parameters[7]);
    failed |= hdc->xebec.cylinders != 0u || f.irq;
    x86_hdc_advance_at(hdc, 0u);
    failed |= hdc->xebec.cylinders != 257u || hdc->xebec.heads != 1u ||
        f.queries != 0u || f.drq || !f.irq ||
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK || value != 0u;
    for (lib_u8 unit = 0u; unit < 2u; ++unit) {
        const lib_u8 verify[] = {0x05u, (lib_u8)(unit << 5u), 1u, 255u, 3u, 0u};
        const lib_u32 reads = f.reads;
        const lib_u64 sum = f.sector_sum;
        x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
        for (lib_size i = 0u; i < sizeof(verify); ++i)
            x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, verify[i]);
        x86_hdc_advance_at(hdc, 0u);
        /* Head limit skips physical head 1 without rewriting the disk layout. */
        failed |= f.reads != reads + 3u || f.sector_sum != sum + 3070u ||
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
            value != (lib_u32)(unit << 5u);
    }
    {
        const lib_u8 verify[] = {0x05u, 0u, 0x41u, 0u, 2u, 0u};
        const lib_u32 reads = f.reads;
        x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
        for (lib_size i = 0u; i < sizeof(verify); ++i)
            x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, verify[i]);
        x86_hdc_advance_at(hdc, 0u);
        failed |= f.reads != reads + 1u || hdc->xebec.last_sense[0] != 0xa1u ||
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK || value != 2u;
    }
    {
        static const lib_u8 invalid[][3] = {
            {0u, 0u, 1u}, {4u, 1u, 1u}, {1u, 1u, 0u}, {1u, 1u, 33u}
        };
        for (lib_size row = 0u; row < sizeof(invalid) / sizeof(invalid[0]); ++row) {
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
            for (lib_size i = 0u; i < sizeof(initialize); ++i)
                x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, initialize[i]);
            for (lib_size i = 0u; i < sizeof(parameters); ++i)
                x86_hdc_write(hdc, X86_HDC_REGISTER_DATA,
                    i < 3u ? invalid[row][i] : parameters[i]);
            x86_hdc_advance_at(hdc, 0u);
            failed |= hdc->xebec.cylinders != 257u || hdc->xebec.heads != 1u ||
                hdc->xebec.last_sense[0] != 0x20u ||
                x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK || value != 2u;
        }
    }
    {
        const lib_u8 limits[] = {0u, 5u, 11u, 12u};
        for (lib_size row = 0u; row < sizeof(limits); ++row) {
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
            for (lib_size i = 0u; i < sizeof(initialize); ++i)
                x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, initialize[i]);
            for (lib_size i = 0u; i < sizeof(parameters); ++i)
                x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, i == 7u ? limits[row] : parameters[i]);
            x86_hdc_advance_at(hdc, 0u);
            failed |= hdc->xebec.ecc_limit != (limits[row] > 11u ? 11u : limits[row]) ||
                hdc->xebec.cylinders != 257u || hdc->xebec.heads != 1u ||
                x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
                value != (limits[row] > 11u ? 2u : 0u);
        }
    }
    x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
    for (lib_size i = 0u; i < sizeof(initialize); ++i)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, initialize[i]);
    x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0u);
    x86_hdc_reset(hdc);
    failed |= hdc->xebec.cylinders != 0u || hdc->xebec.heads != 0u ||
        hdc->xebec.initialize_count != 0u || f.drq || f.irq;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "Xebec initialize characteristics failed\n");
    return failed;
}

static lib_i32 run_xebec_failure(lib_u8 command, x86_hdc_record_result error,
    lib_u8 sense, lib_bool prefix)
{
    const x86_hdc_geometry diagnostic = {.logical_sector_count = 8u,
        .bytes_per_sector = 512u, .cylinders = 2u, .heads = 2u,
        .sectors_per_track = 2u};
    fixture f = {0};
    x86_hdc *hdc = LIB_NULL;
    lib_u32 value;
    lib_i32 failed = 0;
    lib_u8 byte;
    lib_u8 dcb[6] = {command, 0u, 0u, 0u, prefix ? 2u : 1u, 0u};

    if ((command == 0x05u || command == 0xe3u) && prefix) f.records = 1u;
    else if (!prefix && command != 0x0au) f.forced_read = error;
    if (command == 0xe3u && prefix) f.geometry = &diagnostic;
    if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u) != LIB_STATUS_OK)
        failed = 1;
    for (lib_size i = 0u; i < sizeof(dcb); ++i)
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, dcb[i]) != LIB_STATUS_OK)
            failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (command == 0x08u && prefix) {
        f.forced_read = error;
        for (lib_u32 i = 0u; i < 512u; ++i) {
            byte = 0u;
            x86_hdc_dma_read(hdc, &byte);
            if (byte != 0x5au) failed = 1;
        }
    } else if (command == 0x0au) {
        if (prefix) {
            for (lib_u32 i = 0u; i < 512u; ++i) x86_hdc_dma_write(hdc, 0x5au);
        }
        f.forced_write = error;
        for (lib_u32 i = 0u; i < 512u; ++i) x86_hdc_dma_write(hdc, 0x5au);
    }
    if (!f.irq || f.drq || f.reads != (prefix && command != 0x0au ? 1u : 0u) ||
        f.writes != (prefix && command == 0x0au ? 1u : 0u) ||
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
        value != 2u) failed = 1;
    if (command == 0x08u && hdc->data.record.data[0] != (prefix ? 0x5au : 0u))
        failed = 1;
    dcb[0] = 0x03u;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u) != LIB_STATUS_OK)
        failed = 1;
    for (lib_size i = 0u; i < sizeof(dcb); ++i)
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, dcb[i]) != LIB_STATUS_OK)
            failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    for (lib_u32 i = 0u; i < 5u; ++i) {
        lib_u8 expected = i == 0u ? (sense | 0x80u) :
            (i == 2u && prefix ? 1u : 0u);
        if (command == 0xe3u && prefix && i != 0u)
            expected = i == 1u ? 1u : 0u;
        if (x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
            value != expected) failed = 1;
    }
    x86_hdc_destroy(hdc);
    return failed;
}

static lib_i32 run_xebec_buffer(lib_bool dma)
{
    fixture f = {.unit_mask = 2u}; /* Buffer commands need no attached drive. */
    x86_hdc *hdc = LIB_NULL;
    lib_i32 failed = 0;
    lib_u32 value = 99u;
    lib_u8 byte;
    lib_u8 dcb[6] = {0x0fu, 0u, 255u, 255u, 0u, 255u};

    if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, dma ? 3u : 2u) !=
        LIB_STATUS_OK) failed = 1;
    for (lib_u32 pass = 0u; pass < 2u; ++pass) {
        dcb[0] = pass == 0u ? 0x0fu : 0x0eu;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u) != LIB_STATUS_OK)
            failed = 1;
        for (lib_size i = 0u; i < sizeof(dcb); ++i)
            if (x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, dcb[i]) != LIB_STATUS_OK)
                failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        if (f.drq != dma || f.irq) failed = 1;
        for (lib_u32 i = 0u; i < 512u; ++i) {
            byte = (lib_u8)(i ^ (i >> 8u));
            if (pass == 0u) {
                if (dma) x86_hdc_dma_write(hdc, byte);
                else if (x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, byte) != LIB_STATUS_OK)
                    failed = 1;
            } else {
                lib_u8 received = 0u;
                if (dma) x86_hdc_dma_read(hdc, &received);
                else {
                    if (x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK)
                        failed = 1;
                    received = (lib_u8)value;
                }
                if (received != byte) failed = 1;
            }
            if (i < 511u && (f.drq != dma || f.irq)) failed = 1;
        }
        if (f.drq || !f.irq || f.reads != 0u || f.writes != 0u ||
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
            value != 0u || f.irq) failed = 1;
    }
    x86_hdc_reset(hdc);
    if (f.drq || f.irq) failed = 1;
    x86_hdc_destroy(hdc);
    return failed;
}

static lib_i32 run_xebec_track_format(void)
{
    const x86_hdc_geometry track_geometry = {.logical_sector_count = 7u,
        .bytes_per_sector = 512u, .cylinders = 1u, .heads = 1u, .sectors_per_track = 7u};
    fixture f = {.geometry = &track_geometry};
    x86_hdc *hdc = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
    for (lib_u8 command = 0x06u; command <= 0x07u; ++command) {
        for (lib_u8 interleave = 1u; interleave <= 16u; ++interleave) {
            const lib_u8 dcb[] = {command, 0u, 0u, 0u, interleave, 0u};
            lib_bool seen[7] = {0};
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u);
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
            for (lib_size index = 0u; index < sizeof(dcb); ++index)
                x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, dcb[index]);
            x86_hdc_advance_at(hdc, 0u);
            if (!f.irq || f.drq || f.reads != 0u || f.writes != 0u ||
                f.formatted.sector_count != 7u || f.formatted.first_sector != 0u ||
                f.formatted.fill_payload != (command == 0x06u) || f.formatted.fill != 0u ||
                x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
                value != 0u || f.irq) failed = 1;
            for (lib_u32 index = 0u; index < 7u; ++index) {
                const lib_u8 id = f.formatted.ids[index];
                if (id >= 7u || seen[id] || f.formatted.bad[index] != (command == 0x07u)) {
                    failed = 1;
                    break;
                }
                seen[id] = LIB_TRUE;
            }
            if (interleave == 3u) {
                const lib_u8 expected[] = {0u, 5u, 3u, 1u, 6u, 4u, 2u};
                if (lib_memory_compare(f.formatted.ids, expected, sizeof(expected)) != 0)
                    failed = 1;
            }
        }
    }
    failed |= f.formats != 32u;
    {
        const struct {
            lib_u8 interleave, sense;
            x86_hdc_record_result result;
            lib_u32 formats;
        } cases[] = {
            {0u, 0xa0u, X86_HDC_RECORD_OK, 32u},
            {17u, 0xa0u, X86_HDC_RECORD_OK, 32u},
            {1u, 0x83u, X86_HDC_RECORD_PROTECTED, 33u},
            {1u, 0xa1u, X86_HDC_RECORD_RANGE, 34u}
        };
        for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            const lib_u8 dcb[] = {0x06u, 0u, 0u, 0u, cases[index].interleave, 0u};
            const lib_u8 request[] = {0x03u, 0u, 0u, 0u, 0u, 0u};
            f.forced_format = cases[index].result;
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
            for (lib_size byte = 0u; byte < sizeof(dcb); ++byte)
                x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, dcb[byte]);
            x86_hdc_advance_at(hdc, 0u);
            if (!f.irq || f.drq || f.formats != cases[index].formats ||
                f.formatted.fill_payload || !f.formatted.bad[0] ||
                x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
                value != 2u) failed = 1;
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
            for (lib_size byte = 0u; byte < sizeof(request); ++byte)
                x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, request[byte]);
            x86_hdc_advance_at(hdc, 0u);
            for (lib_size byte = 0u; byte < 5u; ++byte) {
                if (x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
                    value != (byte == 0u ? cases[index].sense : 0u)) failed = 1;
            }
        }
    }
    x86_hdc_destroy(hdc);
    return failed;
}

static lib_i32 run_xebec_drive_format(void)
{
    const x86_hdc_geometry drive = {.logical_sector_count = 28u, .bytes_per_sector = 512u,
        .cylinders = 2u, .heads = 2u, .sectors_per_track = 7u};
    const struct {
        lib_u8 head;
        lib_u32 fail_call, calls;
        lib_u64 first, last, sum;
    } cases[] = {
        {0u, 0u, 4u, 0u, 21u, 42u},
        {1u, 0u, 3u, 7u, 21u, 42u},
        {0u, 2u, 2u, 0u, 0u, 0u}
    };
    lib_i32 failed = 0;

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        fixture f = {.geometry = &drive, .fail_format_call = cases[index].fail_call,
            .forced_format = cases[index].fail_call ? X86_HDC_RECORD_FAILURE : X86_HDC_RECORD_OK};
        x86_hdc *hdc = LIB_NULL;
        lib_u32 value = 0u;
        const lib_u8 dcb[] = {0x04u, cases[index].head, 0u, 0u, 3u, 0u};

        if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
        x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 3u);
        x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
        for (lib_size byte = 0u; byte < sizeof(dcb); ++byte)
            x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, dcb[byte]);
        x86_hdc_advance_at(hdc, 0u);
        if (f.formats != cases[index].calls || f.first_format_sector != cases[index].first ||
            f.formatted.first_sector != cases[index].last || f.format_sector_sum != cases[index].sum ||
            !f.formatted.fill_payload || f.queries != cases[index].calls || f.drq || !f.irq ||
            f.reads != 0u || f.writes != 0u ||
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
            value != (cases[index].fail_call ? 2u : 0u)) failed = 1;
        x86_hdc_destroy(hdc);
    }
    return failed;
}

static lib_i32 run_mfm_track_format(x86_hdc_protocol protocol)
{
    const x86_hdc_geometry track_geometry = {.logical_sector_count = 7u,
        .bytes_per_sector = 512u, .cylinders = 1u, .heads = 1u, .sectors_per_track = 7u};
    fixture f = {.geometry = &track_geometry};
    x86_hdc *hdc = LIB_NULL;
    lib_u8 table[512] = {0};
    const lib_u8 ids[] = {3u, 1u, 2u, 4u, 5u, 6u, 7u};
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    if (create(protocol, &f, &hdc) != LIB_STATUS_OK) return 1;
    x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 7u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x91u);
    x86_hdc_advance_at(hdc, 0u);
    x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value);
    for (lib_u32 index = 0u; index < sizeof(ids); ++index) table[index * 2u + 1u] = ids[index];
    table[0] = 0x80u;
    x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER, 0xffu);
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x50u);
    x86_hdc_advance_at(hdc, 0u);
    if (f.formats != 0u || f.irq || f.drq || hdc->data.phase != X86_HDC_PHASE_DATA_WRITE ||
        (hdc->data.status & X86_HDC_STATUS_DRQ) == 0u) failed = 1;
    for (lib_u32 index = 0u; index < 255u; ++index)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA,
            (lib_u16)table[index * 2u] | ((lib_u16)table[index * 2u + 1u] << 8u));
    if (f.formats != 0u || hdc->data.phase != X86_HDC_PHASE_DATA_WRITE ||
        hdc->data.data_index != 510u) failed = 1;
    x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0u);
    if (f.formats != 0u || hdc->data.phase != X86_HDC_PHASE_PENDING_WRITE_SECTOR) failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (f.formats != 1u || !f.irq || f.formatted.first_sector != 0u ||
        !f.formatted.fill_payload || f.formatted.ids[0] != 2u || !f.formatted.bad[0] ||
        hdc->data.phase != X86_HDC_PHASE_IDLE || f.reads != 0u || f.writes != 0u)
        failed = 1;
    for (lib_u32 variant = 0u; variant < 4u; ++variant) {
        table[0] = variant == 0u ? 1u : 0x80u;
        table[1] = variant == 1u ? 0u : 3u;
        table[3] = variant == 2u ? 3u : 1u;
        f.forced_format = variant == 3u ? X86_HDC_RECORD_PROTECTED : X86_HDC_RECORD_OK;
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x50u);
        x86_hdc_advance_at(hdc, 0u);
        for (lib_u32 index = 0u; index < 256u; ++index)
            x86_hdc_write(hdc, X86_HDC_REGISTER_DATA,
                (lib_u16)table[index * 2u] | ((lib_u16)table[index * 2u + 1u] << 8u));
        x86_hdc_advance_at(hdc, 0u);
        if (!f.irq || hdc->data.error != X86_HDC_ERROR_ABORT ||
            f.formats != (variant == 3u ? 2u : 1u) || f.formatted.ids[0] != 2u)
            failed = 1;
        x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value);
    }
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x50u);
    x86_hdc_advance_at(hdc, 0u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0x0180u);
    x86_hdc_reset(hdc);
    x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0x0200u);
    x86_hdc_advance_at(hdc, 0u);
    if (f.formats != 2u || f.irq || f.drq || hdc->data.phase != X86_HDC_PHASE_IDLE)
        failed = 1;
    x86_hdc_destroy(hdc);
    return failed;
}

static lib_i32 run_mfm_long(x86_hdc_protocol protocol)
{
    fixture f = {.stored_records = LIB_TRUE};
    x86_hdc *hdc = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    if (create(protocol, &f, &hdc) != LIB_STATUS_OK) return 1;
    x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x91u);
    x86_hdc_advance_at(hdc, 0u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x33u);
    x86_hdc_advance_at(hdc, 0u);
    for (lib_u32 sector = 0u; sector < 2u; ++sector) {
        x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value);
        for (lib_u32 word = 0u; word < 256u; ++word)
            x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, sector == 0u ? 0x5a5au : 0xa5a5u);
        if (f.writes != sector || hdc->data.phase != X86_HDC_PHASE_DATA_WRITE ||
            hdc->data.data_index != 512u || f.irq) failed = 1;
        for (lib_u32 byte = 0u; byte < 4u; ++byte) {
            x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0x91u + byte + sector * 4u);
            if (byte < 3u && (f.writes != sector ||
                hdc->data.phase != X86_HDC_PHASE_DATA_WRITE)) failed = 1;
        }
        if (f.writes != sector || hdc->data.phase != X86_HDC_PHASE_PENDING_WRITE_SECTOR)
            failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        if (f.writes != sector + 1u || !f.saved[sector].ecc_present ||
            f.saved[sector].data[511] != (sector == 0u ? 0x5au : 0xa5u) ||
            f.saved[sector].ecc[3] != 0x94u + sector * 4u || !f.irq) failed = 1;
    }
    x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER, 1u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x23u);
    x86_hdc_advance_at(hdc, 0u);
    for (lib_u32 sector = 0u; sector < 2u; ++sector) {
        x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value);
        for (lib_u32 word = 0u; word < 256u; ++word) {
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
            if (value != (sector == 0u ? 0x5a5au : 0xa5a5u)) failed = 1;
        }
        if (hdc->data.phase != X86_HDC_PHASE_DATA_READ || hdc->data.data_index != 512u)
            failed = 1;
        for (lib_u32 byte = 0u; byte < 4u; ++byte) {
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
            if (value != 0x91u + byte + sector * 4u) failed = 1;
        }
        x86_hdc_advance_at(hdc, 0u);
    }
    if (f.reads != 2u || hdc->data.phase != X86_HDC_PHASE_IDLE ||
        hdc->data.sector_count != 0u) failed = 1;
    /* An ordinary write replaces supplied tags with newly calculated ECC. */
    x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER, 1u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 1u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x30u);
    x86_hdc_advance_at(hdc, 0u);
    for (lib_u32 word = 0u; word < 256u; ++word)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0x3c3cu);
    x86_hdc_advance_at(hdc, 0u);
    if (f.writes != 3u || !f.saved[0].ecc_present ||
        f.saved[0].ecc[0] == 0x91u) failed = 1;
    f.saved[0].data[0] = 0x77u;
    x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 1u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x22u);
    x86_hdc_advance_at(hdc, 0u);
    if (hdc->data.error != 0u || hdc->data.phase != X86_HDC_PHASE_DATA_READ ||
        hdc->data.record.data[0] != 0x77u ||
        hdc->data.record.ecc[0] != f.saved[0].ecc[0]) failed = 1;
    for (lib_u32 word = 0u; word < 256u; ++word)
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
    for (lib_u32 byte = 0u; byte < 4u; ++byte)
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
    x86_hdc_advance_at(hdc, 0u);
    /* Reset during the ECC tail must not overwrite the installed record. */
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x32u);
    x86_hdc_advance_at(hdc, 0u);
    for (lib_u32 word = 0u; word < 256u; ++word)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0x9d9du);
    for (lib_u32 byte = 0u; byte < 3u; ++byte)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0xccu);
    x86_hdc_reset(hdc);
    x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0xccu);
    x86_hdc_advance_at(hdc, 0u);
    if (f.writes != 3u || f.saved[0].data[0] != 0x77u || !f.saved[0].ecc_present ||
        f.irq || hdc->data.phase != X86_HDC_PHASE_IDLE) failed = 1;
    f.forced_write = X86_HDC_RECORD_FAILURE;
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x32u);
    x86_hdc_advance_at(hdc, 0u);
    for (lib_u32 word = 0u; word < 256u; ++word)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0xe5e5u);
    for (lib_u32 byte = 0u; byte < 4u; ++byte)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0x82u);
    x86_hdc_advance_at(hdc, 0u);
    if (f.writes != 3u || f.saved[0].data[0] != 0x77u || !f.saved[0].ecc_present ||
        hdc->data.error != X86_HDC_ERROR_ABORT || !f.irq) failed = 1;
    f.forced_read = X86_HDC_RECORD_FAILURE;
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x22u);
    x86_hdc_advance_at(hdc, 0u);
    if (hdc->data.error != X86_HDC_ERROR_ABORT ||
        hdc->data.record.data[0] != 0xe5u || hdc->data.record.ecc[0] != 0x82u)
        failed = 1;
    x86_hdc_destroy(hdc);
    return failed;
}

static lib_i32 run_xebec_long(lib_bool dma)
{
    fixture f = {.stored_records = LIB_TRUE};
    x86_hdc *hdc = LIB_NULL;
    const lib_u8 write[] = {0xe6u, 0u, 0u, 0u, 2u, 0u};
    const lib_u8 read[] = {0xe5u, 0u, 0u, 0u, 2u, 0u};
    const lib_u8 partial[] = {0xe6u, 0u, 0u, 0u, 1u, 0u};
    lib_u32 value = 0u;
    lib_u8 byte = 0u;
    lib_i32 failed = 0;

    if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
    x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, dma ? 3u : 2u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
    for (lib_size index = 0u; index < sizeof(write); ++index)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, write[index]);
    x86_hdc_advance_at(hdc, 0u);
    for (lib_u32 sector = 0u; sector < 2u; ++sector) {
        for (lib_u32 index = 0u; index < 516u; ++index) {
            byte = (lib_u8)(index < 512u ? sector + 0x5au : index - 512u + 0xa1u + sector * 4u);
            if (dma) x86_hdc_dma_write(hdc, byte);
            else x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, byte);
            if (index < 515u && (f.writes != sector || f.drq != dma || f.irq)) failed = 1;
        }
        if (f.writes != sector + 1u || !f.saved[sector].ecc_present ||
            f.saved[sector].ecc[3] != 0xa4u + sector * 4u) failed = 1;
    }
    if (f.drq || !f.irq || x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
        value != 0u) failed = 1;
    x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
    for (lib_size index = 0u; index < sizeof(read); ++index)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, read[index]);
    x86_hdc_advance_at(hdc, 0u);
    for (lib_u32 sector = 0u; sector < 2u; ++sector) {
        for (lib_u32 index = 0u; index < 516u; ++index) {
            byte = 0u;
            if (dma) x86_hdc_dma_read(hdc, &byte);
            else {
                x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
                byte = (lib_u8)value;
            }
            if (byte != (lib_u8)(index < 512u ? sector + 0x5au :
                index - 512u + 0xa1u + sector * 4u)) failed = 1;
            if (index < 515u && (f.drq != dma || f.irq)) failed = 1;
        }
    }
    x86_hdc_terminal_count(hdc);
    if (f.drq || !f.irq || f.reads != 2u ||
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK || value != 0u)
        failed = 1;
    x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
    for (lib_size index = 0u; index < sizeof(partial); ++index)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, partial[index]);
    x86_hdc_advance_at(hdc, 0u);
    for (lib_u32 index = 0u; index < 515u; ++index) {
        if (dma) x86_hdc_dma_write(hdc, 0xccu);
        else x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0xccu);
    }
    x86_hdc_terminal_count(hdc);
    if (f.writes != 2u || f.saved[0].data[0] != 0x5au || f.saved[0].ecc[0] != 0xa1u ||
        f.drq || !f.irq ||
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK || value != 2u)
        failed = 1;
    f.saved[0].ecc_present = LIB_FALSE;
    x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
    for (lib_size index = 0u; index < sizeof(read); ++index)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, read[index]);
    x86_hdc_advance_at(hdc, 0u);
    if (f.drq != dma || f.irq || hdc->xebec.phase != X86_XEBEC_PHASE_DMA_READ)
        failed = 1;
    for (lib_u32 sector = 0u; sector < 2u; ++sector) {
        for (lib_u32 index = 0u; index < 516u; ++index) {
            if (dma) x86_hdc_dma_read(hdc, &byte);
            else x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
        }
    }
    if (f.drq || !f.irq || x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) !=
            LIB_STATUS_OK || value != 0u) failed = 1;
    x86_hdc_destroy(hdc);
    return failed;
}

static lib_i32 run_xebec_controller_diagnostic(lib_u8 command)
{
    fixture f = {.unit_mask = 2u};
    x86_hdc *hdc = LIB_NULL;
    lib_u32 value = 99u;
    lib_i32 failed = 0;
    const lib_u8 dcb[] = {command, 0u, 255u, 255u, 255u, 255u};
    if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
    for (lib_u8 mask = 0u; mask < 4u; ++mask) {
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, mask) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u) != LIB_STATUS_OK)
            failed = 1;
        for (lib_size byte = 0u; byte < sizeof(dcb); ++byte)
            if (x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, dcb[byte]) != LIB_STATUS_OK)
                failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        failed |= f.drq || f.irq != ((mask & 2u) != 0u) ||
            f.queries != 0u || f.reads != 0u || f.writes != 0u;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, mask | 2u) != LIB_STATUS_OK ||
            !f.irq || f.drq ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, mask & 1u) != LIB_STATUS_OK ||
            f.irq)
            failed = 1;
        failed |=
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
            value != 0u || f.irq;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, mask | 2u) != LIB_STATUS_OK ||
            f.irq || hdc->data.irq_pending)
            failed = 1;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, mask & 1u) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u) != LIB_STATUS_OK)
            failed = 1;
        for (lib_size byte = 0u; byte < sizeof(dcb); ++byte)
            if (x86_hdc_write(hdc, X86_HDC_REGISTER_DATA,
                    byte == 0u ? 0x03u : 0u) != LIB_STATUS_OK) failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        for (lib_size byte = 0u; byte < 4u; ++byte)
            if (x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
                value != 0u || f.irq) failed = 1;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, mask | 2u) != LIB_STATUS_OK ||
            !f.irq || f.drq ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, mask & 1u) != LIB_STATUS_OK ||
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
            value != 0u || f.irq ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, mask | 2u) != LIB_STATUS_OK ||
            f.irq || hdc->data.irq_pending)
            failed = 1;
        /* Read Buffer observes the logical model's final pattern, not disk data. */
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 2u) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u) != LIB_STATUS_OK)
            failed = 1;
        for (lib_size byte = 0u; byte < sizeof(dcb); ++byte)
            if (x86_hdc_write(hdc, X86_HDC_REGISTER_DATA,
                    byte == 0u ? 0x0eu : dcb[byte]) != LIB_STATUS_OK) failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        for (lib_size byte = 0u; byte < 512u; ++byte)
            if (x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
                value != 0xaau) failed = 1;
        if (!f.irq || f.drq ||
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
            value != 0u || f.irq) failed = 1;
    }
    x86_hdc_destroy(hdc);
    return failed;
}

static lib_i32 run_deadline_boundary(x86_hdc_protocol protocol, lib_bool transfer,
    lib_bool write)
{
    fixture f = {0};
    x86_hdc *hdc = LIB_NULL;
    x86_hdc_observation observation;
    const x86_hdc_config config = {.protocol = protocol,
        .command_ticks = transfer ? 0u : 5u, .next_sector_ticks = 5u,
        .xebec_geometry = geometry};
    const x86_hdc_connection connection = {.query = query, .read = read_sector,
        .write = write_sector, .irq = irq, .drq = drq, .context = &f};
    lib_u64 due = 0u;
    lib_i32 failed = 0;
    if (x86_hdc_create(&config, &connection, &hdc) != LIB_STATUS_OK) return 1;
    x86_hdc_advance_at(hdc, LIB_UINT64_MAX - 2u);
    x86_hdc_advance_at(hdc, 1u);
    if (x86_hdc_capture(hdc, &observation) != LIB_STATUS_OK ||
        observation.elapsed_ticks != LIB_UINT64_MAX - 2u) failed = 1;
    if (protocol == X86_HDC_PROTOCOL_XEBEC_XT) {
        const lib_u8 dcb[] = {0xe0u, 0u, 0u, 0u, 0u, 0u};
        x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, 2u);
        x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
        for (lib_size byte = 0u; byte < sizeof(dcb); ++byte)
            x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, dcb[byte]);
    } else {
        x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u);
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND,
            transfer ? (write ? 0x30u : 0x20u) : 0x90u);
    }
    if (transfer) {
        lib_u32 value = 0u;
        x86_hdc_advance_at(hdc, LIB_UINT64_MAX - 2u);
        x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value);
        for (lib_u32 word = 0u; word < 256u; ++word) {
            if (write) x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, 0x5a5au);
            else {
                x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
                if (value != 0x5a5au) failed = 1;
            }
        }
    }
    if (x86_hdc_next_due_tick(hdc, &due) != LIB_STATUS_OK || due != LIB_UINT64_MAX)
        failed = 1;
    x86_hdc_advance_at(hdc, LIB_UINT64_MAX - 1u);
    if (f.irq) failed = 1;
    x86_hdc_advance_at(hdc, LIB_UINT64_MAX);
    if (!f.irq || x86_hdc_next_due_tick(hdc, &due) != LIB_STATUS_INVALID_STATE ||
        (transfer && (f.reads != (write ? 0u : 2u) || f.writes != (write ? 1u : 0u))))
        failed = 1;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC deadline protocol=%u failed\n", (lib_u32)protocol);
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
    /* A raw logical record gains calculated L2 check bytes for Long. */
    x86_hdc_reset(hdc);
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x22u) != LIB_STATUS_OK)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value) != LIB_STATUS_OK ||
        value != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC |
            X86_HDC_STATUS_DRQ)) failed = 1;
    if (x86_hdc_read(hdc, X86_HDC_REGISTER_ERROR_FEATURES, &value) != LIB_STATUS_OK ||
        value != 0u) failed = 1;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC WD commands protocol=%u failed\n",
        (lib_u32)protocol);
    return failed;
}

static lib_i32 run_wd_head_gate(x86_hdc_protocol protocol)
{
    const x86_hdc_geometry heads = {
        .logical_sector_count = 32u, .bytes_per_sector = 512u,
        .cylinders = 1u, .heads = 16u, .sectors_per_track = 2u
    };
    const struct {
        lib_u8 head, control, count;
        lib_u64 first_sector;
    } cases[] = {
        {0u, 8u, 1u, 0u},
        {8u, 0u, 1u, 0u},
        {8u, 8u, 1u, 16u},
        {7u, 8u, 2u, 15u}
    };
    lib_i32 failed = 0;
    for (lib_size i = 0u; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        fixture f = {.geometry = &heads, .first_sector = cases[i].first_sector};
        x86_hdc *hdc = LIB_NULL;
        x86_hdc_observation observation;
        lib_u32 value = 0u;
        if (create(protocol, &f, &hdc) != LIB_STATUS_OK) return 1;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0x2fu) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x91u) != LIB_STATUS_OK)
            failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_CONTROL, cases[i].control) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, cases[i].count) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER,
                cases[i].count == 2u ? 2u : 1u) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0x20u | cases[i].head) !=
                LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x20u) != LIB_STATUS_OK)
            failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        for (lib_u32 sector = 0u; sector < cases[i].count; ++sector) {
            if (x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value) != LIB_STATUS_OK ||
                (value & X86_HDC_STATUS_DRQ) == 0u) failed = 1;
            for (lib_u32 word = 0u; word < 256u; ++word)
                if (x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK ||
                    value != 0x5a5au) failed = 1;
            x86_hdc_advance_at(hdc, 0u);
        }
        if (f.reads != cases[i].count ||
            x86_hdc_capture(hdc, &observation) != LIB_STATUS_OK ||
            observation.sectors_remaining != 0u ||
            observation.fixed_disk_register != cases[i].control ||
            (observation.status & (X86_HDC_STATUS_BSY | X86_HDC_STATUS_DRQ |
                X86_HDC_STATUS_ERR)) != 0u) failed = 1;
        x86_hdc_destroy(hdc);
    }
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC WD head gate protocol=%u failed\n",
        (lib_u32)protocol);
    return failed;
}

static lib_i32 run_compaq_drive_address(void)
{
    const struct {
        lib_u8 drive_head, control, address;
    } cases[] = {
        {0x20u, 0u, 0x7eu},
        {0x30u, 0u, 0x7du},
        {0x2au, 0u, 0x76u},
        {0x3au, 0u, 0x75u},
        {0x2au, 8u, 0x56u},
        {0x3au, 8u, 0x55u},
        {0x2fu, 8u, 0x42u},
        {0x3fu, 8u, 0x41u}
    };
    fixture f = {0};
    x86_hdc *hdc = LIB_NULL;
    lib_i32 failed = 0;
    lib_u32 address = 0u;
    if (create(X86_HDC_PROTOCOL_COMPAQ_WD_40MB, &f, &hdc) != LIB_STATUS_OK) return 1;
    for (lib_size i = 0u; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_CONTROL, cases[i].control) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, cases[i].drive_head) !=
                LIB_STATUS_OK ||
            x86_hdc_read(hdc, X86_HDC_REGISTER_DRIVE_ADDRESS, &address) != LIB_STATUS_OK ||
            address != cases[i].address) failed = 1;
    }
    if (f.reads != 0u || f.writes != 0u || f.irq || f.drq) failed = 1;
    x86_hdc_destroy(hdc);
    return failed;
}

static lib_i32 run_verify(x86_hdc_protocol protocol, lib_u8 command)
{
    const struct {
        lib_u8 count;
        lib_u32 records, reads;
        lib_u16 remaining;
        lib_bool error;
    } cases[] = {
        {1u, 2u, 1u, 0u, LIB_FALSE},
        {2u, 2u, 2u, 0u, LIB_FALSE},
        {3u, 2u, 2u, 1u, LIB_TRUE},
        {0u, 2u, 2u, 254u, LIB_TRUE},
        {0u, 256u, 256u, 0u, LIB_FALSE}
    };
    const x86_hdc_geometry medium = {
        .logical_sector_count = 256u, .bytes_per_sector = 512u,
        .cylinders = 128u, .heads = 1u, .sectors_per_track = 2u
    };
    lib_i32 failed = 0;
    for (lib_size i = 0u; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        fixture f = {.geometry = &medium, .records = cases[i].records};
        x86_hdc *hdc = LIB_NULL;
        x86_hdc_observation observation;
        lib_u32 status = 0u;
        lib_u64 due = 0u;
        if (create(protocol, &f, &hdc) != LIB_STATUS_OK) return 1;
        if (protocol != X86_HDC_PROTOCOL_ATA_PIO) {
            if (x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u) != LIB_STATUS_OK ||
                x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0x20u) != LIB_STATUS_OK ||
                x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x91u) != LIB_STATUS_OK)
                failed = 1;
            x86_hdc_advance_at(hdc, 0u);
        }
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, cases[i].count) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, command) != LIB_STATUS_OK)
            failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        if (!f.irq || f.drq || f.reads != cases[i].reads || f.writes != 0u ||
            x86_hdc_capture(hdc, &observation) != LIB_STATUS_OK ||
            observation.sectors_remaining != cases[i].remaining ||
            observation.sector_count != (lib_u8)cases[i].remaining ||
            (observation.status & (X86_HDC_STATUS_BSY | X86_HDC_STATUS_DRQ)) != 0u ||
            ((observation.status & X86_HDC_STATUS_ERR) != 0u) != cases[i].error ||
            (cases[i].error && (observation.error != X86_HDC_ERROR_ID_NOT_FOUND ||
                observation.cylinder_low != 1u || observation.sector_number != 1u)) ||
            x86_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK ||
            x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) != LIB_STATUS_OK ||
            f.irq) failed = 1;
        x86_hdc_destroy(hdc);
    }
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC verify protocol=%u command=%x failed\n",
        (lib_u32)protocol, (lib_u32)command);
    return failed;
}

static lib_i32 run_wd_parameters(x86_hdc_protocol protocol)
{
    const x86_hdc_geometry medium = {
        .logical_sector_count = 128u, .bytes_per_sector = 512u,
        .cylinders = 4u, .heads = 8u, .sectors_per_track = 4u
    };
    const struct {
        lib_u8 head, sector;
        lib_u64 record;
        lib_u8 final_cylinder, final_head;
    } cases[] = {
        {0x21u, 2u, 7u, 2u, 0u},
        {0x32u, 3u, 20u, 1u, 3u},
        {0x21u, 2u, 7u, 2u, 0u}
    };
    fixture f = {.geometry = &medium, .unit_mask = 3u};
    x86_hdc *hdc = LIB_NULL;
    lib_i32 failed = 0;
    lib_u32 status = 0u;
    if (create(protocol, &f, &hdc) != LIB_STATUS_OK) return 1;
    for (lib_u32 unit = 0u; unit < 2u; ++unit) {
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, unit == 0u ? 2u : 3u) !=
                LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, unit == 0u ? 0x21u : 0x33u) !=
                LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x91u) != LIB_STATUS_OK)
            failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        if (!f.irq || x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) !=
                LIB_STATUS_OK || (status & X86_HDC_STATUS_ERR) != 0u || f.irq)
            failed = 1;
    }
    /* Rejected parameters must leave the already initialized drive intact. */
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0x27u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 0u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x91u) != LIB_STATUS_OK)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) != LIB_STATUS_OK ||
        (status & X86_HDC_STATUS_ERR) == 0u) failed = 1;
    for (lib_size i = 0u; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        const lib_u32 reads = f.reads;
        x86_hdc_observation observation;
        f.first_sector = cases[i].record;
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, cases[i].head) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_CYLINDER_LOW, 1u) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER, cases[i].sector) !=
                LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u) != LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x40u) != LIB_STATUS_OK)
            failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        if (f.reads != reads + 2u ||
            x86_hdc_capture(hdc, &observation) != LIB_STATUS_OK ||
            (observation.status & (X86_HDC_STATUS_ERR | X86_HDC_STATUS_DRQ)) != 0u ||
            observation.cylinder_low != cases[i].final_cylinder ||
            (observation.drive_head & 0x0fu) != cases[i].final_head ||
            observation.sector_number != 1u || observation.sector_count != 0u)
            failed = 1;
    }
    x86_hdc_reset(hdc);
    f.first_sector = protocol == X86_HDC_PROTOCOL_COMPAQ_WD_40MB ? 67u : 15u;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0x23u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER,
            protocol == X86_HDC_PROTOCOL_COMPAQ_WD_40MB ? 17u : 4u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x40u) != LIB_STATUS_OK)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) != LIB_STATUS_OK ||
        (status & X86_HDC_STATUS_ERR) != 0u) failed = 1;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC parameters protocol=%u failed\n",
        (lib_u32)protocol);
    return failed;
}

static lib_i32 run_ata_recalibrate(void)
{
    fixture f = {.unit_mask = 3u};
    x86_hdc *hdc = LIB_NULL;
    x86_hdc_observation observation;
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    if (create(X86_HDC_PROTOCOL_ATA_PIO, &f, &hdc) != LIB_STATUS_OK) return 1;
    for (lib_u32 unit = 0u; unit < 2u; ++unit) {
        for (lib_u32 lba = 0u; lba < 2u; ++lba) {
            const lib_u8 drive = (lib_u8)(0xa5u | (unit << 4u) | (lba << 6u));
            if (x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, drive) != LIB_STATUS_OK ||
                x86_hdc_write(hdc, X86_HDC_REGISTER_CYLINDER_LOW, 0x7fu) != LIB_STATUS_OK ||
                x86_hdc_write(hdc, X86_HDC_REGISTER_CYLINDER_HIGH, 0x03u) != LIB_STATUS_OK ||
                x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER, 7u) != LIB_STATUS_OK ||
                x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, unit ? 0x1fu : 0x10u) != LIB_STATUS_OK)
                failed = 1;
            x86_hdc_advance_at(hdc, 0u);
            if (!f.irq || x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value) != LIB_STATUS_OK ||
                value != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC) || f.irq ||
                x86_hdc_capture(hdc, &observation) != LIB_STATUS_OK ||
                observation.cylinder_low != 0u || observation.cylinder_high != 0u ||
                observation.sector_number != (lba ? 0u : 1u) ||
                observation.drive_head != (drive & 0xf0u)) failed = 1;
        }
    }
    f.unit_mask = 1u;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0xb0u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x10u) != LIB_STATUS_OK) failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (x86_hdc_read(hdc, X86_HDC_REGISTER_ERROR_FEATURES, &value) != LIB_STATUS_OK ||
        value != X86_HDC_ERROR_ABORT || f.reads != 0u || f.writes != 0u) failed = 1;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC ATA recalibrate failed\n");
    return failed;
}

static lib_i32 run_ata_parameters(void)
{
    const x86_hdc_geometry medium = {
        .logical_sector_count = 128u, .bytes_per_sector = 512u,
        .cylinders = 4u, .heads = 8u, .sectors_per_track = 4u
    };
    fixture f = {.geometry = &medium, .first_sector = 127u};
    x86_hdc *hdc = LIB_NULL;
    lib_u16 identify[256] = {0};
    lib_u32 value = 0u;
    lib_i32 failed = 0;
    if (create(X86_HDC_PROTOCOL_ATA_PIO, &f, &hdc) != LIB_STATUS_OK) return 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0x21u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x91u) != LIB_STATUS_OK)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (!f.irq || x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value) != LIB_STATUS_OK ||
        (value & (X86_HDC_STATUS_ERR | X86_HDC_STATUS_DRQ)) != 0u) failed = 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0xecu) != LIB_STATUS_OK)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    for (lib_size i = 0u; i < 256u; ++i) {
        if (x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value) != LIB_STATUS_OK) failed = 1;
        identify[i] = (lib_u16)value;
    }
    x86_hdc_advance_at(hdc, 0u);
    if (identify[1] != 4u || identify[3] != 8u || identify[6] != 4u ||
        identify[53] != 1u || identify[54] != 32u || identify[55] != 2u ||
        identify[56] != 2u || identify[57] != 128u || identify[58] != 0u ||
        identify[60] != 128u || identify[61] != 0u) failed = 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_CYLINDER_LOW, 31u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER, 2u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 1u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x40u) != LIB_STATUS_OK)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (f.reads != 1u || x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value) !=
        LIB_STATUS_OK || (value & X86_HDC_STATUS_ERR) != 0u) failed = 1;
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 0u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x91u) != LIB_STATUS_OK)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (x86_hdc_read(hdc, X86_HDC_REGISTER_ERROR_FEATURES, &value) != LIB_STATUS_OK ||
        value != X86_HDC_ERROR_ABORT) failed = 1;
    for (lib_u32 lba = 0u; lba < 2u; ++lba) {
        if (x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, lba == 0u ? 0x21u : 0x40u) !=
                LIB_STATUS_OK ||
            x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x40u) != LIB_STATUS_OK)
            failed = 1;
        x86_hdc_advance_at(hdc, 0u);
        if (x86_hdc_read(hdc, X86_HDC_REGISTER_ERROR_FEATURES, &value) != LIB_STATUS_OK ||
            value != X86_HDC_ERROR_ID_NOT_FOUND || f.reads != 1u) failed = 1;
    }
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0x27u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 4u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x91u) != LIB_STATUS_OK)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (x86_hdc_write(hdc, X86_HDC_REGISTER_CYLINDER_LOW, 3u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER, 4u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 1u) != LIB_STATUS_OK ||
        x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x40u) != LIB_STATUS_OK)
        failed = 1;
    x86_hdc_advance_at(hdc, 0u);
    if (f.reads != 2u || x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value) !=
        LIB_STATUS_OK || (value & X86_HDC_STATUS_ERR) != 0u) failed = 1;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC ATA parameters failed\n");
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

static lib_i32 run_mfm_ecc(x86_hdc_protocol protocol)
{
    fixture f = {.stored_records = LIB_TRUE};
    x86_hdc *hdc = LIB_NULL;
    lib_u32 value;
    lib_i32 failed = 0;
    const lib_u32 encoded = protocol == X86_HDC_PROTOCOL_IBM_WD1003_ST506 ?
        0x0830d99du : 0x64fff4d8u;
    for (lib_u32 sector = 0u; sector < 2u; ++sector) {
        lib_memory_set(f.saved[sector].data, 0xff, sizeof(f.saved[sector].data));
        for (lib_u32 byte = 0u; byte < 4u; ++byte)
            f.saved[sector].ecc[byte] = (lib_u8)(encoded >> (24u - byte * 8u));
        f.saved[sector].ecc_present = LIB_TRUE;
    }
    f.saved[0].data[0] ^= 1u;
    f.saved[1].data[0] ^= protocol == X86_HDC_PROTOCOL_IBM_WD1003_ST506 ? 0x3fu : 0x0fu;
    if (protocol == X86_HDC_PROTOCOL_COMPAQ_WD_40MB) f.saved[1].data[1] ^= 0xffu;
    if (create(protocol, &f, &hdc) != LIB_STATUS_OK) return 1;
    x86_hdc_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0x80u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x20u);
    x86_hdc_advance_at(hdc, 0u);
    failed |= hdc->data.phase != X86_HDC_PHASE_DATA_READ ||
        (hdc->data.status & 0x04u) == 0u || f.reads != 1u || f.writes != 0u;
    x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value);
    for (lib_u32 word = 0u; word < 256u; ++word) {
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
        failed |= value != 0xffffu;
    }
    x86_hdc_advance_at(hdc, 0u);
    failed |= f.saved[0].data[0] != 0xfeu || f.reads != 2u ||
        hdc->data.error != 0x40u || hdc->data.sector_count != 1u ||
        hdc->data.sector_number != 2u || !f.irq;
    if (protocol == X86_HDC_PROTOCOL_COMPAQ_WD_40MB) {
        failed |= hdc->data.phase != X86_HDC_PHASE_DATA_READ;
        x86_hdc_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value);
        for (lib_u32 word = 0u; word < 256u; ++word) {
            x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
            failed |= value != ((lib_u32)f.saved[1].data[word * 2u] |
                ((lib_u32)f.saved[1].data[word * 2u + 1u] << 8u));
        }
        x86_hdc_advance_at(hdc, 0u);
        failed |= f.irq || hdc->data.sector_count != 1u ||
            hdc->data.sector_number != 2u;
    }
    failed |= hdc->data.phase != X86_HDC_PHASE_IDLE ||
        (hdc->data.status & X86_HDC_STATUS_ERR) == 0u || f.writes != 0u;
    x86_hdc_reset(hdc);
    x86_hdc_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 1u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x20u);
    x86_hdc_advance_at(hdc, 0u);
    failed |= hdc->data.error != 0x40u || (hdc->data.status & 0x04u) != 0u;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC ECC protocol=%u failed\n", (lib_u32)protocol);
    return failed;
}

static lib_i32 run_xebec_ecc(lib_bool dma)
{
    fixture f = {.stored_records = LIB_TRUE};
    x86_hdc *hdc = LIB_NULL;
    const lib_u8 read[] = {0x08u, 0u, 0u, 0u, 2u, 0u};
    const lib_u8 query_length[] = {0x0du, 0u, 0u, 0u, 0u, 0u};
    lib_u32 value = 0u;
    lib_u8 byte;
    lib_i32 failed = 0;
    lib_memory_set(f.saved[0].data, 0xff, sizeof(f.saved[0].data));
    lib_memory_copy(f.saved[0].ecc, (const lib_u8[4]){0x64u, 0xffu, 0xf4u, 0xd8u}, 4u);
    f.saved[0].ecc_present = LIB_TRUE;
    f.saved[0].data[10] ^= 1u;
    if (create(X86_HDC_PROTOCOL_XEBEC_XT, &f, &hdc) != LIB_STATUS_OK) return 1;
    x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_MASK, dma ? 3u : 2u);
    x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
    for (lib_size index = 0u; index < sizeof(read); ++index)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, read[index]);
    x86_hdc_advance_at(hdc, 0u);
    failed |= f.irq || f.drq != dma || f.reads != 1u;
    for (lib_u32 index = 0u; index < 512u; ++index) {
        if (dma) x86_hdc_dma_read(hdc, &byte);
        else { x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value); byte = (lib_u8)value; }
        failed |= byte != 0xffu;
    }
    failed |= f.drq || !f.irq || f.reads != 1u || f.writes != 0u ||
        f.saved[0].data[10] != 0xfeu || hdc->xebec.last_sense[0] != 0x98u;
    x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
    failed |= value != 2u || f.irq;
    x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
    for (lib_size index = 0u; index < sizeof(query_length); ++index)
        x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, query_length[index]);
    x86_hdc_advance_at(hdc, 0u);
    x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
    failed |= value != 1u;
    x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
    failed |= value != 0u || hdc->xebec.phase != X86_XEBEC_PHASE_IDLE;
    {
        const lib_u8 initialize[] = {0x0cu, 0u, 0u, 0u, 0u, 0u};
        const lib_u8 parameters[] = {0u, 1u, 1u, 0u, 0u, 0u, 0u, 0u};
        x86_hdc_write(hdc, X86_HDC_REGISTER_XEBEC_SELECT, 0u);
        for (lib_size index = 0u; index < sizeof(initialize); ++index)
            x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, initialize[index]);
        for (lib_size index = 0u; index < sizeof(parameters); ++index)
            x86_hdc_write(hdc, X86_HDC_REGISTER_DATA, parameters[index]);
        x86_hdc_advance_at(hdc, 0u);
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
        failed |= value != 0u || hdc->xebec.ecc_limit != 0u;
        failed |= !send_dcb(hdc, LIB_FALSE);
        x86_hdc_advance_at(hdc, 0u);
        failed |= f.drq || !f.irq || hdc->xebec.last_sense[0] != 0x91u;
        x86_hdc_read(hdc, X86_HDC_REGISTER_DATA, &value);
        failed |= value != 2u || hdc->xebec.corrected_burst != 1u;
    }
    x86_hdc_reset(hdc);
    failed |= hdc->xebec.corrected_burst != 0u || hdc->xebec.ecc_limit != 11u;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "Xebec ECC dma=%u failed\n", (lib_u32)dma);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;
    for (lib_u32 protocol = X86_HDC_PROTOCOL_ATA_PIO;
        protocol <= X86_HDC_PROTOCOL_XEBEC_XT; ++protocol)
        failed |= run_deadline_boundary((x86_hdc_protocol)protocol, LIB_FALSE, LIB_FALSE);
    for (lib_u32 protocol = X86_HDC_PROTOCOL_ATA_PIO;
        protocol <= X86_HDC_PROTOCOL_IBM_WD1003_ST506; ++protocol) {
        failed |= run_deadline_boundary((x86_hdc_protocol)protocol, LIB_TRUE, LIB_FALSE);
        failed |= run_deadline_boundary((x86_hdc_protocol)protocol, LIB_TRUE, LIB_TRUE);
    }
    for (lib_u32 protocol = X86_HDC_PROTOCOL_ATA_PIO;
        protocol <= X86_HDC_PROTOCOL_IBM_WD1003_ST506; ++protocol) {
        failed |= run_taskfile((x86_hdc_protocol)protocol, LIB_FALSE);
        failed |= run_taskfile((x86_hdc_protocol)protocol, LIB_TRUE);
    }
    for (lib_u32 write = 0u; write < 2u; ++write)
        for (lib_u32 destroy = 0u; destroy < 2u; ++destroy)
            failed |= run_xebec((lib_bool)write, (lib_bool)destroy);
    for (lib_u32 write = 0u; write < 2u; ++write) {
        for (lib_u32 abort = 0u; abort < 2u; ++abort)
            failed |= run_xebec_dma((lib_bool)write, (lib_bool)abort);
        failed |= run_xebec_record_bounds((lib_bool)write, 512u, 0u, 1u, 1u, 2u);
        failed |= run_xebec_record_bounds((lib_bool)write, 256u, 2u, 1u, 1u, 2u);
        failed |= run_xebec_record_bounds((lib_bool)write, 1024u, 2u, 1u, 1u, 2u);
        failed |= run_xebec_record_bounds((lib_bool)write, 512u, 2u, 1025u, 1u, 2u);
        failed |= run_xebec_record_bounds((lib_bool)write, 512u, 2u, 1u, 33u, 2u);
        failed |= run_xebec_record_bounds((lib_bool)write, 512u, 2u, 1u, 1u, 65u);
    }
    failed |= run_wd_commands(X86_HDC_PROTOCOL_COMPAQ_WD_40MB);
    failed |= run_xebec_commands();
    failed |= run_xebec_initialize();
    failed |= run_xebec_buffer(LIB_FALSE);
    failed |= run_xebec_buffer(LIB_TRUE);
    failed |= run_xebec_controller_diagnostic(0xe0u);
    failed |= run_xebec_controller_diagnostic(0xe4u);
    failed |= run_xebec_track_format();
    failed |= run_xebec_drive_format();
    failed |= run_mfm_track_format(X86_HDC_PROTOCOL_IBM_WD1003_ST506);
    failed |= run_mfm_track_format(X86_HDC_PROTOCOL_COMPAQ_WD_40MB);
    failed |= run_mfm_long(X86_HDC_PROTOCOL_IBM_WD1003_ST506);
    failed |= run_mfm_long(X86_HDC_PROTOCOL_COMPAQ_WD_40MB);
    failed |= run_xebec_long(LIB_FALSE);
    failed |= run_xebec_long(LIB_TRUE);
    for (lib_u32 i = X86_HDC_RECORD_ABSENT; i <= X86_HDC_RECORD_FAILURE; ++i) {
        lib_u8 read_sense = i == X86_HDC_RECORD_ABSENT ? 0x04u :
            (i == X86_HDC_RECORD_RANGE ? 0x21u : 0x11u);
        lib_u8 write_sense = i == X86_HDC_RECORD_ABSENT ? 0x04u :
            (i == X86_HDC_RECORD_RANGE ? 0x21u : 0x03u);
        failed |= run_xebec_failure(0x08u, (x86_hdc_record_result)i, read_sense, LIB_FALSE);
        failed |= run_xebec_failure(0x05u, (x86_hdc_record_result)i, read_sense, LIB_FALSE);
        failed |= run_xebec_failure(0xe3u, (x86_hdc_record_result)i, read_sense, LIB_FALSE);
        failed |= run_xebec_failure(0x0au, (x86_hdc_record_result)i, write_sense, LIB_FALSE);
    }
    failed |= run_xebec_failure(0x08u, X86_HDC_RECORD_RANGE, 0x21u, LIB_TRUE);
    failed |= run_xebec_failure(0x05u, X86_HDC_RECORD_RANGE, 0x21u, LIB_TRUE);
    failed |= run_xebec_failure(0xe3u, X86_HDC_RECORD_RANGE, 0x21u, LIB_TRUE);
    failed |= run_xebec_failure(0x0au, X86_HDC_RECORD_RANGE, 0x21u, LIB_TRUE);
    failed |= run_wd_commands(X86_HDC_PROTOCOL_IBM_WD1003_ST506);
    failed |= run_wd_head_gate(X86_HDC_PROTOCOL_COMPAQ_WD_40MB);
    failed |= run_wd_head_gate(X86_HDC_PROTOCOL_IBM_WD1003_ST506);
    failed |= run_verify(X86_HDC_PROTOCOL_ATA_PIO, 0x40u);
    failed |= run_verify(X86_HDC_PROTOCOL_ATA_PIO, 0x41u);
    failed |= run_verify(X86_HDC_PROTOCOL_IBM_WD1003_ST506, 0x40u);
    failed |= run_verify(X86_HDC_PROTOCOL_IBM_WD1003_ST506, 0x41u);
    failed |= run_verify(X86_HDC_PROTOCOL_COMPAQ_WD_40MB, 0x40u);
    failed |= run_verify(X86_HDC_PROTOCOL_COMPAQ_WD_40MB, 0x41u);
    failed |= run_compaq_drive_address();
    failed |= run_mfm_ecc(X86_HDC_PROTOCOL_IBM_WD1003_ST506);
    failed |= run_mfm_ecc(X86_HDC_PROTOCOL_COMPAQ_WD_40MB);
    failed |= run_xebec_ecc(LIB_FALSE);
    failed |= run_xebec_ecc(LIB_TRUE);
    failed |= run_wd_parameters(X86_HDC_PROTOCOL_IBM_WD1003_ST506);
    failed |= run_wd_parameters(X86_HDC_PROTOCOL_COMPAQ_WD_40MB);
    failed |= run_ata_recalibrate();
    failed |= run_ata_parameters();
    for (lib_u32 write = 0u; write < 2u; ++write) {
        failed |= run_large_lba(0x00800000u, (lib_bool)write);
        failed |= run_large_lba(0x0fffffffu, (lib_bool)write);
    }
    return failed;
}
