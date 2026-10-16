#ifndef X86_NXVM_BOARD_TIMING_INTERFACE_H
#define X86_NXVM_BOARD_TIMING_INTERFACE_H

#include "lib/types/types_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct x86_nxvm_board_state x86_nxvm_board_state;

#define X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT 30u

typedef enum x86_nxvm_board_timing_capability {
    X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXEC,
    X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXCEPT,
    X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_PREFETCH,
    X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_RETIRE,
    X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_FPU,
    X86_NXVM_BOARD_TIMING_CAPABILITY_TIME_CLOCK,
    X86_NXVM_BOARD_TIMING_CAPABILITY_TIME_LIFECYCLE,
    X86_NXVM_BOARD_TIMING_CAPABILITY_TXN_MEMORY,
    X86_NXVM_BOARD_TIMING_CAPABILITY_TXN_PORT,
    X86_NXVM_BOARD_TIMING_CAPABILITY_TXN_ARBITRATION,
    X86_NXVM_BOARD_TIMING_CAPABILITY_MEM_RAM_A20_PARITY,
    X86_NXVM_BOARD_TIMING_CAPABILITY_MEM_ROM_FIRMWARE,
    X86_NXVM_BOARD_TIMING_CAPABILITY_MACHINE_CONFIG,
    X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_PIC,
    X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_DMA,
    X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_PIT,
    X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_RTC_CMOS,
    X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_KBC_NMI,
    X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_FDC,
    X86_NXVM_BOARD_TIMING_CAPABILITY_CTRL_HDC,
    X86_NXVM_BOARD_TIMING_CAPABILITY_MEDIA_BACKING,
    X86_NXVM_BOARD_TIMING_CAPABILITY_DISPLAY_VADP,
    X86_NXVM_BOARD_TIMING_CAPABILITY_DISPLAY_PRESENT,
    X86_NXVM_BOARD_TIMING_CAPABILITY_INPUT_HOST,
    X86_NXVM_BOARD_TIMING_CAPABILITY_TRACE_DEBUG,
    X86_NXVM_BOARD_TIMING_CAPABILITY_PLATFORM_MAILBOX,
    X86_NXVM_BOARD_TIMING_CAPABILITY_PLATFORM_RESOURCE,
    X86_NXVM_BOARD_TIMING_CAPABILITY_PLATFORM_WAIT,
    X86_NXVM_BOARD_TIMING_CAPABILITY_SESSION_COMMAND,
    X86_NXVM_BOARD_TIMING_CAPABILITY_PRODUCT_DEBUG
} x86_nxvm_board_timing_capability;

typedef enum x86_nxvm_board_timing_disposition {
    X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK,
    X86_NXVM_BOARD_TIMING_DISPOSITION_NON_GUEST_TIME,
    X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED
} x86_nxvm_board_timing_disposition;

typedef enum x86_nxvm_board_timing_seam {
    X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM,
    X86_NXVM_BOARD_TIMING_SEAM_RETIREMENT,
    X86_NXVM_BOARD_TIMING_SEAM_CLOCK,
    X86_NXVM_BOARD_TIMING_SEAM_LIFECYCLE,
    X86_NXVM_BOARD_TIMING_SEAM_TRANSACTION,
    X86_NXVM_BOARD_TIMING_SEAM_MEMORY,
    X86_NXVM_BOARD_TIMING_SEAM_CONFIGURATION,
    X86_NXVM_BOARD_TIMING_SEAM_DEVICE,
    X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION
} x86_nxvm_board_timing_seam;

typedef struct x86_nxvm_board_timing_declaration {
    x86_nxvm_board_timing_capability capability;
    x86_nxvm_board_timing_disposition disposition;
    x86_nxvm_board_timing_seam seam;
} x86_nxvm_board_timing_declaration;

lib_status x86_nxvm_board_get_timing_disposition(const x86_nxvm_board_state *board,
    x86_nxvm_board_timing_capability capability,
    x86_nxvm_board_timing_disposition *out_disposition);
lib_status x86_nxvm_board_get_timing_declaration(const x86_nxvm_board_state *board,
    x86_nxvm_board_timing_capability capability,
    x86_nxvm_board_timing_declaration *out_declaration);

/* Validate a complete copied table; source provenance and seam policy remain
 * the board composition's responsibility. */
lib_status x86_nxvm_board_validate_timing_declarations(
    const x86_nxvm_board_timing_declaration *declarations, lib_size count);
/* Construction-only, once-only publication. Failure changes no live entry;
 * the caller's array is not retained. Reset preserves the frozen table. */
lib_status x86_nxvm_board_install_timing_declarations(x86_nxvm_board_state *board,
    const x86_nxvm_board_timing_declaration *declarations, lib_size count);

#ifdef __cplusplus
}
#endif

#endif
