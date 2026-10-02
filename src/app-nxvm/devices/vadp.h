/* NXVM video board adapter: PC routes and copied display snapshots. */
#ifndef CORE_MACHINE_VADP_H
#define CORE_MACHINE_VADP_H
#include "x86/chips/video/video_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"

#define CORE_MACHINE_DEVICE_VADP "CGA Adapter"
#define CORE_MACHINE_VADP_VIDEO_BASE 0x000b8000u
#define CORE_MACHINE_VADP_VIDEO_BYTES 0x00004000u
#define CORE_MACHINE_VADP_TEXT_BASE CORE_MACHINE_VADP_VIDEO_BASE
#define CORE_MACHINE_VADP_TEXT_BYTES CORE_MACHINE_VADP_VIDEO_BYTES
#define CORE_MACHINE_VADP_EGA_APERTURE_BASE 0x000a0000u
#define CORE_MACHINE_VADP_EGA_APERTURE_BYTES 0x00010000u
/* Register the full EGA CPU-decode span.  Graphics Controller register 6
 * selects a smaller active window inside this span. */
#define CORE_MACHINE_VADP_EGA_CPU_DECODE_BYTES 0x00020000u
#define CORE_MACHINE_VADP_PORT_ATTRIBUTE 0x03c0u
#define CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ 0x03c1u
#define CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX 0x03c4u
#define CORE_MACHINE_VADP_PORT_SEQUENCER_DATA 0x03c5u
#define CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX 0x03ceu
#define CORE_MACHINE_VADP_PORT_GRAPHICS_DATA 0x03cfu
#define CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX 0x03b4u
#define CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA 0x03b5u
#define CORE_MACHINE_VADP_PORT_CRTC_INDEX 0x03d4u
#define CORE_MACHINE_VADP_PORT_CRTC_DATA 0x03d5u
#define CORE_MACHINE_VADP_PORT_MONO_STATUS 0x03bau
#define CORE_MACHINE_VADP_PORT_MONO_LIGHTPEN_LATCH_RESET 0x03bbu
#define CORE_MACHINE_VADP_PORT_MONO_LIGHTPEN_LATCH_SET 0x03bcu
#define CORE_MACHINE_VADP_PORT_MODE 0x03d8u
#define CORE_MACHINE_VADP_PORT_COLOR 0x03d9u
#define CORE_MACHINE_VADP_PORT_STATUS 0x03dau
#define CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT 0x03c2u
#define CORE_MACHINE_VADP_PORT_VGA_DAC_MASK 0x03c6u
#define CORE_MACHINE_VADP_PORT_VGA_DAC_READ_INDEX 0x03c7u
#define CORE_MACHINE_VADP_PORT_VGA_DAC_WRITE_INDEX 0x03c8u
#define CORE_MACHINE_VADP_PORT_VGA_DAC_DATA 0x03c9u
#define CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0 0x03c2u
#define CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_MONO 0x03bau
#define CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_COLOR 0x03dau
#define CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT 0x03c2u
#define CORE_MACHINE_VADP_PORT_COMPAQ_FEATURE_CONTROL \
    CORE_MACHINE_VADP_PORT_STATUS
#define CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE 0x03c6u
#define CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_RESET 0x03dbu
#define CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET 0x03dcu
#define CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT 0x07c6u
#define CORE_MACHINE_VADP_PORT_COMPAQ_DISPLAY_TYPE 0x0bc6u
#define CORE_MACHINE_VADP_PORT_COMPAQ_INITIAL_MODE 0x0fc6u


typedef struct core_machine core_machine;
typedef struct core_machine_display_config core_machine_display_config;

typedef struct t_vadp {
    x86_video *chip;
    core_machine *machine;
    lib_bool configured;
} t_vadp;

lib_status core_machine_vadp_initialize(t_vadp *adapter, core_machine *machine);
lib_status core_machine_vadp_configure(t_vadp *adapter,
    const core_machine_display_config *config);
void core_machine_vadp_finalize(t_vadp *adapter);
lib_i32 core_machine_vadp_capture_snapshot(t_vadp *adapter,
    x86_video_snapshot *out_snapshot);
#endif
