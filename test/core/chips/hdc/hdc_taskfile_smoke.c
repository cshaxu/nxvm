#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "core/chips/hdc/hdc_interface.h"

/* Original ATA command assertions, with chip-local records and registers. */
typedef struct fixture_media {
    lib_u8 sector[2][512];
    lib_u8 ecc[2][4];
    lib_bool ecc_present[2];
    lib_u32 query_count;
    lib_u32 read_count;
    lib_u32 write_count;
    lib_bool present;
    lib_bool read_only;
    x86_hdc_record_result forced_read_result;
    x86_hdc_record_result forced_write_result;
} fixture_media;

static x86_hdc_record_result fixture_query(void *context, lib_u8 unit,
    x86_hdc_medium *out_info)
{
    fixture_media *media = context;

    if (media == LIB_NULL || out_info == LIB_NULL) {
        return X86_HDC_RECORD_RANGE;
    }
    if (unit != 0u) return X86_HDC_RECORD_ABSENT;
    ++media->query_count;
    lib_memory_set(out_info, 0, sizeof(*out_info));
    out_info->present = media->present;
    out_info->geometry_known = LIB_TRUE;
    out_info->read_only = media->read_only;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 1u;
    out_info->geometry.sectors_per_track = 2u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 2u;
    return media->present ? X86_HDC_RECORD_OK : X86_HDC_RECORD_ABSENT;
}

static x86_hdc_record_result fixture_read_record(void *context, lib_u8 unit,
    lib_u64 sector, x86_hdc_record *buffer)
{
    fixture_media *media = context;

    if (media == LIB_NULL || buffer == LIB_NULL || !media->present) {
        return X86_HDC_RECORD_ABSENT;
    }
    if (media->forced_read_result != X86_HDC_RECORD_OK) {
        buffer->data[0] = 0xedu;
        buffer->ecc_present = LIB_TRUE;
        return media->forced_read_result;
    }
    if (unit != 0u || sector >= 2u) {
        return X86_HDC_RECORD_RANGE;
    }
    ++media->read_count;
    lib_memory_copy(buffer->data, media->sector[sector], sizeof(media->sector[0]));
    lib_memory_copy(buffer->ecc, media->ecc[sector], sizeof(buffer->ecc));
    buffer->ecc_present = media->ecc_present[sector];
    return X86_HDC_RECORD_OK;
}

static x86_hdc_record_result fixture_write_record(void *context, lib_u8 unit,
    lib_u64 sector, const x86_hdc_record *buffer)
{
    fixture_media *media = context;

    if (media == LIB_NULL || buffer == LIB_NULL || !media->present) {
        return X86_HDC_RECORD_ABSENT;
    }
    if (media->read_only) return X86_HDC_RECORD_PROTECTED;
    if (media->forced_write_result != X86_HDC_RECORD_OK) {
        return media->forced_write_result;
    }
    if (unit != 0u || sector >= 2u) {
        return X86_HDC_RECORD_RANGE;
    }
    ++media->write_count;
    lib_memory_copy(media->sector[sector], buffer->data, sizeof(media->sector[0]));
    lib_memory_copy(media->ecc[sector], buffer->ecc, sizeof(buffer->ecc));
    media->ecc_present[sector] = buffer->ecc_present;
    return X86_HDC_RECORD_OK;
}


static x86_hdc_observation hdc_observe(x86_hdc *hdc)
{
    x86_hdc_observation observation = {0};
    (void)x86_hdc_capture(hdc, &observation);
    return observation;
}

static void hdc_service(x86_hdc *hdc)
{
    lib_u64 due;
    if (x86_hdc_next_due_tick(hdc, &due) == LIB_STATUS_OK) x86_hdc_advance_at(hdc, due);
}

static lib_bool fixture_write(x86_hdc *hdc, x86_hdc_register reg,
    lib_u32 value)
{
    return x86_hdc_write(hdc, reg, value) == LIB_STATUS_OK;
}

static lib_bool fixture_read(x86_hdc *hdc, x86_hdc_register reg,
    lib_u32 *out_value)
{
    return x86_hdc_read(hdc, reg, out_value) == LIB_STATUS_OK;
}

