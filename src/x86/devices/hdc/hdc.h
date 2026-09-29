#ifndef X86_HDC_H
#define X86_HDC_H
#include "x86/devices/hdc/hdc_interface.h"


typedef struct x86_xebec_data {
    lib_u8 dcb[6];
    lib_u8 dcb_count;
    lib_u8 initialize[8];
    lib_u8 initialize_count;
    lib_u8 response[5];
    lib_u8 response_count;
    lib_u8 response_index;
    lib_u8 last_sense[4];
    lib_u8 mask_pattern;
    lib_u16 byte_index;
    lib_u8 sectors_remaining;
    x86_xebec_phase phase;
} x86_xebec_data;

typedef struct x86_hdc_data {
    lib_u8 features;
    lib_u8 error;
    lib_u8 sector_count;
    lib_u8 sector_number;
    lib_u8 cylinder_low;
    lib_u8 cylinder_high;
    lib_u8 drive_head;
    /* WD head extension is distinct from ATA device control. */
    lib_u8 fixed_disk_register;
    lib_u8 step_rate_selector;
    lib_u16 step_pulse_limit;
    lib_u32 step_rate_ticks;
    lib_u8 status;
    lib_u8 device_control;
    lib_u8 irq_pending;
    lib_u8 reset_asserted;
    lib_u8 last_command;
    lib_u8 pending_command;
    lib_u8 pending_features;
    lib_u8 pending_sector_count;
    lib_u8 pending_sector_number;
    lib_u8 pending_cylinder_low;
    lib_u8 pending_cylinder_high;
    lib_u8 pending_drive_head;
    lib_u16 sectors_remaining;
    lib_u32 command_count;
    lib_u64 elapsed_ticks;
    lib_u64 next_service_tick;
    x86_hdc_phase phase;
    lib_u16 data_index;
    lib_u8 data[512];
} x86_hdc_data;

struct x86_hdc {
    x86_hdc_data data;
    x86_xebec_data xebec;
    x86_hdc_config config;
    x86_hdc_connection connect;
};
#endif
