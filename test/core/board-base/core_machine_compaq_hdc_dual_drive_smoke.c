#include "lib/types/file.h"
#include "hdc_fixture.h"
#include "lib/types/types_interface.h"
#include "core/board-base/hdc.h"
#include "core/board-base/media_interface.h"
#include "core/board-base/pic_bus_interface.h"
#include "core/x86/machine_interface.h"

static lib_u32 native_create_attempts, native_fail_at, native_live;

static lib_status test_native_create(const x86_hdc_config *config,
    const x86_hdc_connection *connection, x86_hdc **out)
{
    lib_status status;
    if (++native_create_attempts == native_fail_at) {
        *out = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    status = x86_hdc_create(config, connection, out);
    if (status == LIB_STATUS_OK) ++native_live;
    return status;
}

static void test_native_destroy(x86_hdc *chip)
{
    if (chip != LIB_NULL) {
        lib_test_assert(native_live != 0u);
        --native_live;
    }
    x86_hdc_destroy(chip);
}

/* Inject at the Board-owned construction boundary, not the chip allocator. */
#define x86_hdc_create test_native_create
#define x86_hdc_destroy test_native_destroy
#include "core/board-base/hdc.c"
#undef x86_hdc_create
#undef x86_hdc_destroy

typedef struct core_machine_compaq_hdc_media {
    lib_u8 sector[512];
    lib_u8 ecc[4];
    lib_bool ecc_present;
    lib_u32 reads;
    lib_u64 last_read_offset;
    lib_u32 formats;
    core_machine_media_track formatted;
} core_machine_compaq_hdc_media;

static core_machine_media_result core_machine_compaq_hdc_query(void *opaque,
    core_machine_media_info *out_info)
{
    if (opaque == LIB_NULL || out_info == LIB_NULL) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    lib_memory_set(out_info, 0, sizeof(*out_info));
    out_info->present = LIB_TRUE;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN |
        CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 16u;
    out_info->geometry.sectors_per_track = 17u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 272u;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_compaq_hdc_read(void *opaque,
    lib_u64 offset, void *buffer, lib_u32 byte_count)
{
    core_machine_compaq_hdc_media *media = opaque;

    if (media == LIB_NULL || buffer == LIB_NULL || byte_count != 512u ||
        offset % 512u != 0u || offset / 512u >= 272u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    ++media->reads;
    media->last_read_offset = offset;
    lib_memory_copy(buffer, media->sector, sizeof(media->sector));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_compaq_hdc_write(void *opaque,
    lib_u64 offset, const void *buffer, lib_u32 byte_count)
{
    core_machine_compaq_hdc_media *media = opaque;

    if (media == LIB_NULL || buffer == LIB_NULL || byte_count != 512u ||
        offset % 512u != 0u || offset / 512u >= 272u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    lib_memory_copy(media->sector, buffer, sizeof(media->sector));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}
static core_machine_media_result core_machine_compaq_hdc_read_record(void *opaque,
    lib_u64 sector, core_machine_media_record *record)
{
    core_machine_compaq_hdc_media *media = opaque;
    core_machine_media_result result = core_machine_compaq_hdc_read(opaque,
        sector * 512u, record->data, 512u);
    if (result == CORE_MACHINE_MEDIA_RESULT_OK) {
        lib_memory_copy(record->ecc, media->ecc, sizeof(record->ecc));
        record->ecc_present = media->ecc_present;
    }
    return result;
}

static core_machine_media_result core_machine_compaq_hdc_write_record(void *opaque,
    lib_u64 sector, const core_machine_media_record *record)
{
    core_machine_compaq_hdc_media *media = opaque;
    core_machine_media_result result = core_machine_compaq_hdc_write(opaque,
        sector * 512u, record->data, 512u);
    if (result == CORE_MACHINE_MEDIA_RESULT_OK) {
        lib_memory_copy(media->ecc, record->ecc, sizeof(media->ecc));
        media->ecc_present = record->ecc_present;
    }
    return result;
}

static core_machine_media_result core_machine_compaq_hdc_format(void *opaque,
    const core_machine_media_track *track)
{
    core_machine_compaq_hdc_media *media = opaque;
    ++media->formats;
    media->formatted = *track;
    media->ecc_present = LIB_FALSE;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static lib_status core_machine_compaq_hdc_fdc_direction(void *opaque,
    lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    (void)opaque;
    if (port != 0x03f7u || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = 0x80u;
    return LIB_STATUS_OK;
}

static lib_i32 core_machine_compaq_hdc_install(core_machine *machine, core_machine_hdc *hdc)
{
    const core_machine_port_provider *provider = core_machine_hdc_port_provider();
    core_machine_port_route routes[11] = {0};

    if (machine == LIB_NULL || hdc == LIB_NULL || provider == LIB_NULL) return 0;
    for (lib_u16 index = 0u; index < 8u; ++index) {
        routes[index] = (core_machine_port_route) {
            .address = (lib_u16)(0x01f0u + index), .read = provider->read,
            .write = provider->write, .owner = hdc};
    }
    routes[8] = (core_machine_port_route) {.address = 0x03f6u,
        .read = provider->read, .write = provider->write, .owner = hdc};
    routes[9] = (core_machine_port_route) {.address = 0x03f7u,
        .read = core_machine_compaq_hdc_fdc_direction};
    routes[10] = (core_machine_port_route) {.address = 0x03f7u,
        .read = provider->read, .owner = hdc, .wired_or_read = LIB_TRUE};
    return core_machine_install_port_routes(machine, routes, 11u) == LIB_STATUS_OK;
}

lib_i32 main(void)
{
    const core_machine_hdc_config config = {
        .protocol = CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB, .irq = 14u,
        .service = {.command_ticks = 3u, .next_sector_ticks = 2u},
        .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .drive_address_port = 0x03f7u, .lba28_supported = LIB_FALSE,
            .clock_ticks_per_second = 8000000u}
    };
    const core_machine_media_provider media_provider = {
        .query = core_machine_compaq_hdc_query,
        .read_bytes = core_machine_compaq_hdc_read,
        .write_bytes = core_machine_compaq_hdc_write,
        .format_track = core_machine_compaq_hdc_format,
        .read_record = core_machine_compaq_hdc_read_record,
        .write_record = core_machine_compaq_hdc_write_record,
    };
    core_machine_compaq_hdc_media media = {0};
    core_machine_compaq_hdc_media slave_media = {0};
    core_machine_media_registry *registry = LIB_NULL;
    const core_machine_executor_config executor = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_8086};
    core_machine_hdc *hdc = LIB_NULL;
    core_machine_hdc *empty_hdc = LIB_NULL;
    core_machine *port = LIB_NULL;
    core_machine *empty_port = LIB_NULL;
    core_machine_pic_bus *master = LIB_NULL;
    core_machine_pic_bus *slave = LIB_NULL;
    core_machine_pic_bus *empty_master = LIB_NULL;
    core_machine_pic_bus *empty_slave = LIB_NULL;
    lib_u32 value;
    lib_i32 failed = 0;

    media.sector[0] = 0x34u;
    media.sector[1] = 0x12u;
    slave_media.sector[0] = 0x78u;
    slave_media.sector[1] = 0x56u;
    if (core_machine_neutral_create(&executor, &port) != LIB_STATUS_OK ||
        core_machine_neutral_create(&executor, &empty_port) != LIB_STATUS_OK ||
        core_machine_pic_initialize(&master, &slave, port,
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        core_machine_pic_initialize(&empty_master, &empty_slave, empty_port,
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        core_machine_hdc_create(&hdc) != LIB_STATUS_OK ||
        core_machine_hdc_create(&empty_hdc) != LIB_STATUS_OK) {
        failed |= 0x01;
        goto done;
    }
    for (lib_u32 failing_unit = 1u; failing_unit <= 2u; ++failing_unit) {
        native_create_attempts = 0u;
        native_fail_at = failing_unit;
        failed |= core_machine_hdc_configure(hdc, LIB_NULL, 1u, 2u,
            master, slave, &config) != LIB_STATUS_NO_MEMORY ||
            native_create_attempts != failing_unit || native_live != 0u ||
            hdc->chips[0] != LIB_NULL || hdc->chips[1] != LIB_NULL ||
            hdc->selected_controller != 0u || core_machine_hdc_irq_pending(hdc);
    }
    native_fail_at = 0u;
    if (core_machine_media_registry_create(&registry) != LIB_STATUS_OK ||
        core_machine_media_registry_bind(registry, 1u, &media, &media_provider) !=
            LIB_STATUS_OK) {
        failed |= 0x01;
    } else if (core_machine_media_registry_bind(registry, 2u, &slave_media, &media_provider) !=
            LIB_STATUS_OK) {
        failed |= 0x02;
    } else if (core_machine_media_registry_freeze(registry) != LIB_STATUS_OK) {
        failed |= 0x02;
    } else {
        if (core_machine_hdc_configure(hdc, registry, 1u, 2u,
                master, slave, &config) != LIB_STATUS_OK ||
            !core_machine_compaq_hdc_install(port, hdc) ||
            core_machine_freeze_execution_providers(port) != LIB_STATUS_OK ||
            core_machine_reset(port) != LIB_STATUS_OK) {
            failed |= 0x02;
        } else {
            failed |= hdc->chips[0] == LIB_NULL || hdc->chips[1] == LIB_NULL ||
                hdc->chips[0] == hdc->chips[1];
            test_hdc_port_write(port, 0x01f2u, 1u);
            test_hdc_port_write(port, 0x01f3u, 1u);
            test_hdc_port_write(port, 0x01f6u, 0x2au);
            test_hdc_port_write(port, 0x01f7u, 0x20u);
            hdc_service(hdc);
            value = test_hdc_port_read(port, 0x03f7u);
            failed |= value != 0xf6u || !core_machine_hdc_irq_pending(hdc);
            value = test_hdc_port_read(port, 0x03f6u);
            failed |= (value & X86_HDC_STATUS_DRQ) == 0u ||
                !core_machine_hdc_irq_pending(hdc);
            value = test_hdc_port_read(port, 0x01f7u);
            failed |= (value & X86_HDC_STATUS_DRQ) == 0u ||
                core_machine_hdc_irq_pending(hdc);
            value = test_hdc_port_read(port, 0x01f0u);
            failed |= value != 0x1234u;
            for (lib_u16 index = 1u; index < 256u; ++index) {
                (void)test_hdc_port_read(port, 0x01f0u);
            }
            hdc_service(hdc);
            failed |= core_machine_hdc_irq_pending(hdc);

            (void)test_hdc_port_read(port, 0x01f7u);

            test_hdc_port_write(port, 0x01f6u, 0x3au);
            test_hdc_port_write(port, 0x01f2u, 1u);
            test_hdc_port_write(port, 0x01f3u, 1u);
            test_hdc_port_write(port, 0x01f7u, 0x20u);
            hdc_service(hdc);
            value = test_hdc_port_read(port, 0x01f0u);
            failed |= value != 0x5678u;
            for (lib_u16 index = 1u; index < 256u; ++index) {
                (void)test_hdc_port_read(port, 0x01f0u);
            }
            hdc_service(hdc);
            failed |= !core_machine_hdc_irq_pending(hdc);

            failed |= test_hdc_port_read(port, 0x03f7u) != 0xf5u;
            test_hdc_port_write(port, 0x01f2u, 2u);
            test_hdc_port_write(port, 0x01f7u, 0x40u);
            hdc_service(hdc);
            failed |= (test_hdc_port_read(port, 0x03f6u) &
                (X86_HDC_STATUS_ERR | X86_HDC_STATUS_DRQ | X86_HDC_STATUS_BSY)) !=
                0u || !core_machine_hdc_irq_pending(hdc) ||
                slave_media.reads != 3u || media.reads != 1u ||
                test_hdc_port_read(port, 0x01f2u) != 0u;
            for (lib_u32 code = 0u; code < 16u; ++code) {
                const lib_u32 expected_ticks = code == 0u ? 280u :
                    code == 14u ? 26u : code == 15u ? 128u : code * 4000u;
                test_hdc_port_write(port, 0x01f7u, 0x10u | code);
                hdc_service(hdc);
                failed |= hdc_observe(hdc).step_rate_selector != code ||
                    hdc_observe(hdc).step_rate_ticks != expected_ticks;
            }
            test_hdc_port_write(port, 0x01f6u, 0x21u);
            test_hdc_port_write(port, 0x01f2u, 2u);
            test_hdc_port_write(port, 0x01f7u, 0x91u);
            hdc_service(hdc);
            test_hdc_port_write(port, 0x01f6u, 0x33u);
            test_hdc_port_write(port, 0x01f2u, 5u);
            test_hdc_port_write(port, 0x01f7u, 0x91u);
            hdc_service(hdc);
            test_hdc_port_write(port, 0x01f6u, 0x31u);
            test_hdc_port_write(port, 0x01f3u, 5u);
            test_hdc_port_write(port, 0x01f2u, 2u);
            test_hdc_port_write(port, 0x01f7u, 0x40u);
            hdc_service(hdc);
            failed |= (test_hdc_port_read(port, 0x03f6u) & X86_HDC_STATUS_ERR) != 0u ||
                slave_media.reads != 5u || slave_media.last_read_offset != 10u * 512u;
            test_hdc_port_write(port, 0x01f6u, 0x21u);
            test_hdc_port_write(port, 0x01f3u, 2u);
            test_hdc_port_write(port, 0x01f2u, 1u);
            test_hdc_port_write(port, 0x01f7u, 0x40u);
            hdc_service(hdc);
            failed |= (test_hdc_port_read(port, 0x03f6u) & X86_HDC_STATUS_ERR) != 0u ||
                media.reads != 2u || media.last_read_offset != 3u * 512u;
            test_hdc_port_write(port, 0x03f6u, X86_HDC_DEVICE_CONTROL_SRST);
            test_hdc_port_write(port, 0x03f6u, 0u);
            failed |= core_machine_hdc_irq_pending(hdc) ||
                test_hdc_port_read(port, 0x03f6u) !=
                    (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC);

            test_hdc_port_write(port, 0x01f6u, 0x31u);
            test_hdc_port_write(port, 0x01f2u, 17u);
            test_hdc_port_write(port, 0x01f4u, 0u);
            test_hdc_port_write(port, 0x01f5u, 0u);
            test_hdc_port_write(port, 0x01f7u, 0x50u);
            hdc_service(hdc);
            for (lib_u32 index = 0u; index < 255u; ++index) {
                const lib_u32 word = index < 17u ? (index + 1u) << 8u : 0u;
                test_hdc_port_write(port, 0x01f0u, word);
            }
            /* Switching controllers must preserve the slave's partial table. */
            test_hdc_port_write(port, 0x01f6u, 0x21u);
            failed |= slave_media.formats != 0u;
            test_hdc_port_write(port, 0x01f0u, 0u);
            failed |= slave_media.formats != 0u || media.formats != 0u;
            test_hdc_port_write(port, 0x01f6u, 0x31u);
            failed |= hdc_observe(hdc).data_index != 510u;
            test_hdc_port_write(port, 0x01f0u, 0u);
            hdc_service(hdc);
            failed |= slave_media.formats != 1u || media.formats != 0u ||
                slave_media.formatted.first_sector != 17u ||
                slave_media.formatted.sector_count != 17u ||
                !core_machine_hdc_irq_pending(hdc) ||
                !core_machine_pic_irq_source_is_asserted(hdc->connect.irq_source);

            /* Each selected enclosure retains its own command and data phase;
             * both are advanced by the same board time/deadline reducer. */
            core_machine_hdc_reset(hdc);
            test_hdc_port_write(port, 0x01f6u, 0x20u);
            test_hdc_port_write(port, 0x01f2u, 1u);
            test_hdc_port_write(port, 0x01f3u, 1u);
            test_hdc_port_write(port, 0x01f7u, 0x20u);
            core_machine_hdc_advance_at(hdc, 1u);
            test_hdc_port_write(port, 0x01f6u, 0x30u);
            test_hdc_port_write(port, 0x01f2u, 1u);
            test_hdc_port_write(port, 0x01f3u, 1u);
            test_hdc_port_write(port, 0x01f7u, 0x20u);
            {
                lib_u64 due = 0u;
                x86_hdc_observation primary = {0};
                failed |= core_machine_hdc_next_due_tick(hdc, &due) != LIB_STATUS_OK ||
                    due != 3u;
                core_machine_hdc_advance_at(hdc, due);
                failed |= x86_hdc_capture(hdc->chips[0], &primary) != LIB_STATUS_OK ||
                    primary.phase != X86_HDC_PHASE_DATA_READ ||
                    hdc_observe(hdc).phase != X86_HDC_PHASE_PENDING_COMMAND;
                failed |= core_machine_hdc_next_due_tick(hdc, &due) != LIB_STATUS_OK ||
                    due != 4u;
                core_machine_hdc_advance_at(hdc, due);
            }
            failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_DATA_READ ||
                !core_machine_hdc_irq_pending(hdc) ||
                !core_machine_pic_irq_source_is_asserted(hdc->connect.irq_source);
            test_hdc_port_write(port, 0x03f6u, X86_HDC_DEVICE_CONTROL_NIEN);
            failed |= core_machine_hdc_irq_pending(hdc) ||
                core_machine_pic_irq_source_is_asserted(hdc->connect.irq_source);
            test_hdc_port_write(port, 0x03f6u, 0u);
            failed |= !core_machine_hdc_irq_pending(hdc);
            (void)test_hdc_port_read(port, 0x01f7u);
            failed |= !core_machine_hdc_irq_pending(hdc) ||
                !core_machine_pic_irq_source_is_asserted(hdc->connect.irq_source);
            test_hdc_port_write(port, 0x01f6u, 0x20u);
            (void)test_hdc_port_read(port, 0x01f7u);
            failed |= core_machine_hdc_irq_pending(hdc) ||
                core_machine_pic_irq_source_is_asserted(hdc->connect.irq_source);
            failed |= test_hdc_port_read(port, 0x01f0u) != 0x1234u;
            test_hdc_port_write(port, 0x01f6u, 0x30u);
            failed |= test_hdc_port_read(port, 0x01f0u) != 0x5678u;
            for (lib_u32 index = 1u; index < 256u; ++index)
                (void)test_hdc_port_read(port, 0x01f0u);
            hdc_service(hdc);
            (void)test_hdc_port_read(port, 0x01f7u);
            test_hdc_port_write(port, 0x01f6u, 0x20u);
            failed |= hdc_observe(hdc).data_index != 2u ||
                hdc_observe(hdc).phase != X86_HDC_PHASE_DATA_READ;
            for (lib_u32 index = 1u; index < 256u; ++index)
                (void)test_hdc_port_read(port, 0x01f0u);
            hdc_service(hdc);

            /* Slave write completion does not publish the master's partial
             * record. Shared reset cancels the remaining master data phase. */
            core_machine_hdc_reset(hdc);
            test_hdc_port_write(port, 0x01f6u, 0x20u);
            test_hdc_port_write(port, 0x01f2u, 1u);
            test_hdc_port_write(port, 0x01f3u, 1u);
            test_hdc_port_write(port, 0x01f7u, 0x30u);
            hdc_service(hdc);
            test_hdc_port_write(port, 0x01f0u, 0x1111u);
            test_hdc_port_write(port, 0x01f6u, 0x30u);
            test_hdc_port_write(port, 0x01f2u, 1u);
            test_hdc_port_write(port, 0x01f3u, 1u);
            test_hdc_port_write(port, 0x01f7u, 0x30u);
            hdc_service(hdc);
            for (lib_u32 index = 0u; index < 256u; ++index)
                test_hdc_port_write(port, 0x01f0u, 0x2222u);
            hdc_service(hdc);
            failed |= slave_media.sector[0] != 0x22u ||
                slave_media.sector[1] != 0x22u || media.sector[0] != 0x34u;
            test_hdc_port_write(port, 0x01f6u, 0x20u);
            failed |= hdc_observe(hdc).data_index != 2u ||
                hdc_observe(hdc).phase != X86_HDC_PHASE_DATA_WRITE;
            test_hdc_port_write(port, 0x03f6u, X86_HDC_DEVICE_CONTROL_SRST);
            test_hdc_port_write(port, 0x03f6u, 0u);
            failed |= core_machine_hdc_irq_pending(hdc) ||
                hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE ||
                media.sector[0] != 0x34u;
            test_hdc_port_write(port, 0x01f6u, 0x30u);
            failed |= hdc_observe(hdc).phase != X86_HDC_PHASE_IDLE;

            /* A fitted Compaq controller remains reset-ready with no mounted
             * image.  Firmware may probe it before deciding to boot the FDD;
             * only a sector command is allowed to report absent media. */
            if (core_machine_hdc_configure(empty_hdc, registry, 3u,
                    CORE_MACHINE_MEDIA_ID_INVALID, empty_master, empty_slave,
                    &config) != LIB_STATUS_OK ||
                !core_machine_compaq_hdc_install(empty_port, empty_hdc) ||
                core_machine_freeze_execution_providers(empty_port) != LIB_STATUS_OK ||
                core_machine_reset(empty_port) != LIB_STATUS_OK) {
                failed |= 0x04;
            } else {
                test_hdc_port_write(empty_port, 0x01f6u, 0xa0u);
                failed |= test_hdc_port_read(empty_port, 0x03f6u) !=
                    (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC);
            }
        }
    }
done:
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "COMPAQ-HDC-ROUTE:FAIL %x status=%x error=%x phase=%u irq=%u chs=%x:%x:%x\n", failed, hdc_observe(hdc).status, hdc_observe(hdc).error, hdc_observe(hdc).phase, hdc_observe(hdc).irq_pending, hdc_observe(hdc).cylinder_high, hdc_observe(hdc).cylinder_low, hdc_observe(hdc).sector_number);
    }
    core_machine_hdc_destroy(empty_hdc);
    core_machine_hdc_destroy(hdc);
    failed |= native_live != 0u;
    core_machine_media_registry_destroy(registry);
    core_machine_pic_finalize(empty_master, empty_slave);
    core_machine_pic_finalize(master, slave);
    core_machine_destroy(empty_port);
    core_machine_destroy(port);
    if (failed) return 1;
    lib_c_printf("%s\n", "COMPAQ-HDC-ROUTE:OK");
    lib_c_printf("%s\n", "PORT-WIRED-OR:OK");
    lib_c_printf("%s\n", "COMPAQ-HDC-DUAL-DRIVE:OK");
    return 0;
}
