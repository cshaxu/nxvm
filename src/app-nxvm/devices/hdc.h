#ifndef CORE_MACHINE_HDC_H
#define CORE_MACHINE_HDC_H
#include "lib/types/types_interface.h"


#include "app-nxvm/devices/controller_interface.h"
#include "app-nxvm/devices/dma.h"
#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/port_interface.h"

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
    core_machine_xebec_phase phase;
} core_machine_xebec_data;

typedef struct core_machine_hdc_data {
    lib_u8 features;
    lib_u8 error;
    lib_u8 sector_count;
    lib_u8 sector_number;
    lib_u8 cylinder_low;
    lib_u8 cylinder_high;
    lib_u8 drive_head;
    /* IBM's fixed-disk register at 3F6 supplies head bit 3; it is not ATA
     * device control and must not alter the task-file drive-select bit. */
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
    core_machine_hdc_phase phase;
    lib_u16 data_index;
    lib_u8 data[512];
} core_machine_hdc_data;

typedef struct core_machine_hdc_connection {
    const core_machine_media_registry *media_registry;
    core_machine_media_id media_id;
    core_machine_media_id slave_media_id;
    core_machine_pic_irq_source irq_source;
    core_machine_dma_request_binding dma_request;
    void (*dma_request_assert)(void *owner,
        const core_machine_dma_request_binding *binding);
    void (*dma_request_deassert)(void *owner,
        const core_machine_dma_request_binding *binding);
    void *dma_request_owner;
    core_machine_hdc_config config;
} core_machine_hdc_connection;

typedef struct core_machine_hdc {
    core_machine_hdc_data data;
    core_machine_xebec_data xebec;
    core_machine_hdc_connection connect;
} core_machine_hdc;

void core_machine_hdc_connect(core_machine_hdc *hdc,
    const core_machine_media_registry *media_registry,
    core_machine_media_id media_id, core_machine_media_id slave_media_id,
    t_pic *pic_master, t_pic *pic_slave, const core_machine_hdc_config *config);
void core_machine_hdc_bind_dma_request(core_machine_hdc *hdc,
    const core_machine_dma_request_binding *binding,
    void (*request_assert)(void *owner,
        const core_machine_dma_request_binding *binding),
    void (*request_deassert)(void *owner,
        const core_machine_dma_request_binding *binding), void *owner);
void core_machine_hdc_initialize(core_machine_hdc *hdc);
void core_machine_hdc_reset(core_machine_hdc *hdc);
/* Owner-local immediate service helper for direct controller clients.  The
 * production scheduler advances the same owner only through elapsed ticks. */
void core_machine_hdc_advance(core_machine_hdc *hdc);
void core_machine_hdc_advance_elapsed(core_machine_hdc *hdc,
    lib_u64 elapsed_ticks);
lib_status core_machine_hdc_next_due_tick(const core_machine_hdc *hdc,
    lib_u64 *out_due_tick);
void core_machine_hdc_finalize(core_machine_hdc *hdc);
const core_machine_port_provider *core_machine_hdc_port_provider(void);
const core_machine_dma_channel_provider *core_machine_hdc_dma_provider(void);
lib_u8 core_machine_hdc_irq_pending(const core_machine_hdc *hdc);

#endif
