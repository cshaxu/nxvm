#ifndef CORE_MACHINE_BOARD_TRACE_INTERFACE_H
#define CORE_MACHINE_BOARD_TRACE_INTERFACE_H

#include "x86-core/machine/trace_interface.h"

/* Board event identities share Core's copied scalar envelope and trace owner.
 * Preserve the existing diagnostic stream values and ordering. */
typedef enum core_machine_board_trace_event_type {
    CORE_MACHINE_BOARD_TRACE_PIT_ADVANCE = 9,
    CORE_MACHINE_BOARD_TRACE_PIC_REFRESH = 10,
    CORE_MACHINE_BOARD_TRACE_RTC_ADVANCE = 11,
    CORE_MACHINE_BOARD_TRACE_KBC_ADVANCE = 12,
    CORE_MACHINE_BOARD_TRACE_VADP_ADVANCE = 13,
    CORE_MACHINE_BOARD_TRACE_FDC_ADVANCE = 17,
    CORE_MACHINE_BOARD_TRACE_HDC_ADVANCE = 18
} core_machine_board_trace_event_type;

#endif
