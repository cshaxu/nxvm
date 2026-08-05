#ifndef VM_MACHINE_HDC_H
#define VM_MACHINE_HDC_H

#include "type.h"

#include "core/machine/block_interface.h"
#include "core/machine/pic.h"
#include "core/machine/port_interface.h"
#include "vm/machine/hdd.h"

typedef struct t_pic t_pic;

#define VM_MACHINE_HDC_STATUS_ERR 0x01u
#define VM_MACHINE_HDC_STATUS_DRQ 0x08u
#define VM_MACHINE_HDC_STATUS_DSC 0x10u
#define VM_MACHINE_HDC_STATUS_DRDY 0x40u
#define VM_MACHINE_HDC_STATUS_BSY 0x80u

#define VM_MACHINE_HDC_ERROR_ABORT 0x04u
#define VM_MACHINE_HDC_ERROR_ID_NOT_FOUND 0x10u

typedef enum vm_machine_hdc_phase {
    VM_MACHINE_HDC_PHASE_IDLE,
    VM_MACHINE_HDC_PHASE_DATA_READ,
    VM_MACHINE_HDC_PHASE_DATA_WRITE
} vm_machine_hdc_phase;

typedef struct vm_machine_hdc_config {
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
} vm_machine_hdc_config;

typedef struct vm_machine_hdc_data {
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
    uint8_t last_command;
    uint8_t sectors_remaining;
    uint32_t command_count;
    vm_machine_hdc_phase phase;
    uint16_t data_index;
    uint8_t data[512];
} vm_machine_hdc_data;

typedef struct vm_machine_hdc_connection {
    t_hdd *backend;
    core_machine_pic_irq_source irq_source;
    vm_machine_hdc_config config;
} vm_machine_hdc_connection;

typedef struct vm_machine_hdc {
    vm_machine_hdc_data data;
    vm_machine_hdc_connection connect;
} vm_machine_hdc;

C_VOID vm_machine_hdc_connect(vm_machine_hdc *hdc, t_hdd *backend,
    t_pic *pic_master, t_pic *pic_slave, const vm_machine_hdc_config *config);
C_VOID vm_machine_hdc_initialize(vm_machine_hdc *hdc);
C_VOID vm_machine_hdc_reset(vm_machine_hdc *hdc);
C_VOID vm_machine_hdc_refresh(vm_machine_hdc *hdc);
C_VOID vm_machine_hdc_finalize(vm_machine_hdc *hdc);
C_VOID vm_machine_hdc_get_geometry(const vm_machine_hdc *hdc,
    core_machine_block_geometry *out_geometry);
const core_machine_port_provider *vm_machine_hdc_port_provider(C_VOID);
type_bool vm_machine_hdc_irq_pending(const vm_machine_hdc *hdc);

#endif
