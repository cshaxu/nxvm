#include "x86-nxvm/machine/machine_interface.h"
#include "../../x86-nxvm/machine/support/media.h"
#include "../../x86-nxvm/board-base/support/controller_fixture.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/media/hdd_interface.h"
#include "../support/model40_session_assets.h"

#define MODEL40_HDC_BYTES (925u * 5u * 17u * 512u)

static lib_i32 read_first_sector(x86_nxvm_machine *session, lib_u8 drive_head,
    lib_u16 expected_word)
{
    lib_u32 value;
    lib_u16 index;

    if (session == LIB_NULL || session->x86_core_machine == LIB_NULL) return 0;
    if (x86_core_machine_bus_write(session->x86_core_machine, 0x01f2u, 1u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x01f3u, 1u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x01f4u, 0u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x01f5u, 0u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x01f6u, drive_head) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x01f7u, 0x20u) != LIB_STATUS_OK) return 0;
    test_board_hdc_service(session->board);
    if (x86_core_machine_bus_read(session->x86_core_machine, 0x03f6u, &value) != LIB_STATUS_OK ||
        (value & X86_HDC_STATUS_DRQ) == 0u || !test_board_hdc_irq_pending(session->board)) return 0;
    if (x86_core_machine_bus_read(session->x86_core_machine, 0x01f7u, &value) != LIB_STATUS_OK ||
        (value & X86_HDC_STATUS_DRQ) == 0u || test_board_hdc_irq_pending(session->board) ||
        x86_core_machine_bus_read(session->x86_core_machine, 0x01f0u, &value) != LIB_STATUS_OK ||
        value != expected_word) return 0;
    for (index = 1u; index < 256u; ++index) {
        if (x86_core_machine_bus_read(session->x86_core_machine, 0x01f0u, &value) != LIB_STATUS_OK)
            return 0;
    }
    test_board_hdc_service(session->board);
    if (test_board_hdc_irq_pending(session->board)) return 0;
    if (x86_core_machine_bus_read(session->x86_core_machine, 0x01f7u, &value) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x01f7u, 0xecu) != LIB_STATUS_OK) return 0;
    test_board_hdc_service(session->board);
    if (x86_core_machine_bus_read(session->x86_core_machine, 0x01f7u, &value) != LIB_STATUS_OK ||
        (value & X86_HDC_STATUS_ERR) == 0u || test_board_hdc_irq_pending(session->board) ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x03f6u,
            X86_HDC_DEVICE_CONTROL_SRST) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x03f6u, 0u) != LIB_STATUS_OK) return 0;
    return !test_board_hdc_irq_pending(session->board) &&
        x86_core_machine_bus_read(session->x86_core_machine, 0x01f1u, &value) == LIB_STATUS_OK &&
        value == X86_HDC_ERROR_DIAGNOSTIC_OK;
}

lib_i32 main(void)
{
    lib_u8 *image = (lib_u8 *)lib_allocate_zero(1u, MODEL40_HDC_BYTES);
    x86_nxvm_machine *session = LIB_NULL;
    lib_i32 failed = image == LIB_NULL;

    if (!failed) {
        image[0u] = 0xa5u;
        image[1u] = 0x5au;
        failed = vm_model40_fixture_create(&session) != LIB_STATUS_OK || session == LIB_NULL ||
            x86_nxvm_machine_hdd_replace_bytes(session->fixed_disk[0u], image, MODEL40_HDC_BYTES) ||
            x86_nxvm_machine_hdd_set_geometry(session->fixed_disk[0u], 925u, 5u, 17u) ||
            !vm_test_hdd_info(session->fixed_disk[0u]).present ||
            test_board_hdc_connection_config(session->board).service.command_ticks != 0u ||
            test_board_hdc_connection_config(session->board).service.next_sector_ticks != 0u ||
            vm_test_hdd_info(session->fixed_disk[0u]).geometry.cylinders != 925u || vm_test_hdd_info(session->fixed_disk[0u]).geometry.heads != 5u ||
            vm_test_hdd_info(session->fixed_disk[0u]).geometry.sectors_per_track != 17u || vm_test_hdd_info(session->fixed_disk[0u]).geometry.bytes_per_sector != 512u ||
            test_board_hdc_slave_media_id(session->board) != X86_NXVM_BOARD_MEDIA_ID_INVALID ||
            !read_first_sector(session, 0x20u, 0x5aa5u) ||
            !read_first_sector(session, 0xa0u, 0x5aa5u);
    }
    x86_nxvm_machine_destroy(session);
    lib_release(image);
    if (failed) return 1;
    lib_c_printf("MODEL40-HDC-MEMORY-MEDIA:OK\n");
    return 0;
}
