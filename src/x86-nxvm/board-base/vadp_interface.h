#ifndef X86_NXVM_BOARD_VADP_INTERFACE_H
#define X86_NXVM_BOARD_VADP_INTERFACE_H
#include "x86-nxvm/chips/video/video_interface.h"
#include "x86-core/machine/machine_interface.h"

typedef struct t_vadp t_vadp;
/* Standard EGA planar aperture used by frozen board declarations. */
#define X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE 0x000a0000u
#define X86_NXVM_BOARD_VADP_EGA_APERTURE_BYTES 0x00010000u
typedef struct x86_nxvm_board_display_port_topology {
    lib_u16 attribute_first;
    lib_u16 attribute_last;
    lib_u16 sequencer_first;
    lib_u16 sequencer_last;
    lib_u16 graphics_first;
    lib_u16 graphics_last;
    lib_u16 crtc_first;
    lib_u16 crtc_last;
} x86_nxvm_board_display_port_topology;

typedef struct x86_nxvm_board_display_config {
    x86_video_text_timing text_timing;
    x86_video_text_glyph_config text_glyphs;
    lib_u8 cga_vram_present;
    lib_u8 ega_present;
    x86_video_ega_personality ega_personality;
    x86_video_cecg_config cecg;
    x86_video_ega_sequencer_config ega_sequencer;
    x86_video_ega_controller_config ega_controllers;
    x86_nxvm_board_display_port_topology ports;
    lib_bool vga_present;
    lib_u8 vertical_irq; /* Zero is unconnected; board selects the PIC route. */
} x86_nxvm_board_display_config;

/* Serialized board adapter owns its video chip and Core routes. Configure
 * preserves the previous chip/routes on failure. Core outlives this handle;
 * destroy only after dispatch stops. Captures return copied values, never a
 * chip or framebuffer pointer. */
lib_status x86_nxvm_board_vadp_create(x86_core_machine *machine, t_vadp **out_adapter);
/* Validate the complete board declaration before its routes are admitted. */
lib_bool x86_nxvm_board_vadp_config_is_valid(const x86_nxvm_board_display_config *config);
void x86_nxvm_board_vadp_destroy(t_vadp *adapter);
lib_status x86_nxvm_board_vadp_configure(t_vadp *adapter,
    const x86_nxvm_board_display_config *config);
void x86_nxvm_board_vadp_reset(t_vadp *adapter);
void x86_nxvm_board_vadp_advance(t_vadp *adapter, lib_u64 ticks);
lib_status x86_nxvm_board_vadp_observe_bus(const t_vadp *adapter,
    x86_video_bus_observation *out_observation);
lib_status x86_nxvm_board_vadp_capture_snapshot(t_vadp *adapter,
    x86_video_snapshot *out_snapshot);
void x86_nxvm_board_vadp_observe_snapshot(const t_vadp *adapter,
    lib_u8 acknowledged_generation_valid, lib_u64 acknowledged_generation,
    x86_video_snapshot_observation *out_observation);
#endif
