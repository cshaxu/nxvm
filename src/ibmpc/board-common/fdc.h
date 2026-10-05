/* Copyright 2012-2026 Neko. */
#ifndef CORE_MACHINE_FDC_H
#define CORE_MACHINE_FDC_H
#include "ibmpc/board-common/fdc_interface.h"
#include "ibmpc/board-common/dma_bus_interface.h"
#include "ibmpc/board-common/pic_bus_interface.h"
#include "ibmpc/board-common/fdc_observation_interface.h"
#include "x86/core/port_interface.h"
#include "x86/chips/fdc8272/fdc8272_interface.h"

#define CORE_MACHINE_DEVICE_FDC "Intel 8272A"

typedef struct {
    const core_machine_media_registry *media_registry;
    core_machine_fdc_drive_bindings drives;
    core_machine_dma_request_binding dma_request;
    core_machine_fdc_dma_request_operation dma_request_assert;
    core_machine_fdc_dma_request_operation dma_request_deassert;
    void *dma_request_owner;
    core_machine_pic_irq_source *irq_source;
    core_machine *machine;
    core_machine_fdc_config config;
    core_machine_fdc_terminal_observation_provider observation_provider;
    lib_u64 observation_sequence;
} core_machine_fdc_connection;


typedef struct {
    lib_u8 dor, dir, ccr;
    lib_u64 observed_media_generation[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_bool media_changed[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_bool initial_media_baseline_pending;
} core_machine_fdc_board_data;

struct core_machine_fdc {
    x86_fdc *chip;
    core_machine_fdc_board_data data;
    core_machine_fdc_connection connect;
    lib_u16 drive_cylinder[CORE_MACHINE_FDC_DRIVE_COUNT];
};

/* digital input register bits */
#define VFDC_DIR_HD 0x01 /* high density select */
#define VFDC_DIR_DC 0x80 /* diskette change */

/* digital output register bits */
#define VFDC_DOR_ME(id) (1 << ((id) + 4)) /* motor engine enable */
#define VFDC_DOR_DS   0x03 /* drive select */
#define VFDC_DOR_NRS  0x04 /* fdc enable(1) or hold(0) fdc at reset */
#define VFDC_DOR_ENRQ 0x08 /* dma and i/o interface enabled */

/* Configuration control register data-rate select.  The 8272A-compatible
 * PC/AT route uses 500, 300 and 250 kbps; 1 Mbps is not admitted here. */
#define VFDC_CCR_RATE_500 0x00u
#define VFDC_CCR_RATE_300 0x01u
#define VFDC_CCR_RATE_250 0x02u
#define VFDC_CCR_RATE_MASK 0x03u


lib_status core_machine_fdc_connect(core_machine_fdc *fdc,
    const core_machine_media_registry *media_registry,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_dma_request_binding *dma_request,
    core_machine_fdc_dma_request_operation dma_request_assert,
    core_machine_fdc_dma_request_operation dma_request_deassert,
    void *dma_request_owner, core_machine_pic_bus *pic_master, core_machine_pic_bus *pic_slave,
    core_machine *machine, const core_machine_fdc_config *config,
    const core_machine_fdc_terminal_observation_provider *observation_provider);
const core_machine_dma_channel_provider *core_machine_fdc_dma_provider(void);
lib_status core_machine_fdc_initialize(core_machine_fdc *fdc);
void core_machine_fdc_reset(core_machine_fdc *fdc);
void core_machine_fdc_advance_at(core_machine_fdc *fdc,
    lib_u64 elapsed_ticks);
lib_status core_machine_fdc_next_due_tick(const core_machine_fdc *fdc,
    lib_u64 *out_due_tick);
void core_machine_fdc_refresh(core_machine_fdc *fdc);
void core_machine_fdc_finalize(core_machine_fdc *fdc);


#endif