static lib_bool fixture_command(x86_hdc *hdc, lib_u8 command)
{
    if (!fixture_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, command)) return LIB_FALSE;
    hdc_service(hdc);
    return LIB_TRUE;
}

static lib_bool fixture_program_chs(x86_hdc *hdc)
{
    return fixture_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 1u) &&
        fixture_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER, 1u) &&
        fixture_write(hdc, X86_HDC_REGISTER_CYLINDER_LOW, 0u) &&
        fixture_write(hdc, X86_HDC_REGISTER_CYLINDER_HIGH, 0u) &&
        fixture_write(hdc, X86_HDC_REGISTER_DRIVE_HEAD, 0u);
}

static lib_bool fixture_drain(x86_hdc *hdc, lib_u16 *first_word)
{
    lib_u32 word;

    for (lib_u32 index = 0u; index < 256u; ++index) {
        if (!fixture_read(hdc, X86_HDC_REGISTER_DATA, &word)) return LIB_FALSE;
        if (index == 0u && first_word != LIB_NULL) *first_word = (lib_u16)word;
    }
    hdc_service(hdc);
    return LIB_TRUE;
}

static lib_bool fixture_fill(x86_hdc *hdc, lib_u16 first_word)
{
    for (lib_u32 index = 0u; index < 256u; ++index) {
        if (!fixture_write(hdc, X86_HDC_REGISTER_DATA,
                index == 0u ? first_word : 0u)) return LIB_FALSE;
    }
    hdc_service(hdc);
    return LIB_TRUE;
}

static lib_i32 fixture_pio_interrupts(x86_hdc_protocol protocol, lib_bool write)
{
    const x86_hdc_config config = {.protocol = protocol, .command_ticks = 3u};
    fixture_media media = {.present = LIB_TRUE};
    const x86_hdc_connection connection = {.context = &media, .query = fixture_query,
        .read = fixture_read_record, .write = fixture_write_record};
    x86_hdc *hdc = LIB_NULL;
    lib_u32 status = 0u;
    lib_i32 failed = 0;

    if (x86_hdc_create(&config, &connection, &hdc) != LIB_STATUS_OK) return 1;
    /* IBM AT / ATA PIO notification, adopted as Compaq's explicit L2 model. */
    if (!fixture_program_chs(hdc) ||
        !fixture_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u) ||
        !fixture_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND,
            write ? 0x31u : 0x21u)) failed = 1;
    for (lib_u32 phase = 0u; phase < 2u; ++phase) {
        const x86_hdc_observation before = hdc_observe(hdc);
        const x86_hdc_register registers[] = {
            X86_HDC_REGISTER_ERROR_FEATURES, X86_HDC_REGISTER_SECTOR_COUNT,
            X86_HDC_REGISTER_SECTOR_NUMBER, X86_HDC_REGISTER_CYLINDER_LOW,
            X86_HDC_REGISTER_CYLINDER_HIGH, X86_HDC_REGISTER_DRIVE_HEAD,
            X86_HDC_REGISTER_STATUS_COMMAND};
        x86_hdc_observation after;
        failed |= before.phase != (phase == 0u ? X86_HDC_PHASE_PENDING_COMMAND :
            write ? X86_HDC_PHASE_DATA_WRITE : X86_HDC_PHASE_DATA_READ);
        for (lib_size reg = 0u; reg < sizeof(registers) / sizeof(registers[0]); ++reg)
            failed |= !fixture_write(hdc, registers[reg], 0x90u);
        after = hdc_observe(hdc);
        failed |= after.phase != before.phase || after.status != before.status ||
            after.sector_count != before.sector_count ||
            after.sector_number != before.sector_number ||
            after.cylinder_low != before.cylinder_low ||
            after.cylinder_high != before.cylinder_high ||
            after.drive_head != before.drive_head || after.data_index != before.data_index;
        if (phase == 0u) hdc_service(hdc);
    }
    if (x86_hdc_irq_pending(hdc) != !write) failed = 1;
    for (lib_u32 sector = 0u; sector < 2u; ++sector) {
        if (!fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
            (status & 0x08u) == 0u || x86_hdc_irq_pending(hdc) ||
            !(write ? fixture_fill(hdc, 0x5a5au) : fixture_drain(hdc, LIB_NULL)) ||
            x86_hdc_irq_pending(hdc) != (sector == 0u || write)) failed = 1;
    }
    if (hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE ||
        hdc_observe(hdc).sector_count != 0u ||
        media.read_count != (write ? 0u : 2u) ||
        media.write_count != (write ? 2u : 0u)) failed = 1;

    x86_hdc_reset(hdc);
    if (!fixture_write(hdc, X86_HDC_REGISTER_CONTROL, 0x02u) ||
        !fixture_program_chs(hdc) || !fixture_command(hdc, 0x20u) ||
        x86_hdc_irq_pending(hdc) ||
        !fixture_write(hdc, X86_HDC_REGISTER_CONTROL, 0u) ||
        !x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        x86_hdc_irq_pending(hdc) ||
        !fixture_write(hdc, X86_HDC_REGISTER_CONTROL, 0x04u) ||
        hdc_observe(hdc).status != 0x80u ||
        hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE ||
        !fixture_write(hdc, X86_HDC_REGISTER_CONTROL, 0u) ||
        hdc_observe(hdc).status != 0x50u || x86_hdc_irq_pending(hdc)) failed = 1;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC PIO interrupt protocol=%u write=%u failed\n",
        (lib_u32)protocol, (lib_u32)write);
    return failed;
}

