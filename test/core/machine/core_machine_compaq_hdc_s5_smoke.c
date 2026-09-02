#include "type.h"

#include "core/machine/hdc.h"
#include "core/machine/media_interface.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"

typedef struct core_machine_compaq_hdc_media {
    type_unsigned_8 sector[512];
} core_machine_compaq_hdc_media;

static core_machine_media_result core_machine_compaq_hdc_query(C_VOID *opaque,
    core_machine_media_info *out_info)
{
    if (opaque == STD_NULL || out_info == STD_NULL) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    STD_MEMSET(out_info, 0, sizeof(*out_info));
    out_info->present = TYPE_TRUE;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 16u;
    out_info->geometry.sectors_per_track = 17u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 272u;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_compaq_hdc_read(C_VOID *opaque,
    type_unsigned_64 offset, C_VOID *buffer, type_unsigned_32 byte_count)
{
    core_machine_compaq_hdc_media *media = opaque;

    (C_VOID)offset;
    if (media == STD_NULL || buffer == STD_NULL || byte_count != 512u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    STD_MEMCPY(buffer, media->sector, sizeof(media->sector));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_compaq_hdc_write(C_VOID *opaque,
    type_unsigned_64 offset, const C_VOID *buffer, type_unsigned_32 byte_count)
{
    core_machine_compaq_hdc_media *media = opaque;

    (C_VOID)offset;
    if (media == STD_NULL || buffer == STD_NULL || byte_count != 512u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    STD_MEMCPY(media->sector, buffer, sizeof(media->sector));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}
static type_status core_machine_compaq_hdc_fdc_direction(C_VOID *opaque,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    (C_VOID)opaque;
    if (port != 0x03f7u || out_value == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = 0x80u;
    return TYPE_STATUS_OK;
}

static C_INT core_machine_compaq_hdc_install(t_port *port, core_machine_hdc *hdc)
{
    const core_machine_port_provider *provider = core_machine_hdc_port_provider();
    type_unsigned_16 value;

    if (port == STD_NULL || hdc == STD_NULL || provider == STD_NULL) return 0;
    for (value = 0x01f0u; value <= 0x01f7u; ++value) {
        if (core_machine_port_add_read_provider(port, value, provider->read, hdc) !=
                TYPE_STATUS_OK ||
            core_machine_port_add_write_provider(port, value, provider->write, hdc) !=
                TYPE_STATUS_OK) return 0;
    }
    return core_machine_port_add_read_provider(port, 0x03f6u, provider->read, hdc) ==
            TYPE_STATUS_OK &&
        core_machine_port_add_write_provider(port, 0x03f6u, provider->write, hdc) ==
            TYPE_STATUS_OK &&
        core_machine_port_add_read_provider(port, 0x03f7u,
            core_machine_compaq_hdc_fdc_direction, STD_NULL) == TYPE_STATUS_OK &&
        core_machine_port_add_read_wired_or_provider(port, 0x03f7u, provider->read, hdc) ==
            TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    const core_machine_hdc_config config = {
        .protocol = CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB, .irq = 14u,
        .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .drive_address_port = 0x03f7u, .lba28_supported = TYPE_FALSE}
    };
    const core_machine_media_provider media_provider = {
        core_machine_compaq_hdc_query, core_machine_compaq_hdc_read,
        core_machine_compaq_hdc_write, STD_NULL, STD_NULL, STD_NULL, STD_NULL
    };
    core_machine_compaq_hdc_media media = {{0}};
    core_machine_compaq_hdc_media slave_media = {{0}};
    core_machine_media_registry *registry = STD_NULL;
    core_machine_hdc hdc = {0};
    core_machine_hdc empty_hdc = {0};
    t_port port = {0};
    t_port empty_port = {0};
    t_pic master = {0};
    t_pic slave = {0};
    type_unsigned_32 value;
    C_INT failed = 0;

    media.sector[0] = 0x34u;
    media.sector[1] = 0x12u;
    slave_media.sector[0] = 0x78u;
    slave_media.sector[1] = 0x56u;
    core_machine_port_initialize(&port);
    core_machine_pic_initialize(&master, &slave, &port, CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    if (core_machine_media_registry_create(&registry) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(registry, 1u, &media, &media_provider) !=
            TYPE_STATUS_OK) {
        failed |= 0x01;
    } else if (core_machine_media_registry_bind(registry, 2u, &slave_media, &media_provider) !=
            TYPE_STATUS_OK) {
        failed |= 0x02;
    } else if (core_machine_media_registry_freeze(registry) != TYPE_STATUS_OK) {
        failed |= 0x02;
    } else {
        core_machine_hdc_connect(&hdc, registry, 1u, 2u, &master, &slave, &config);
        core_machine_hdc_initialize(&hdc);
        if (!core_machine_compaq_hdc_install(&port, &hdc)) {
            failed |= 0x02;
        } else {
            core_machine_port_write(&port, 0x01f2u, 1u);
            core_machine_port_write(&port, 0x01f3u, 1u);
            core_machine_port_write(&port, 0x01f6u, 0x2au);
            core_machine_port_write(&port, 0x01f7u, 0x20u);
            core_machine_hdc_advance(&hdc);
            value = core_machine_port_read(&port, 0x03f7u);
            failed |= value != 0x8au || !core_machine_hdc_irq_pending(&hdc);
            value = core_machine_port_read(&port, 0x03f6u);
            failed |= (value & CORE_MACHINE_HDC_STATUS_DRQ) == 0u ||
                !core_machine_hdc_irq_pending(&hdc);
            value = core_machine_port_read(&port, 0x01f7u);
            failed |= (value & CORE_MACHINE_HDC_STATUS_DRQ) == 0u ||
                core_machine_hdc_irq_pending(&hdc);
            value = core_machine_port_read(&port, 0x01f0u);
            failed |= value != 0x1234u;
            for (type_unsigned_16 index = 1u; index < 256u; ++index) {
                (C_VOID)core_machine_port_read(&port, 0x01f0u);
            }
            core_machine_hdc_advance(&hdc);
            failed |= !core_machine_hdc_irq_pending(&hdc);

            core_machine_port_write(&port, 0x01f2u, 1u);
            core_machine_port_write(&port, 0x01f3u, 1u);
            core_machine_port_write(&port, 0x01f6u, 0x3au);
            core_machine_port_write(&port, 0x01f7u, 0x20u);
            core_machine_hdc_advance(&hdc);
            value = core_machine_port_read(&port, 0x01f0u);
            failed |= value != 0x5678u;
            for (type_unsigned_16 index = 1u; index < 256u; ++index) {
                (C_VOID)core_machine_port_read(&port, 0x01f0u);
            }
            core_machine_hdc_advance(&hdc);
            failed |= !core_machine_hdc_irq_pending(&hdc);

            core_machine_port_write(&port, 0x01f7u, 0x40u);
            core_machine_hdc_advance(&hdc);
            failed |= (core_machine_port_read(&port, 0x03f6u) & CORE_MACHINE_HDC_STATUS_ERR) !=
                0u || !core_machine_hdc_irq_pending(&hdc);
            core_machine_port_write(&port, 0x01f4u, 0x7fu);
            core_machine_port_write(&port, 0x01f5u, 0x03u);
            core_machine_port_write(&port, 0x01f7u, 0x10u);
            core_machine_hdc_advance(&hdc);
            failed |= hdc.data.cylinder_low != 0u || hdc.data.cylinder_high != 0u;
            core_machine_port_write(&port, 0x01f7u, 0x91u);
            core_machine_hdc_advance(&hdc);
            failed |= (core_machine_port_read(&port, 0x01f7u) & CORE_MACHINE_HDC_STATUS_ERR) != 0u;
            core_machine_port_write(&port, 0x01f7u, 0x90u);
            core_machine_hdc_advance(&hdc);
            failed |= core_machine_port_read(&port, 0x01f1u) != 0x01u;
            core_machine_port_write(&port, 0x01f7u, 0xecu);
            core_machine_hdc_advance(&hdc);
            failed |= (core_machine_port_read(&port, 0x01f7u) & CORE_MACHINE_HDC_STATUS_ERR) ==
                0u || core_machine_port_read(&port, 0x01f1u) != CORE_MACHINE_HDC_ERROR_ABORT;
            core_machine_port_write(&port, 0x01f7u, 0x22u);
            core_machine_hdc_advance(&hdc);
            failed |= (core_machine_port_read(&port, 0x01f7u) & CORE_MACHINE_HDC_STATUS_ERR) ==
                0u || core_machine_port_read(&port, 0x01f1u) != CORE_MACHINE_HDC_ERROR_ABORT;
            core_machine_port_write(&port, 0x03f6u, CORE_MACHINE_HDC_DEVICE_CONTROL_SRST);
            core_machine_port_write(&port, 0x03f6u, 0u);
            failed |= core_machine_hdc_irq_pending(&hdc) ||
                core_machine_port_read(&port, 0x03f6u) !=
                    (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC);

            /* A fitted Compaq controller remains reset-ready with no mounted
             * image.  Firmware may probe it before deciding to boot the FDD;
             * only a sector command is allowed to report absent media. */
            core_machine_port_initialize(&empty_port);
            core_machine_hdc_connect(&empty_hdc, registry, 3u,
                CORE_MACHINE_MEDIA_ID_INVALID, &master, &slave, &config);
            core_machine_hdc_initialize(&empty_hdc);
            if (!core_machine_compaq_hdc_install(&empty_port, &empty_hdc)) {
                failed |= 0x04;
            } else {
                core_machine_port_write(&empty_port, 0x01f6u, 0xa0u);
                failed |= core_machine_port_read(&empty_port, 0x03f6u) !=
                    (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC);
            }
        }
    }
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T386:S5:COMPAQ-HDC-ROUTE:FAIL %x status=%x error=%x phase=%u irq=%u chs=%x:%x:%x\n", failed, hdc.data.status, hdc.data.error, hdc.data.phase, hdc.data.irq_pending, hdc.data.cylinder_high, hdc.data.cylinder_low, hdc.data.sector_number);
        core_machine_hdc_finalize(&hdc);
        core_machine_media_registry_destroy(registry);
        core_machine_port_finalize(&port);
        return 1;
    }
    core_machine_hdc_finalize(&empty_hdc);
    core_machine_port_finalize(&empty_port);
    core_machine_hdc_finalize(&hdc);
    core_machine_media_registry_destroy(registry);
    core_machine_port_finalize(&port);
    puts("M5:T386:S5:COMPAQ-HDC-ROUTE:OK");
    puts("M5:T386:S5:PORT-WIRED-OR:OK");
    puts("M5:T430:S1:COMPAQ-HDC-DUAL-DRIVE:OK");
    return 0;
}
