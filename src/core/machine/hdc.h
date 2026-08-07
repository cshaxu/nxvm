#ifndef CORE_MACHINE_HDC_H
#define CORE_MACHINE_HDC_H

#include "type.h"

#include "core/machine/media_interface.h"
#include "core/machine/pic.h"
#include "core/machine/port_interface.h"

typedef struct t_pic t_pic;

#define CORE_MACHINE_HDC_STATUS_ERR 0x01u
#define CORE_MACHINE_HDC_STATUS_DRQ 0x08u
#define CORE_MACHINE_HDC_STATUS_DSC 0x10u
#define CORE_MACHINE_HDC_STATUS_DRDY 0x40u
#define CORE_MACHINE_HDC_STATUS_BSY 0x80u

#define CORE_MACHINE_HDC_ERROR_ABORT 0x04u
#define CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND 0x10u

typedef enum core_machine_hdc_phase {
    CORE_MACHINE_HDC_PHASE_IDLE,
    CORE_MACHINE_HDC_PHASE_DATA_READ,
    CORE_MACHINE_HDC_PHASE_DATA_WRITE
} core_machine_hdc_phase;

typedef struct core_machine_hdc_config {
    uint16_t data_port;
    uint16_t error_features_port;
    uint16_t sector_count_port;
    uint16_t sector_number_port;
    uint16_t cylinder_low_port;
    uint16_t cylinder_high_port;
    uint16_t drive_head_port;
    uint16_t status_command_port;
    uint16_t alternate_status_device_control_port;
    uint8_t irq;
    type_bool lba28_supported;
} core_machine_hdc_config;

typedef struct core_machine_hdc_data {
    uint8_t features;
    uint8_t error;
    uint8_t sector_count;
    uint8_t sector_number;
    uint8_t cylinder_low;
    uint8_t cylinder_high;
    uint8_t drive_head;
    uint8_t status;
    uint8_t device_control;
    type_bool irq_pending;
    type_bool reset_asserted;
    uint8_t last_command;
    uint16_t sectors_remaining;
    uint32_t command_count;
    core_machine_hdc_phase phase;
    uint16_t data_index;
    uint8_t data[512];
} core_machine_hdc_data;

typedef struct core_machine_hdc_connection {
    const core_machine_media_registry *media_registry;
    core_machine_media_id media_id;
    core_machine_pic_irq_source irq_source;
    core_machine_hdc_config config;
} core_machine_hdc_connection;

typedef struct core_machine_hdc {
    core_machine_hdc_data data;
    core_machine_hdc_connection connect;
} core_machine_hdc;

C_VOID core_machine_hdc_connect(core_machine_hdc *hdc,
    const core_machine_media_registry *media_registry,
    core_machine_media_id media_id,
    t_pic *pic_master, t_pic *pic_slave, const core_machine_hdc_config *config);
C_VOID core_machine_hdc_initialize(core_machine_hdc *hdc);
C_VOID core_machine_hdc_reset(core_machine_hdc *hdc);
C_VOID core_machine_hdc_refresh(core_machine_hdc *hdc);
C_VOID core_machine_hdc_finalize(core_machine_hdc *hdc);
const core_machine_port_provider *core_machine_hdc_port_provider(C_VOID);
type_bool core_machine_hdc_irq_pending(const core_machine_hdc *hdc);

#endif
