#include "hdc_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86/ibmpc-common/hdc.h"
#include "x86/ibmpc-common/media_interface.h"
#include "x86/ibmpc-common/pic_bus_interface.h"
#include "x86/core/machine_interface.h"

typedef struct core_machine_compaq_hdc_media {
    lib_u8 sector[512];
} core_machine_compaq_hdc_media;

static core_machine_media_result core_machine_compaq_hdc_query(void *opaque,
    core_machine_media_info *out_info)
{
    if (opaque == LIB_NULL || out_info == LIB_NULL) return CORE_MACHINE_MEDIA_RESULT_ABSENT;
    lib_memory_set(out_info, 0, sizeof(*out_info));
    out_info->present = LIB_TRUE;
    out_info->capabilities = CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN;
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

    (void)offset;
    if (media == LIB_NULL || buffer == LIB_NULL || byte_count != 512u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    lib_memory_copy(buffer, media->sector, sizeof(media->sector));
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static core_machine_media_result core_machine_compaq_hdc_write(void *opaque,
    lib_u64 offset, const void *buffer, lib_u32 byte_count)
{
    core_machine_compaq_hdc_media *media = opaque;

    (void)offset;
    if (media == LIB_NULL || buffer == LIB_NULL || byte_count != 512u) {
        return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    }
    lib_memory_copy(media->sector, buffer, sizeof(media->sector));
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
        .bus.task_file = {
            .data_port = 0x01f0u, .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .drive_address_port = 0x03f7u, .lba28_supported = LIB_FALSE}
    };
    const core_machine_media_provider media_provider = {
        core_machine_compaq_hdc_query, core_machine_compaq_hdc_read,
        core_machine_compaq_hdc_write, LIB_NULL, LIB_NULL, LIB_NULL, LIB_NULL
    };
    core_machine_compaq_hdc_media media = {{0}};
    core_machine_compaq_hdc_media slave_media = {{0}};
    core_machine_media_registry *registry = LIB_NULL;
    const core_machine_executor_config executor = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086};
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
            test_hdc_port_write(port, 0x01f2u, 1u);
            test_hdc_port_write(port, 0x01f3u, 1u);
            test_hdc_port_write(port, 0x01f6u, 0x2au);
            test_hdc_port_write(port, 0x01f7u, 0x20u);
            hdc_service(hdc);
            value = test_hdc_port_read(port, 0x03f7u);
            failed |= value != 0x8au || !core_machine_hdc_irq_pending(hdc);
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
            failed |= !core_machine_hdc_irq_pending(hdc);

            test_hdc_port_write(port, 0x01f2u, 1u);
            test_hdc_port_write(port, 0x01f3u, 1u);
            test_hdc_port_write(port, 0x01f6u, 0x3au);
            test_hdc_port_write(port, 0x01f7u, 0x20u);
            hdc_service(hdc);
            value = test_hdc_port_read(port, 0x01f0u);
            failed |= value != 0x5678u;
            for (lib_u16 index = 1u; index < 256u; ++index) {
                (void)test_hdc_port_read(port, 0x01f0u);
            }
            hdc_service(hdc);
            failed |= !core_machine_hdc_irq_pending(hdc);

            test_hdc_port_write(port, 0x01f7u, 0x40u);
            hdc_service(hdc);
            failed |= (test_hdc_port_read(port, 0x03f6u) & X86_HDC_STATUS_ERR) !=
                0u || !core_machine_hdc_irq_pending(hdc);
            test_hdc_port_write(port, 0x03f6u, X86_HDC_DEVICE_CONTROL_SRST);
            test_hdc_port_write(port, 0x03f6u, 0u);
            failed |= core_machine_hdc_irq_pending(hdc) ||
                test_hdc_port_read(port, 0x03f6u) !=
                    (X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC);

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
        fprintf(stderr, "M5:T386:S5:COMPAQ-HDC-ROUTE:FAIL %x status=%x error=%x phase=%u irq=%u chs=%x:%x:%x\n", failed, hdc_observe(hdc).status, hdc_observe(hdc).error, hdc_observe(hdc).phase, hdc_observe(hdc).irq_pending, hdc_observe(hdc).cylinder_high, hdc_observe(hdc).cylinder_low, hdc_observe(hdc).sector_number);
    }
    core_machine_hdc_destroy(empty_hdc);
    core_machine_hdc_destroy(hdc);
    core_machine_media_registry_destroy(registry);
    core_machine_pic_finalize(empty_master, empty_slave);
    core_machine_pic_finalize(master, slave);
    core_machine_destroy(empty_port);
    core_machine_destroy(port);
    if (failed) return 1;
    puts("M5:T386:S5:COMPAQ-HDC-ROUTE:OK");
    puts("M5:T386:S5:PORT-WIRED-OR:OK");
    puts("M5:T430:S1:COMPAQ-HDC-DUAL-DRIVE:OK");
    return 0;
}