static lib_i32 fixture_identify_capability(lib_bool lba)
{
    const x86_hdc_config config = {.protocol = X86_HDC_PROTOCOL_ATA_PIO,
        .lba28_supported = lba};
    fixture_media media = {.present = LIB_TRUE};
    const x86_hdc_connection connection = {.context = &media, .query = fixture_query,
        .read = fixture_read_record, .write = fixture_write_record};
    x86_hdc *hdc = LIB_NULL;
    lib_u32 value = 0u, reads;
    lib_i32 failed = 0;

    if (x86_hdc_create(&config, &connection, &hdc) != LIB_STATUS_OK) return 1;
    if (!fixture_program_chs(hdc) ||
        !fixture_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u) ||
        !fixture_command(hdc, 0x20u)) failed = 1;
    media.forced_read_result = X86_HDC_RECORD_RANGE;
    if (!fixture_drain(hdc, LIB_NULL) ||
        hdc_observe(hdc).error != 0x10u ||
        hdc_observe(hdc).sectors_remaining != 1u) failed = 1;
    reads = media.read_count;
    media.forced_read_result = X86_HDC_RECORD_OK;
    if (!fixture_command(hdc, 0xecu) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &value)) failed = 1;
    for (lib_u32 index = 0u; index < 256u; ++index) {
        if (!fixture_read(hdc, X86_HDC_REGISTER_DATA, &value) ||
            (index == 49u && value != (lba ? 0x0200u : 0u))) failed = 1;
    }
    hdc_service(hdc);
    if (hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE ||
        hdc_observe(hdc).sectors_remaining != 0u ||
        media.read_count != reads || x86_hdc_irq_pending(hdc)) failed = 1;
    x86_hdc_destroy(hdc);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC Identify LBA=%u failed\n", (lib_u32)lba);
    return failed;
}


