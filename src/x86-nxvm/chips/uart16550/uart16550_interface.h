#ifndef X86_UART16550_INTERFACE_H
#define X86_UART16550_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct x86_uart16550 x86_uart16550;

/* The board owns address decode, IRQ gating and the single guest clock.
 * This bounded model has no connected external serial endpoint. */
lib_status x86_uart16550_create(lib_u32 ticks_per_second, lib_u32 input_clock_hz,
    x86_uart16550 **out_uart);
void x86_uart16550_destroy(x86_uart16550 *uart);
void x86_uart16550_reset(x86_uart16550 *uart);
lib_u8 x86_uart16550_read(x86_uart16550 *uart, lib_u8 offset);
void x86_uart16550_write(x86_uart16550 *uart, lib_u8 offset, lib_u8 value);
void x86_uart16550_advance(x86_uart16550 *uart, lib_u64 ticks);
lib_bool x86_uart16550_next_event(const x86_uart16550 *uart, lib_u64 *out_ticks);
lib_bool x86_uart16550_irq(const x86_uart16550 *uart);
lib_bool x86_uart16550_out2(const x86_uart16550 *uart);

#endif
