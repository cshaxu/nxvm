/* Product-neutral display-mode notification and text snapshot contracts. */
#ifndef CORE_MACHINE_DISPLAY_INTERFACE_H
#define CORE_MACHINE_DISPLAY_INTERFACE_H

#include "type.h"


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

typedef struct core_machine_vadp_text_timing {
    type_unsigned_32 active_display_ticks;
    type_unsigned_32 horizontal_blank_ticks;
    type_unsigned_32 vertical_retrace_ticks;
} core_machine_vadp_text_timing;

typedef struct core_machine_vadp_ega_sequencer_config {
    type_unsigned_32 aperture_base;
    type_unsigned_32 aperture_bytes;
    type_unsigned_8 reset;
    type_unsigned_8 clocking_mode;
    type_unsigned_8 map_mask;
    type_unsigned_8 memory_mode;
    type_bool planar_ega;
} core_machine_vadp_ega_sequencer_config;

typedef enum core_machine_vadp_ega_personality {
    CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC = 0,
    CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR
} core_machine_vadp_ega_personality;

/* Composition declares board-fixed CECG switch state; VADP owns the
 * resulting register state and reset behavior. */
typedef struct core_machine_vadp_cecg_config {
    type_unsigned_8 control_mode;
    type_unsigned_8 environment;
    type_unsigned_8 display_type;
    type_unsigned_8 initial_mode;
    type_bool lightpen_switch_open;
    type_bool cpu_video_memory_disabled;
    type_bool color_io_base;
    type_unsigned_8 sw1_closed_mask;
    type_unsigned_8 clock_switch_select;
    type_bool special_features_present;
    type_bool vertical_retrace_irq_enabled;
    type_bool odd_even_high_page;
} core_machine_vadp_cecg_config;

typedef struct core_machine_vadp_ega_controller_config {
    type_unsigned_8 graphics[CORE_MACHINE_DISPLAY_EGA_GRAPHICS_REGISTER_COUNT];
    type_unsigned_8 attribute[CORE_MACHINE_DISPLAY_EGA_ATTRIBUTE_REGISTER_COUNT];
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

typedef C_VOID (*core_machine_display_provider)(C_VOID *context);

typedef struct core_machine_display_snapshot {
    core_machine_display_kind kind;
    type_unsigned_16 columns;
    type_unsigned_16 rows;
    type_unsigned_8 cursor_top;
    type_unsigned_8 cursor_bottom;
    /* Text coordinates are column then row, relative to display start. */
    type_unsigned_8 cursor_x;
    type_unsigned_8 cursor_y;
    C_INT cursor_visible;
    C_INT buffer_changed;
    C_INT cursor_changed;
    type_unsigned_8 characters[CORE_MACHINE_DISPLAY_MAX_COLUMNS * CORE_MACHINE_DISPLAY_MAX_ROWS];
    type_unsigned_8 attributes[CORE_MACHINE_DISPLAY_MAX_COLUMNS * CORE_MACHINE_DISPLAY_MAX_ROWS];
    type_unsigned_16 pixel_width;
    type_unsigned_16 pixel_height;
    type_unsigned_8 pixels[CORE_MACHINE_DISPLAY_MAX_PIXELS];
    type_unsigned_32 palette_rgb[CORE_MACHINE_DISPLAY_PALETTE_ENTRIES];
} core_machine_display_snapshot;

/* A copied-frame consumer may acknowledge this opaque generation only after a
 * successful publish.  VADP offers it only when it owns and observes every
 * input to the selected frame (CGA/text VRAM, EGA planar, or VGA chain-4).
 * Other display paths retain normal capture. */
typedef struct core_machine_display_snapshot_observation {
    type_unsigned_64 generation;
    type_bool generation_reliable;
    type_bool capture_required;
} core_machine_display_snapshot_observation;

typedef C_INT (*core_machine_display_snapshot_provider)(C_VOID *context,
    core_machine_display_snapshot *out_snapshot);

typedef struct core_machine_display_provider_slot core_machine_display_provider_slot;

type_status core_machine_display_provider_slot_create(
    core_machine_display_provider_slot **out_slot);
C_VOID core_machine_display_provider_slot_bind(
    core_machine_display_provider_slot *slot, C_VOID *mode_context,
    core_machine_display_provider mode_provider, C_VOID *snapshot_context,
    core_machine_display_snapshot_provider snapshot_provider);
C_VOID core_machine_display_provider_slot_freeze(
    core_machine_display_provider_slot *slot);
C_VOID core_machine_display_provider_slot_destroy(
    core_machine_display_provider_slot *slot);
C_VOID core_machine_display_notify_mode_changed_to(
    const core_machine_display_provider_slot *slot);
C_INT core_machine_display_capture_snapshot_from(
    const core_machine_display_provider_slot *slot,
    core_machine_display_snapshot *out_snapshot);

#endif
