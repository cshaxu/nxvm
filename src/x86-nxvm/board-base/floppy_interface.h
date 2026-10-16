#ifndef X86_NXVM_BOARD_DEVICE_FLOPPY_H
#define X86_NXVM_BOARD_DEVICE_FLOPPY_H
#include "lib/types/types_interface.h"

#include "x86-nxvm/board-base/media_interface.h"
#include "x86-nxvm/board-base/controller_interface.h"

typedef enum x86_nxvm_board_floppy_kind {
    X86_NXVM_BOARD_FLOPPY_35_1440K = 0,
    X86_NXVM_BOARD_FLOPPY_525_1200K,
    X86_NXVM_BOARD_FLOPPY_525_360K,
    X86_NXVM_BOARD_FLOPPY_35_720K
} x86_nxvm_board_floppy_kind;

const x86_nxvm_board_media_geometry *x86_nxvm_board_floppy_geometry_get(
    x86_nxvm_board_floppy_kind kind);
lib_u8 x86_nxvm_board_floppy_cmos_type_get(x86_nxvm_board_floppy_kind kind);
x86_nxvm_board_fdc_channel_provider x86_nxvm_board_floppy_channel_get(
    x86_nxvm_board_floppy_kind kind);

#endif
