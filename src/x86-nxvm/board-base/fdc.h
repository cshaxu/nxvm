/* Copyright 2012-2026 Neko. */
#ifndef X86_NXVM_BOARD_FDC_H
#define X86_NXVM_BOARD_FDC_H
#include "x86-nxvm/board-base/fdc_interface.h"
#include "x86-nxvm/board-base/dma_bus_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-nxvm/board-base/fdc_observation_interface.h"
#include "x86-core/machine/port_interface.h"
#include "x86-nxvm/chips/fdc8272/fdc8272_interface.h"

#define X86_NXVM_BOARD_DEVICE_FDC "Intel 8272A"

typedef struct {
    const x86_nxvm_board_media_registry *media_registry;
    x86_nxvm_board_fdc_drive_bindings drives;
    x86_nxvm_board_dma_request_binding dma_request;
    x86_nxvm_board_fdc_dma_request_operation dma_request_assert;
    x86_nxvm_board_fdc_dma_request_operation dma_request_deassert;
    void *dma_request_owner;
    x86_nxvm_board_pic_irq_source *irq_source;
    x86_core_machine *machine;
    x86_nxvm_board_fdc_config config;
    x86_nxvm_board_fdc_terminal_observation_provider observation_provider;
    lib_u64 observation_sequence;
} x86_nxvm_board_fdc_connection;


typedef struct {
    lib_u8 dor, dir, ccr;
    lib_u64 observed_media_generation[X86_NXVM_BOARD_FDC_DRIVE_COUNT];
    lib_bool media_changed[X86_NXVM_BOARD_FDC_DRIVE_COUNT];
    lib_bool initial_media_baseline_pending;
} x86_nxvm_board_fdc_board_data;

struct x86_nxvm_board_fdc {
    x86_fdc *chip;
    x86_nxvm_board_fdc_board_data data;
    x86_nxvm_board_fdc_connection connect;
    lib_u16 drive_cylinder[X86_NXVM_BOARD_FDC_DRIVE_COUNT];
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


lib_status x86_nxvm_board_fdc_connect(x86_nxvm_board_fdc *fdc,
    const x86_nxvm_board_media_registry *media_registry,
    const x86_nxvm_board_fdc_drive_bindings *drives,
    const x86_nxvm_board_dma_request_binding *dma_request,
    x86_nxvm_board_fdc_dma_request_operation dma_request_assert,
    x86_nxvm_board_fdc_dma_request_operation dma_request_deassert,
    void *dma_request_owner, x86_nxvm_board_pic_bus *pic_master, x86_nxvm_board_pic_bus *pic_slave,
    x86_core_machine *machine, const x86_nxvm_board_fdc_config *config,
    const x86_nxvm_board_fdc_terminal_observation_provider *observation_provider);
const x86_nxvm_board_dma_channel_provider *x86_nxvm_board_fdc_dma_provider(void);
lib_status x86_nxvm_board_fdc_initialize(x86_nxvm_board_fdc *fdc);
void x86_nxvm_board_fdc_reset(x86_nxvm_board_fdc *fdc);
void x86_nxvm_board_fdc_advance_at(x86_nxvm_board_fdc *fdc,
    lib_u64 elapsed_ticks);
lib_status x86_nxvm_board_fdc_next_due_tick(const x86_nxvm_board_fdc *fdc,
    lib_u64 *out_due_tick);
void x86_nxvm_board_fdc_refresh(x86_nxvm_board_fdc *fdc);
void x86_nxvm_board_fdc_finalize(x86_nxvm_board_fdc *fdc);


#endif
