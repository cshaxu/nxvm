/* PC video board adapter: routes and copied display snapshots. */
#ifndef X86_NXVM_BOARD_VADP_H
#define X86_NXVM_BOARD_VADP_H
#include "x86-nxvm/chips/video/video_interface.h"
#include "x86-nxvm/board-base/vadp_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"

#define X86_NXVM_BOARD_DEVICE_VADP "CGA Adapter"
#define X86_NXVM_BOARD_VADP_VIDEO_BASE 0x000b8000u
#define X86_NXVM_BOARD_VADP_VIDEO_BYTES 0x00004000u
#define X86_NXVM_BOARD_VADP_TEXT_BASE X86_NXVM_BOARD_VADP_VIDEO_BASE
#define X86_NXVM_BOARD_VADP_TEXT_BYTES X86_NXVM_BOARD_VADP_VIDEO_BYTES
/* Register the full EGA CPU-decode span.  Graphics Controller register 6
 * selects a smaller active window inside this span. */
#define X86_NXVM_BOARD_VADP_EGA_CPU_DECODE_BYTES 0x00020000u
#define X86_NXVM_BOARD_VADP_PORT_ATTRIBUTE 0x03c0u
#define X86_NXVM_BOARD_VADP_PORT_ATTRIBUTE_DATA_READ 0x03c1u
#define X86_NXVM_BOARD_VADP_PORT_SEQUENCER_INDEX 0x03c4u
#define X86_NXVM_BOARD_VADP_PORT_SEQUENCER_DATA 0x03c5u
#define X86_NXVM_BOARD_VADP_PORT_GRAPHICS_INDEX 0x03ceu
#define X86_NXVM_BOARD_VADP_PORT_GRAPHICS_DATA 0x03cfu
#define X86_NXVM_BOARD_VADP_PORT_MONO_CRTC_INDEX 0x03b4u
#define X86_NXVM_BOARD_VADP_PORT_MONO_CRTC_DATA 0x03b5u
#define X86_NXVM_BOARD_VADP_PORT_CRTC_INDEX 0x03d4u
#define X86_NXVM_BOARD_VADP_PORT_CRTC_DATA 0x03d5u
#define X86_NXVM_BOARD_VADP_PORT_MONO_STATUS 0x03bau
#define X86_NXVM_BOARD_VADP_PORT_MONO_LIGHTPEN_LATCH_RESET 0x03bbu
#define X86_NXVM_BOARD_VADP_PORT_MONO_LIGHTPEN_LATCH_SET 0x03bcu
#define X86_NXVM_BOARD_VADP_PORT_MODE 0x03d8u
#define X86_NXVM_BOARD_VADP_PORT_COLOR 0x03d9u
#define X86_NXVM_BOARD_VADP_PORT_STATUS 0x03dau
#define X86_NXVM_BOARD_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT 0x03c2u
#define X86_NXVM_BOARD_VADP_PORT_VGA_DAC_MASK 0x03c6u
#define X86_NXVM_BOARD_VADP_PORT_VGA_DAC_READ_INDEX 0x03c7u
#define X86_NXVM_BOARD_VADP_PORT_VGA_DAC_WRITE_INDEX 0x03c8u
#define X86_NXVM_BOARD_VADP_PORT_VGA_DAC_DATA 0x03c9u
#define X86_NXVM_BOARD_VADP_PORT_EGA_INPUT_STATUS_0 0x03c2u
#define X86_NXVM_BOARD_VADP_PORT_EGA_FEATURE_CONTROL_MONO 0x03bau
#define X86_NXVM_BOARD_VADP_PORT_EGA_FEATURE_CONTROL_COLOR 0x03dau
#define X86_NXVM_BOARD_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT 0x03c2u
#define X86_NXVM_BOARD_VADP_PORT_COMPAQ_FEATURE_CONTROL \
    X86_NXVM_BOARD_VADP_PORT_STATUS
#define X86_NXVM_BOARD_VADP_PORT_COMPAQ_CONTROL_MODE 0x03c6u
#define X86_NXVM_BOARD_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_RESET 0x03dbu
#define X86_NXVM_BOARD_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET 0x03dcu
#define X86_NXVM_BOARD_VADP_PORT_COMPAQ_ENVIRONMENT 0x07c6u
#define X86_NXVM_BOARD_VADP_PORT_COMPAQ_DISPLAY_TYPE 0x0bc6u
#define X86_NXVM_BOARD_VADP_PORT_COMPAQ_INITIAL_MODE 0x0fc6u


typedef struct x86_core_machine x86_core_machine;
typedef struct x86_nxvm_board_display_config x86_nxvm_board_display_config;

struct t_vadp {
    x86_video *chip;
    x86_core_machine *machine;
    lib_bool configured;
    x86_nxvm_board_pic_irq_source *irq_source; /* Borrowed; PIC outlives VADP. */
};

lib_status x86_nxvm_board_vadp_initialize(t_vadp *adapter, x86_core_machine *machine);
lib_status x86_nxvm_board_vadp_configure(t_vadp *adapter,
    const x86_nxvm_board_display_config *config);
void x86_nxvm_board_vadp_finalize(t_vadp *adapter);
#endif
