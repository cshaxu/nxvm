#ifndef CORE_MACHINE_INTERFACE_H
#define CORE_MACHINE_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct core_machine core_machine;

#define CORE_MACHINE_TRACE_CAPACITY 16u
#define CORE_MACHINE_VIDEO_WIDTH 256u
#define CORE_MACHINE_VIDEO_HEIGHT 240u
#define CORE_MACHINE_VIDEO_SAMPLE_CAPACITY (CORE_MACHINE_VIDEO_WIDTH * CORE_MACHINE_VIDEO_HEIGHT)
#define CORE_MACHINE_BATTERY_BYTE_CAPACITY 8192u

typedef enum core_bus_transfer_kind {
    CORE_BUS_TRANSFER_READ,
    CORE_BUS_TRANSFER_WRITE
} core_bus_transfer_kind;

typedef struct core_bus_transfer {
    lib_u16 address;
    lib_u8 value;
    core_bus_transfer_kind kind;
} core_bus_transfer;

typedef enum core_reset_kind {
    CORE_RESET_POWER,
    CORE_RESET_WARM
} core_reset_kind;

typedef enum core_machine_stop_reason {
    CORE_MACHINE_STOP_BUDGET,
    CORE_MACHINE_STOP_BREAKPOINT,
    CORE_MACHINE_STOP_UNSUPPORTED_OPCODE,
    CORE_MACHINE_STOP_UNSUPPORTED_DEVICE,
    CORE_MACHINE_STOP_BUS_FAILURE
} core_machine_stop_reason;

typedef struct core_machine_options {
    lib_u8 initial_ram_byte;
} core_machine_options;

typedef struct core_observation {
    lib_u8 a;
    lib_u8 x;
    lib_u8 y;
    lib_u8 s;
    lib_u8 p;
    lib_u16 pc;
    lib_u64 cycles;
    lib_u64 instructions;
    lib_bool cartridge_present;
    lib_bool trap_valid;
    core_machine_stop_reason trap_reason;
    lib_u16 trap_pc;
    lib_u16 trap_address;
    lib_u8 trap_opcode;
} core_observation;

typedef struct core_run_result {
    lib_u32 instructions;
    lib_u32 cycles;
    core_machine_stop_reason reason;
    lib_bool trap_valid;
} core_run_result;

/* A copied machine-facing video result.  The Machine adapter owns host-frame
 * conversion; Core only exposes the completed PPU samples and their revision. */
typedef struct core_machine_video {
    lib_u32 revision;
    lib_bool ready;
} core_machine_video;

/* Battery persistence is a Core-owned cartridge image.  The Machine adapter
 * owns its file format and I/O, while Core validates and commits this value. */
typedef struct core_machine_battery {
    lib_u8 mapper;
    lib_u64 identity;
    lib_size byte_count;
    lib_bool dirty;
    lib_u8 bytes[CORE_MACHINE_BATTERY_BYTE_CAPACITY];
} core_machine_battery;

lib_status core_machine_create(core_machine **out_machine,
    const lib_u8 *rom, lib_size rom_size, const core_machine_options *options);
lib_size core_machine_image_maximum_bytes(void);
lib_bool core_machine_image_normalize_size(const lib_u8 *bytes,
    lib_size *in_out_byte_count);
void core_machine_destroy(core_machine *machine);
lib_status core_machine_reset(core_machine *machine, core_reset_kind kind);
lib_status core_machine_set_interrupt_lines(core_machine *machine,
    lib_bool irq_asserted, lib_bool nmi_asserted);
lib_status core_machine_set_ppu_nmi_line(core_machine *machine, lib_bool asserted);
lib_status core_machine_run(core_machine *machine, lib_u32 instruction_limit,
    lib_u32 cycle_limit, core_run_result *out_result);
lib_status core_machine_debug_step(core_machine *machine, lib_u32 instruction_limit,
    lib_u32 cycle_limit, core_run_result *out_result);
lib_status core_machine_observe(const core_machine *machine, core_observation *out);
lib_status core_machine_peek(const core_machine *machine, lib_u16 address,
    lib_u32 count, lib_u8 *out_bytes);
lib_status core_machine_poke(core_machine *machine, lib_u16 address,
    const lib_u8 *bytes, lib_u32 count);
lib_status core_machine_trace_copy(const core_machine *machine,
    core_bus_transfer *out_transfers, lib_u32 capacity, lib_u32 *out_count);
lib_status core_machine_breakpoint_set(core_machine *machine, lib_u16 address,
    lib_bool enabled);
lib_status core_machine_breakpoint_list(const core_machine *machine,
    lib_u16 *out_addresses, lib_u32 capacity, lib_u32 *out_count);
lib_status core_machine_take_audio(core_machine *machine, lib_i32 *out_samples,
    lib_u16 capacity, lib_u16 *out_count);
lib_status core_machine_copy_video(const core_machine *machine, lib_u16 *out_samples,
    lib_size capacity, core_machine_video *out_video);
lib_status core_machine_set_buttons(core_machine *machine, lib_u8 buttons);
lib_status core_machine_tap_buttons(core_machine *machine, lib_u8 buttons);
lib_status core_machine_clear_transient_buttons(core_machine *machine);
lib_status core_machine_battery_capture(const core_machine *machine,
    core_machine_battery *out_battery);
lib_status core_machine_battery_restore(core_machine *machine,
    const core_machine_battery *battery);
lib_status core_machine_battery_mark_clean(core_machine *machine);

#endif
