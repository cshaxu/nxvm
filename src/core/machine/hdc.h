#ifndef CORE_MACHINE_HDC_H
#define CORE_MACHINE_HDC_H

#include "type.h"

#include "core/machine/controller_interface.h"
#include "core/machine/dma.h"
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
#define CORE_MACHINE_HDC_ERROR_DIAGNOSTIC_OK 0x01u

#define CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN 0x02u
#define CORE_MACHINE_HDC_DEVICE_CONTROL_SRST 0x04u

typedef enum core_machine_hdc_phase {
    CORE_MACHINE_HDC_PHASE_IDLE,
    CORE_MACHINE_HDC_PHASE_PENDING_COMMAND,
    CORE_MACHINE_HDC_PHASE_DATA_READ,
    CORE_MACHINE_HDC_PHASE_DATA_WRITE,
    CORE_MACHINE_HDC_PHASE_PENDING_READ_SECTOR,
    CORE_MACHINE_HDC_PHASE_PENDING_WRITE_SECTOR
} core_machine_hdc_phase;

typedef enum core_machine_xebec_phase {
    CORE_MACHINE_XEBEC_PHASE_IDLE,
    CORE_MACHINE_XEBEC_PHASE_DCB,
    CORE_MACHINE_XEBEC_PHASE_INITIALIZE,
    CORE_MACHINE_XEBEC_PHASE_PENDING_COMMAND,
    CORE_MACHINE_XEBEC_PHASE_RESPONSE,
    CORE_MACHINE_XEBEC_PHASE_DMA_READ,
    CORE_MACHINE_XEBEC_PHASE_DMA_WRITE
} core_machine_xebec_phase;

typedef struct core_machine_xebec_data {
    type_unsigned_8 dcb[6];
    type_unsigned_8 dcb_count;
    type_unsigned_8 initialize[8];
    type_unsigned_8 initialize_count;
    type_unsigned_8 response[5];
    type_unsigned_8 response_count;
    type_unsigned_8 response_index;
    type_unsigned_8 last_sense[4];
    type_unsigned_8 mask_pattern;
    type_unsigned_16 byte_index;
    type_unsigned_8 sectors_remaining;
    core_machine_xebec_phase phase;
} core_machine_xebec_data;

typedef struct core_machine_hdc_data {
    type_unsigned_8 features;
    type_unsigned_8 error;
    type_unsigned_8 sector_count;
    type_unsigned_8 sector_number;
    type_unsigned_8 cylinder_low;
    type_unsigned_8 cylinder_high;
    type_unsigned_8 drive_head;
    /* IBM's fixed-disk register at 3F6 supplies head bit 3; it is not ATA
     * device control and must not alter the task-file drive-select bit. */
    type_unsigned_8 fixed_disk_register;
    type_unsigned_8 step_rate_selector;
    type_unsigned_16 step_pulse_limit;
    type_unsigned_32 step_rate_ticks;
    type_unsigned_8 status;
    type_unsigned_8 device_control;
    type_bool irq_pending;
    type_bool reset_asserted;
    type_unsigned_8 last_command;
    type_unsigned_8 pending_command;
    type_unsigned_8 pending_features;
    type_unsigned_8 pending_sector_count;
    type_unsigned_8 pending_sector_number;
    type_unsigned_8 pending_cylinder_low;
    type_unsigned_8 pending_cylinder_high;
    type_unsigned_8 pending_drive_head;
    type_unsigned_16 sectors_remaining;
    type_unsigned_32 command_count;
    type_unsigned_64 elapsed_ticks;
    type_unsigned_64 next_service_tick;
    core_machine_hdc_phase phase;
    type_unsigned_16 data_index;
    type_unsigned_8 data[512];
} core_machine_hdc_data;

typedef struct core_machine_hdc_connection {
    const core_machine_media_registry *media_registry;
    core_machine_media_id media_id;
    core_machine_media_id slave_media_id;
    core_machine_pic_irq_source irq_source;
    core_machine_dma_request_binding dma_request;
    C_VOID (*dma_request_assert)(C_VOID *owner,
        const core_machine_dma_request_binding *binding);
    C_VOID (*dma_request_deassert)(C_VOID *owner,
        const core_machine_dma_request_binding *binding);
    C_VOID *dma_request_owner;
    core_machine_hdc_config config;
} core_machine_hdc_connection;

typedef struct core_machine_hdc {
    core_machine_hdc_data data;
    core_machine_xebec_data xebec;
    core_machine_hdc_connection connect;
} core_machine_hdc;

C_VOID core_machine_hdc_connect(core_machine_hdc *hdc,
    const core_machine_media_registry *media_registry,
    core_machine_media_id media_id, core_machine_media_id slave_media_id,
    t_pic *pic_master, t_pic *pic_slave, const core_machine_hdc_config *config);
C_VOID core_machine_hdc_bind_dma_request(core_machine_hdc *hdc,
    const core_machine_dma_request_binding *binding,
    C_VOID (*request_assert)(C_VOID *owner,
        const core_machine_dma_request_binding *binding),
    C_VOID (*request_deassert)(C_VOID *owner,
        const core_machine_dma_request_binding *binding), C_VOID *owner);
C_VOID core_machine_hdc_initialize(core_machine_hdc *hdc);
C_VOID core_machine_hdc_reset(core_machine_hdc *hdc);
/* Owner-local immediate service helper for direct controller clients.  The
 * production scheduler advances the same owner only through elapsed ticks. */
C_VOID core_machine_hdc_advance(core_machine_hdc *hdc);
C_VOID core_machine_hdc_advance_elapsed(core_machine_hdc *hdc,
    type_unsigned_64 elapsed_ticks);
type_status core_machine_hdc_next_due_tick(const core_machine_hdc *hdc,
    type_unsigned_64 *out_due_tick);
C_VOID core_machine_hdc_finalize(core_machine_hdc *hdc);
const core_machine_port_provider *core_machine_hdc_port_provider(C_VOID);
const core_machine_dma_channel_provider *core_machine_hdc_dma_provider(C_VOID);
type_bool core_machine_hdc_irq_pending(const core_machine_hdc *hdc);

#endif
