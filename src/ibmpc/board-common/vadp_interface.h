#ifndef CORE_MACHINE_VADP_INTERFACE_H
#define CORE_MACHINE_VADP_INTERFACE_H
#include "x86/chips/video/video_interface.h"
#include "x86/core/machine_interface.h"

typedef struct t_vadp t_vadp;
/* Standard EGA planar aperture used by frozen board declarations. */
#define CORE_MACHINE_VADP_EGA_APERTURE_BASE 0x000a0000u
#define CORE_MACHINE_VADP_EGA_APERTURE_BYTES 0x00010000u
typedef struct core_machine_display_port_topology {
    lib_u16 attribute_first;
    lib_u16 attribute_last;
    lib_u16 sequencer_first;
    lib_u16 sequencer_last;
    lib_u16 graphics_first;
    lib_u16 graphics_last;
    lib_u16 crtc_first;
    lib_u16 crtc_last;
} core_machine_display_port_topology;

typedef struct core_machine_display_config {
    x86_video_text_timing text_timing;
    x86_video_text_glyph_config text_glyphs;
    lib_u8 cga_vram_present;
    lib_u8 ega_present;
    x86_video_ega_personality ega_personality;
    x86_video_cecg_config cecg;
    x86_video_ega_sequencer_config ega_sequencer;
    x86_video_ega_controller_config ega_controllers;
    core_machine_display_port_topology ports;
    lib_bool vga_present;
} core_machine_display_config;

/* Serialized board adapter owns its video chip and Core routes. Configure
 * preserves the previous chip/routes on failure. Core outlives this handle;
 * destroy only after dispatch stops. Captures return copied values, never a
 * chip or framebuffer pointer. */
lib_status core_machine_vadp_create(core_machine *machine, t_vadp **out_adapter);
/* Validate the complete board declaration before its routes are admitted. */
lib_bool core_machine_vadp_config_is_valid(const core_machine_display_config *config);
void core_machine_vadp_destroy(t_vadp *adapter);
lib_status core_machine_vadp_configure(t_vadp *adapter,
    const core_machine_display_config *config);
void core_machine_vadp_reset(t_vadp *adapter);
void core_machine_vadp_advance(t_vadp *adapter, lib_u64 ticks);
lib_status core_machine_vadp_observe_bus(const t_vadp *adapter,
    x86_video_bus_observation *out_observation);
lib_i32 core_machine_vadp_capture_snapshot(t_vadp *adapter,
    x86_video_snapshot *out_snapshot);
void core_machine_vadp_observe_snapshot(const t_vadp *adapter,
    lib_u8 acknowledged_generation_valid, lib_u64 acknowledged_generation,
    x86_video_snapshot_observation *out_observation);
#endif