lib_i32 main(void)
{
    const x86_hdc_config config = {.protocol = X86_HDC_PROTOCOL_ATA_PIO,
        .lba28_supported = LIB_TRUE, .command_ticks = 7u, .next_sector_ticks = 3u};
    fixture_media media = {.present = LIB_TRUE};
    const x86_hdc_connection connection = {.context = &media, .query = fixture_query,
        .read = fixture_read_record, .write = fixture_write_record};
    x86_hdc *hdc = LIB_NULL;
    lib_u32 status = 0u, error = 0u, queries_before, reads_before;
    lib_u16 word = 0u;
    lib_i32 failed = 0;
    media.sector[0][0] = 0x34u;
    media.sector[0][1] = 0x12u;
    if (x86_hdc_create(&config, &connection, &hdc) != LIB_STATUS_OK) return 1;
    if (hdc_observe(hdc).error != 0x01u || hdc_observe(hdc).sector_count != 1u ||
        hdc_observe(hdc).sector_number != 1u ||
        !fixture_write(hdc,
            X86_HDC_REGISTER_CONTROL,
            X86_HDC_DEVICE_CONTROL_NIEN) ||
        !fixture_program_chs(hdc) ||
        !fixture_command(hdc, 0x20u) ||
        !fixture_read(hdc,
            X86_HDC_REGISTER_CONTROL, &status) ||
        status != (X86_HDC_STATUS_DRDY |
            X86_HDC_STATUS_DSC | X86_HDC_STATUS_DRQ) ||
        x86_hdc_irq_pending(hdc) ||
        !fixture_write(hdc,
            X86_HDC_REGISTER_CONTROL, 0u) ||
        !x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        x86_hdc_irq_pending(hdc) ||
        !fixture_drain(hdc, &word) ||
        word != 0x1234u ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        !fixture_program_chs(hdc) ||
        !fixture_command(hdc, 0x20u) ||
        !x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND,
            &status) ||
        !fixture_drain(hdc, &word) ||
        word != 0x1234u) failed |= 0x400;

    if (!fixture_program_chs(hdc) ||
        !fixture_command(hdc, 0x20u) ||
        !fixture_drain(hdc, &word) || word != 0x1234u ||
        media.read_count != 3u) failed |= 0x08;

    if (!fixture_program_chs(hdc) ||
        !fixture_command(hdc, 0x30u) ||
        !fixture_fill(hdc, 0xa55au) ||
        media.write_count != 1u || media.sector[0][0] != 0x5au ||
        media.sector[0][1] != 0xa5u) failed |= 0x10;

    queries_before = media.query_count;
    /* Re-query the current medium on each command. */
    if (!fixture_program_chs(hdc) ||
        !fixture_command(hdc, 0x20u) ||
        !fixture_drain(hdc, &word) || word != 0xa55au ||
        media.query_count <= queries_before) failed |= 0x20;

    media.forced_read_result = X86_HDC_RECORD_RANGE;
    if (!fixture_program_chs(hdc) ||
        !fixture_command(hdc, 0x20u) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        !fixture_read(hdc, X86_HDC_REGISTER_ERROR_FEATURES, &error) ||
        status != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_ERR) ||
        error != X86_HDC_ERROR_ID_NOT_FOUND) failed |= 0x40;
    media.forced_read_result = X86_HDC_RECORD_OK;

    reads_before = media.read_count;
    media.forced_read_result = X86_HDC_RECORD_FAILURE;
    if (!fixture_program_chs(hdc) ||
        !fixture_command(hdc, 0x30u) ||
        hdc_observe(hdc).phase != X86_HDC_PHASE_DATA_WRITE ||
        !fixture_fill(hdc, 0xa55au) ||
        media.read_count != reads_before || media.write_count != 2u ||
        media.sector[0][0] != 0x5au || media.sector[0][1] != 0xa5u) {
        failed |= 0x10000;
    }
    media.forced_read_result = X86_HDC_RECORD_OK;

    media.read_only = LIB_TRUE;
    if (!fixture_program_chs(hdc) ||
        !fixture_command(hdc, 0x30u) ||
        !fixture_fill(hdc, 0xbeefu) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        !fixture_read(hdc, X86_HDC_REGISTER_ERROR_FEATURES, &error) ||
        status != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_ERR) ||
        error != X86_HDC_ERROR_ABORT) failed |= 0x80;
    media.read_only = LIB_FALSE;

    if (!fixture_program_chs(hdc) ||
        !fixture_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x20u) ||
        hdc_observe(hdc).phase != X86_HDC_PHASE_PENDING_COMMAND ||
        hdc_observe(hdc).status != X86_HDC_STATUS_BSY ||
        hdc_observe(hdc).next_service_tick != hdc_observe(hdc).elapsed_ticks + 7u ||
        x86_hdc_irq_pending(hdc) ||
        !fixture_write(hdc, X86_HDC_REGISTER_SECTOR_NUMBER, 0u) ||
        hdc_observe(hdc).sector_number != 1u) {
        failed |= 0x200;
    }
    hdc_service(hdc);
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_DATA_READ ||
        hdc_observe(hdc).sector_number != 1u ||
        hdc_observe(hdc).status != (X86_HDC_STATUS_DRDY |
            X86_HDC_STATUS_DSC | X86_HDC_STATUS_DRQ) ||
        !fixture_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x30u) ||
        hdc_observe(hdc).phase != X86_HDC_PHASE_DATA_READ ||
        hdc_observe(hdc).status != (X86_HDC_STATUS_DRDY |
            X86_HDC_STATUS_DSC | X86_HDC_STATUS_DRQ) ||
        !x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status);
    for (lib_u32 index = 0u; index < 256u; ++index) {
        failed |= !fixture_read(hdc, X86_HDC_REGISTER_DATA, &status);
        if (index == 0u) failed |= status != 0xa55au;
    }
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_PENDING_READ_SECTOR ||
        hdc_observe(hdc).status != X86_HDC_STATUS_BSY ||
        hdc_observe(hdc).next_service_tick != hdc_observe(hdc).elapsed_ticks + 3u ||
        x86_hdc_irq_pending(hdc);
    hdc_service(hdc);
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE ||
        hdc_observe(hdc).status != (X86_HDC_STATUS_DRDY |
            X86_HDC_STATUS_DSC) || x86_hdc_irq_pending(hdc);

    if (!fixture_program_chs(hdc) ||
        !fixture_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x30u)) {
        failed |= 0x800;
    }
    hdc_service(hdc);
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_DATA_WRITE ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status);
    for (lib_u32 index = 0u; index < 256u; ++index) {
        failed |= !fixture_write(hdc, X86_HDC_REGISTER_DATA,
            index == 0u ? 0x5aa5u : 0u);
    }
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_PENDING_WRITE_SECTOR ||
        hdc_observe(hdc).status != X86_HDC_STATUS_BSY ||
        hdc_observe(hdc).next_service_tick != hdc_observe(hdc).elapsed_ticks + 3u ||
        x86_hdc_irq_pending(hdc);
    hdc_service(hdc);
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE ||
        media.sector[0][0] != 0xa5u || media.sector[0][1] != 0x5au ||
        !x86_hdc_irq_pending(hdc);

    media.sector[1][0] = 0x78u;
    media.sector[1][1] = 0x56u;
    if (!fixture_program_chs(hdc) ||
        !fixture_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u) ||
        !fixture_command(hdc, 0x20u) ||
        !x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        x86_hdc_irq_pending(hdc)) failed |= 0x4000;
    for (lib_u32 index = 0u; index < 256u; ++index) {
        failed |= !fixture_read(hdc, X86_HDC_REGISTER_DATA, &status);
        if (index == 0u) failed |= status != 0x5aa5u;
    }
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_PENDING_READ_SECTOR ||
        hdc_observe(hdc).status != X86_HDC_STATUS_BSY || x86_hdc_irq_pending(hdc);
    hdc_service(hdc);
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_DATA_READ ||
        hdc_observe(hdc).sector_number != 2u || hdc_observe(hdc).sector_count != 1u ||
        !x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc, X86_HDC_REGISTER_CONTROL, &status) ||
        !x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        x86_hdc_irq_pending(hdc);
    for (lib_u32 index = 0u; index < 256u; ++index) {
        failed |= !fixture_read(hdc, X86_HDC_REGISTER_DATA, &status);
        if (index == 0u) failed |= status != 0x5678u;
    }
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_PENDING_READ_SECTOR ||
        hdc_observe(hdc).status != X86_HDC_STATUS_BSY || x86_hdc_irq_pending(hdc);
    hdc_service(hdc);
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE ||
        hdc_observe(hdc).sector_count != 0u || x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        x86_hdc_irq_pending(hdc);

    if (!fixture_program_chs(hdc) ||
        !fixture_write(hdc, X86_HDC_REGISTER_SECTOR_COUNT, 2u) ||
        !fixture_command(hdc, 0x30u) ||
        x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        x86_hdc_irq_pending(hdc)) failed |= 0x8000;
    for (lib_u32 index = 0u; index < 256u; ++index) {
        failed |= !fixture_write(hdc, X86_HDC_REGISTER_DATA,
            index == 0u ? 0x2211u : 0u);
    }
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_PENDING_WRITE_SECTOR ||
        hdc_observe(hdc).status != X86_HDC_STATUS_BSY ||
        x86_hdc_irq_pending(hdc);
    hdc_service(hdc);
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_DATA_WRITE ||
        hdc_observe(hdc).sector_number != 2u || hdc_observe(hdc).sector_count != 1u ||
        media.write_count != 4u || media.sector[0][0] != 0x11u ||
        media.sector[0][1] != 0x22u || !x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc,
            X86_HDC_REGISTER_CONTROL, &status) ||
        !x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        x86_hdc_irq_pending(hdc);
    for (lib_u32 index = 0u; index < 256u; ++index) {
        failed |= !fixture_write(hdc, X86_HDC_REGISTER_DATA,
            index == 0u ? 0x4433u : 0u);
    }
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_PENDING_WRITE_SECTOR ||
        hdc_observe(hdc).status != X86_HDC_STATUS_BSY ||
        x86_hdc_irq_pending(hdc);
    hdc_service(hdc);
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE ||
        hdc_observe(hdc).sector_count != 0u || media.write_count != 5u ||
        media.sector[1][0] != 0x33u || media.sector[1][1] != 0x44u ||
        !x86_hdc_irq_pending(hdc) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        x86_hdc_irq_pending(hdc);
    if (!fixture_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0xecu) ||
        hdc_observe(hdc).phase != X86_HDC_PHASE_PENDING_COMMAND ||
        hdc_observe(hdc).status != X86_HDC_STATUS_BSY) {
        failed |= 0x1000;
    }
    hdc_service(hdc);
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_DATA_READ ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        !fixture_read(hdc, X86_HDC_REGISTER_DATA, &status) ||
        status != 0x0040u;
    for (lib_u32 index = 1u; index < 256u; ++index) {
        failed |= !fixture_read(hdc, X86_HDC_REGISTER_DATA, &status);
    }
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_PENDING_READ_SECTOR;
    hdc_service(hdc);
    failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE ||
        x86_hdc_irq_pending(hdc);

    if (!fixture_program_chs(hdc) ||
        !fixture_write(hdc, X86_HDC_REGISTER_STATUS_COMMAND, 0x20u) ||
        hdc_observe(hdc).phase != X86_HDC_PHASE_PENDING_COMMAND ||
        !fixture_write(hdc,
            X86_HDC_REGISTER_CONTROL,
            X86_HDC_DEVICE_CONTROL_SRST) ||
        hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE ||
        hdc_observe(hdc).status != X86_HDC_STATUS_BSY ||
        !fixture_write(hdc,
            X86_HDC_REGISTER_CONTROL, 0u) ||
        hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE ||
        hdc_observe(hdc).status != (X86_HDC_STATUS_DRDY |
            X86_HDC_STATUS_DSC) || x86_hdc_irq_pending(hdc)) {
        failed |= 0x2000;
    }

    media.present = LIB_FALSE;
    if (!fixture_program_chs(hdc) ||
        !fixture_command(hdc, 0x20u) ||
        !fixture_read(hdc, X86_HDC_REGISTER_STATUS_COMMAND, &status) ||
        status != (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_ERR)) {
        failed |= 0x100;
    }
    x86_hdc_destroy(hdc);
    for (lib_u32 write = 0u; write < 2u; ++write) {
        failed |= fixture_pio_interrupts(X86_HDC_PROTOCOL_ATA_PIO, (lib_bool)write);
        failed |= fixture_pio_interrupts(X86_HDC_PROTOCOL_IBM_WD1003_ST506, (lib_bool)write);
        failed |= fixture_pio_interrupts(X86_HDC_PROTOCOL_COMPAQ_WD_40MB, (lib_bool)write);
    }
    failed |= fixture_identify_capability(LIB_FALSE);
    failed |= fixture_identify_capability(LIB_TRUE);
    if (failed) lib_c_fprintf(lib_c_stderr,
        "HDC ATA command cases failed: bits=%x status=%02x error=%02x word=%04x\n",
        failed, status, error, word);
    return failed != 0;
}
