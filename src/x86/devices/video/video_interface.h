/* Copyright 2012-2026 Neko. */
#ifndef X86_VIDEO_INTERFACE_H
#define X86_VIDEO_INTERFACE_H
#include "x86/devices/video/video_values_interface.h"

typedef struct x86_video x86_video;

/* Copied bus-gating diagnostics for an execution-owner observation point.
 * Sampling has no indexed-register, latch, dirty-generation or frame effect.
 * cpu_memory_disabled is the enhanced-color gate, false in generic mode. */
typedef struct x86_video_bus_observation {
    lib_bool cpu_memory_disabled;
    lib_u8 graphics_miscellaneous;
    lib_u8 sequencer_reset;
} x86_video_bus_observation;

lib_status x86_video_observe_bus(const x86_video *video,
    x86_video_bus_observation *out_observation);
typedef enum x86_video_register {
    X86_VIDEO_REGISTER_MONO_CRTC_INDEX,
    X86_VIDEO_REGISTER_MONO_CRTC_DATA,
    X86_VIDEO_REGISTER_MONO_STATUS,
    X86_VIDEO_REGISTER_MONO_LIGHTPEN_RESET,
    X86_VIDEO_REGISTER_MONO_LIGHTPEN_SET,
    X86_VIDEO_REGISTER_ATTRIBUTE,
    X86_VIDEO_REGISTER_ATTRIBUTE_DATA,
    X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
    X86_VIDEO_REGISTER_SEQUENCER_INDEX,
    X86_VIDEO_REGISTER_SEQUENCER_DATA,
    X86_VIDEO_REGISTER_AUXILIARY_CONTROL,
    X86_VIDEO_REGISTER_DAC_READ_INDEX,
    X86_VIDEO_REGISTER_DAC_WRITE_INDEX,
    X86_VIDEO_REGISTER_DAC_DATA,
    X86_VIDEO_REGISTER_GRAPHICS_INDEX,
    X86_VIDEO_REGISTER_GRAPHICS_DATA,
    X86_VIDEO_REGISTER_COLOR_CRTC_INDEX,
    X86_VIDEO_REGISTER_COLOR_CRTC_DATA,
    X86_VIDEO_REGISTER_MODE,
    X86_VIDEO_REGISTER_COLOR,
    X86_VIDEO_REGISTER_COLOR_STATUS,
    X86_VIDEO_REGISTER_COLOR_LIGHTPEN_RESET,
    X86_VIDEO_REGISTER_COLOR_LIGHTPEN_SET,
    X86_VIDEO_REGISTER_ENVIRONMENT,
    X86_VIDEO_REGISTER_DISPLAY_TYPE,
    X86_VIDEO_REGISTER_INITIAL_MODE,
} x86_video_register;

typedef enum x86_video_memory_region {
    X86_VIDEO_MEMORY_CGA,
    X86_VIDEO_MEMORY_PLANAR
} x86_video_memory_region;

/* One caller owns execution. Configuration precedes runtime use; this component
 * does not register board routes. Memory readers are borrowed for each capture.
 * Values and snapshots are copied; no mutable layout or host service is exposed.
 * Reset retains configuration, clears VRAM and retains the established CRTC rules.
 * Register reads preserve the supplied byte when the selected operation is inactive.
 * Backing-memory capture failure is not a successful frame publication. */
lib_status x86_video_register_read(x86_video *adapter,
    x86_video_register port_id, lib_u8 *out_value);
lib_status x86_video_register_write(x86_video *adapter,
    x86_video_register port_id, lib_u8 value);
lib_status x86_video_configure_ega_personality(x86_video *adapter,
    x86_video_ega_personality personality);
lib_status x86_video_configure_vga(x86_video *adapter);
lib_i32 x86_video_cecg_config_is_valid(
    const x86_video_cecg_config *config);
lib_status x86_video_configure_cecg(x86_video *adapter,
    const x86_video_cecg_config *config);
void x86_video_reset(x86_video *adapter);
lib_status x86_video_configure_text_glyphs(x86_video *adapter,
    const x86_video_text_glyph_config *config);
void x86_video_advance(x86_video *adapter, lib_u64 elapsed_ticks);
lib_status x86_video_configure_text_timing(x86_video *adapter,
    const x86_video_text_timing *timing);
lib_status x86_video_configure_cga_memory(x86_video *adapter);
lib_status x86_video_configure_ega_sequencer(x86_video *adapter,
    const x86_video_ega_sequencer_config *config);
lib_status x86_video_configure_ega_controllers(x86_video *adapter,
    const x86_video_ega_controller_config *config);
lib_i32 x86_video_ega_aperture_contains(const x86_video *adapter,
    lib_u32 physical, lib_size bytes);
lib_i32 x86_video_capture_text_snapshot_from(x86_video *adapter, const x86_video_memory_reader *memory,
    x86_video_snapshot *out_snapshot);
void x86_video_observe_snapshot(const x86_video *adapter,
    lib_u8 acknowledged_generation_valid,
    lib_u64 acknowledged_generation,
    x86_video_snapshot_observation *out_observation);
lib_i32 x86_video_capture_snapshot_from(x86_video *adapter, const x86_video_memory_reader *memory,
    x86_video_snapshot *out_snapshot);
lib_status x86_video_create(x86_video **out_video);
void x86_video_destroy(x86_video *video);
lib_status x86_video_memory_read(x86_video *video, x86_video_memory_region region,
    lib_u32 address, lib_u8 *destination, lib_size bytes);
lib_status x86_video_memory_write(x86_video *video, x86_video_memory_region region,
    lib_u32 address, const lib_u8 *source, lib_size bytes);
lib_status x86_video_memory_query(x86_video *video, x86_video_memory_region region,
    lib_u32 address, lib_size bytes, lib_bool write);
void x86_video_notify_memory_write(x86_video *video, lib_u32 address, lib_size bytes);
#endif
