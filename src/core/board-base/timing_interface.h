#ifndef CORE_MACHINE_BOARD_TIMING_INTERFACE_H
#define CORE_MACHINE_BOARD_TIMING_INTERFACE_H

#include "lib/types/types_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine_board_state core_machine_board_state;

#define CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT 30u

typedef enum core_machine_board_timing_capability {
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CPU_EXEC,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CPU_EXCEPT,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CPU_PREFETCH,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CPU_RETIRE,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CPU_FPU,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_TIME_CLOCK,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_TIME_LIFECYCLE,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_TXN_MEMORY,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_TXN_PORT,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_TXN_ARBITRATION,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_MEM_RAM_A20_PARITY,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_MEM_ROM_FIRMWARE,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_MACHINE_CONFIG,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CTRL_PIC,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CTRL_DMA,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CTRL_PIT,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CTRL_RTC_CMOS,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CTRL_KBC_NMI,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CTRL_FDC,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_CTRL_HDC,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_MEDIA_BACKING,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_DISPLAY_VADP,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_DISPLAY_PRESENT,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_INPUT_HOST,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_TRACE_DEBUG,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_PLATFORM_MAILBOX,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_PLATFORM_RESOURCE,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_PLATFORM_WAIT,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_SESSION_COMMAND,
    CORE_MACHINE_BOARD_TIMING_CAPABILITY_PRODUCT_DEBUG
} core_machine_board_timing_capability;

typedef enum core_machine_board_timing_disposition {
    CORE_MACHINE_BOARD_TIMING_DISPOSITION_L2_FALLBACK,
    CORE_MACHINE_BOARD_TIMING_DISPOSITION_NON_GUEST_TIME,
    CORE_MACHINE_BOARD_TIMING_DISPOSITION_L3_REQUIRED
} core_machine_board_timing_disposition;

typedef enum core_machine_board_timing_seam {
    CORE_MACHINE_BOARD_TIMING_SEAM_CPU_PROGRAM,
    CORE_MACHINE_BOARD_TIMING_SEAM_RETIREMENT,
    CORE_MACHINE_BOARD_TIMING_SEAM_CLOCK,
    CORE_MACHINE_BOARD_TIMING_SEAM_LIFECYCLE,
    CORE_MACHINE_BOARD_TIMING_SEAM_TRANSACTION,
    CORE_MACHINE_BOARD_TIMING_SEAM_MEMORY,
    CORE_MACHINE_BOARD_TIMING_SEAM_CONFIGURATION,
    CORE_MACHINE_BOARD_TIMING_SEAM_DEVICE,
    CORE_MACHINE_BOARD_TIMING_SEAM_OBSERVATION
} core_machine_board_timing_seam;

typedef struct core_machine_board_timing_declaration {
    core_machine_board_timing_capability capability;
    core_machine_board_timing_disposition disposition;
    core_machine_board_timing_seam seam;
} core_machine_board_timing_declaration;

lib_status core_machine_board_get_timing_disposition(const core_machine_board_state *board,
    core_machine_board_timing_capability capability,
    core_machine_board_timing_disposition *out_disposition);
lib_status core_machine_board_get_timing_declaration(const core_machine_board_state *board,
    core_machine_board_timing_capability capability,
    core_machine_board_timing_declaration *out_declaration);

/* Validate a complete copied table; source provenance and seam policy remain
 * the board composition's responsibility. */
lib_status core_machine_board_validate_timing_declarations(
    const core_machine_board_timing_declaration *declarations, lib_size count);
/* Construction-only, once-only publication. Failure changes no live entry;
 * the caller's array is not retained. Reset preserves the frozen table. */
lib_status core_machine_board_install_timing_declarations(core_machine_board_state *board,
    const core_machine_board_timing_declaration *declarations, lib_size count);

#ifdef __cplusplus
}
#endif

#endif
