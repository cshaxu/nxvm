#ifndef X86_NXVM_BOARD_TRACE_INTERFACE_H
#define X86_NXVM_BOARD_TRACE_INTERFACE_H

#include "x86-core/machine/trace_interface.h"

/* Board event identities share Core's copied scalar envelope and trace owner.
 * Preserve the existing diagnostic stream values and ordering. */
typedef enum x86_nxvm_board_trace_event_type {
    X86_NXVM_BOARD_TRACE_PIT_ADVANCE = 9,
    X86_NXVM_BOARD_TRACE_PIC_REFRESH = 10,
    X86_NXVM_BOARD_TRACE_RTC_ADVANCE = 11,
    X86_NXVM_BOARD_TRACE_KBC_ADVANCE = 12,
    X86_NXVM_BOARD_TRACE_VADP_ADVANCE = 13,
    X86_NXVM_BOARD_TRACE_FDC_ADVANCE = 17,
    X86_NXVM_BOARD_TRACE_HDC_ADVANCE = 18
} x86_nxvm_board_trace_event_type;

#endif
