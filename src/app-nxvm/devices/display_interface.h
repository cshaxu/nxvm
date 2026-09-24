/* Product-neutral display-mode notification and text snapshot contracts. */
#ifndef CORE_MACHINE_DISPLAY_INTERFACE_H
#define CORE_MACHINE_DISPLAY_INTERFACE_H
#include "lib/types/types_interface.h"



#define CORE_MACHINE_DISPLAY_MAX_COLUMNS 80u
#define CORE_MACHINE_DISPLAY_MAX_ROWS 25u
#define CORE_MACHINE_DISPLAY_GRAPHICS_WIDTH 320u
#define CORE_MACHINE_DISPLAY_CGA_HIGH_RES_WIDTH 640u
#define CORE_MACHINE_DISPLAY_GRAPHICS_HEIGHT 200u
#define CORE_MACHINE_DISPLAY_EGA_HIGH_RES_HEIGHT 350u
#define CORE_MACHINE_DISPLAY_MAX_PIXELS \
    (CORE_MACHINE_DISPLAY_CGA_HIGH_RES_WIDTH * \
        CORE_MACHINE_DISPLAY_EGA_HIGH_RES_HEIGHT)
#define CORE_MACHINE_DISPLAY_PALETTE_ENTRIES 256u
#define CORE_MACHINE_DISPLAY_EGA_GRAPHICS_REGISTER_COUNT 9u
#define CORE_MACHINE_DISPLAY_EGA_ATTRIBUTE_REGISTER_COUNT 21u
#define CORE_MACHINE_DISPLAY_TEXT_GLYPH_COUNT 256u
#define CORE_MACHINE_DISPLAY_TEXT_GLYPH_ROWS 16u
#define CORE_MACHINE_DISPLAY_TEXT_GLYPH_BYTES \
    (CORE_MACHINE_DISPLAY_TEXT_GLYPH_COUNT * CORE_MACHINE_DISPLAY_TEXT_GLYPH_ROWS)

typedef struct core_machine_vadp_text_timing {
    lib_u32 active_display_ticks;
    lib_u32 horizontal_blank_ticks;
    lib_u32 vertical_retrace_ticks;
} core_machine_vadp_text_timing;

/* A construction-time character generator is normalized by VM composition;
 * VADP thereafter owns the copied 8x16 glyph state exposed to presenters. */
typedef struct core_machine_vadp_text_glyph_config {
    lib_u8 present;
    lib_u8 bytes[CORE_MACHINE_DISPLAY_TEXT_GLYPH_BYTES];
} core_machine_vadp_text_glyph_config;

typedef struct core_machine_vadp_ega_sequencer_config {
    lib_u32 aperture_base;
    lib_u32 aperture_bytes;
    lib_u8 reset;
    lib_u8 clocking_mode;
    lib_u8 map_mask;
    lib_u8 memory_mode;
    lib_u8 planar_ega;
} core_machine_vadp_ega_sequencer_config;

typedef enum core_machine_vadp_ega_personality {
    CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC = 0,
    CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR
} core_machine_vadp_ega_personality;

/* Composition declares board-fixed CECG switch state; VADP owns the
 * resulting register state and reset behavior. */
typedef struct core_machine_vadp_cecg_config {
    lib_u8 control_mode;
    lib_u8 environment;
    lib_u8 display_type;
    lib_u8 initial_mode;
    lib_u8 lightpen_switch_open;
    lib_u8 cpu_video_memory_disabled;
    lib_u8 color_io_base;
    lib_u8 sw1_closed_mask;
    lib_u8 clock_switch_select;
    lib_u8 special_features_present;
    lib_u8 vertical_retrace_irq_enabled;
    lib_u8 odd_even_high_page;
} core_machine_vadp_cecg_config;

typedef struct core_machine_vadp_ega_controller_config {
    lib_u8 graphics[CORE_MACHINE_DISPLAY_EGA_GRAPHICS_REGISTER_COUNT];
    lib_u8 attribute[CORE_MACHINE_DISPLAY_EGA_ATTRIBUTE_REGISTER_COUNT];
} core_machine_vadp_ega_controller_config;

typedef enum core_machine_display_kind {
    CORE_MACHINE_DISPLAY_KIND_TEXT,
    CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4,
    CORE_MACHINE_DISPLAY_KIND_CGA_640X200X2,
    CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16,
    CORE_MACHINE_DISPLAY_KIND_EGA_640X200X16,
    CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16,
    CORE_MACHINE_DISPLAY_KIND_VGA_320X200X256
} core_machine_display_kind;

typedef void (*core_machine_display_provider)(void *context);

typedef struct core_machine_display_snapshot {
    core_machine_display_kind kind;
    lib_u16 columns;
    lib_u16 rows;
    /* Current CRTC character-cell scan-line count, not a font-asset default. */
    lib_u8 text_cell_height;
    lib_u8 cursor_top;
    lib_u8 cursor_bottom;
    /* Text coordinates are column then row, relative to display start. */
    lib_u8 cursor_x;
    lib_u8 cursor_y;
    lib_i32 cursor_visible;
    lib_i32 buffer_changed;
    lib_i32 cursor_changed;
    lib_u8 text_glyphs_present;
    lib_u8 text_glyphs[CORE_MACHINE_DISPLAY_TEXT_GLYPH_BYTES];
    lib_u8 characters[CORE_MACHINE_DISPLAY_MAX_COLUMNS * CORE_MACHINE_DISPLAY_MAX_ROWS];
    lib_u8 attributes[CORE_MACHINE_DISPLAY_MAX_COLUMNS * CORE_MACHINE_DISPLAY_MAX_ROWS];
    lib_u16 pixel_width;
    lib_u16 pixel_height;
    lib_u8 pixels[CORE_MACHINE_DISPLAY_MAX_PIXELS];
    lib_u32 palette_rgb[CORE_MACHINE_DISPLAY_PALETTE_ENTRIES];
} core_machine_display_snapshot;

/* A copied-frame consumer may acknowledge this opaque generation only after a
 * successful publish.  VADP offers it only when it owns and observes every
 * input to the selected frame (CGA/text VRAM, EGA planar, or VGA chain-4).
 * Other display paths retain normal capture. */
typedef struct core_machine_display_snapshot_observation {
    lib_u64 generation;
    lib_u8 generation_reliable;
    lib_u8 capture_required;
} core_machine_display_snapshot_observation;

typedef lib_i32 (*core_machine_display_snapshot_provider)(void *context,
    core_machine_display_snapshot *out_snapshot);

typedef struct core_machine_display_provider_slot core_machine_display_provider_slot;

lib_status core_machine_display_provider_slot_create(
    core_machine_display_provider_slot **out_slot);
void core_machine_display_provider_slot_bind(
    core_machine_display_provider_slot *slot, void *mode_context,
    core_machine_display_provider mode_provider, void *snapshot_context,
    core_machine_display_snapshot_provider snapshot_provider);
void core_machine_display_provider_slot_freeze(
    core_machine_display_provider_slot *slot);
void core_machine_display_provider_slot_destroy(
    core_machine_display_provider_slot *slot);
lib_i32 core_machine_display_capture_snapshot_from(
    const core_machine_display_provider_slot *slot,
    core_machine_display_snapshot *out_snapshot);

#endif
