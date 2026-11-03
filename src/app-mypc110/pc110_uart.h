#ifndef MYPC110_UART_H
#define MYPC110_UART_H

#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-nxvm/chips/uart16550/uart16550_interface.h"

typedef struct mypc110_uart {
    x86_uart16550 *chip;
    x86_nxvm_board_pic_irq_source *irq;
} mypc110_uart;

lib_status mypc110_uart_create(mypc110_uart *uart);
void mypc110_uart_destroy(mypc110_uart *uart);
lib_status mypc110_uart_bind_irq(mypc110_uart *uart,
    x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave);
lib_status mypc110_uart_install_routes(x86_core_machine *machine, mypc110_uart *uart);
void mypc110_uart_reset(mypc110_uart *uart);
void mypc110_uart_advance(mypc110_uart *uart, lib_u64 ticks);

#endif
