#include "app-mypc110/pc110_uart.h"
#include "app-mypc110/pc110_clock.h"

#define MYPC110_UART_BASE 0x03f8u
#define MYPC110_UART_INPUT_CLOCK_HZ 1843200u

static void mypc110_uart_refresh_irq(mypc110_uart *uart)
{
    if (uart->irq == LIB_NULL) return;
    if (x86_uart16550_irq(uart->chip) && x86_uart16550_out2(uart->chip))
        x86_nxvm_board_pic_irq_source_assert(uart->irq);
    else x86_nxvm_board_pic_irq_source_deassert(uart->irq);
}

static lib_status mypc110_uart_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    mypc110_uart *uart = owner;
    (void)tick;
    if (uart == LIB_NULL || uart->chip == LIB_NULL || out_value == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = x86_uart16550_read(uart->chip, (lib_u8)(port - MYPC110_UART_BASE));
    mypc110_uart_refresh_irq(uart);
    return LIB_STATUS_OK;
}

static lib_status mypc110_uart_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    mypc110_uart *uart = owner;
    (void)access_bytes;
    if (uart == LIB_NULL || uart->chip == LIB_NULL || value > 0xffu)
        return LIB_STATUS_INVALID_ARGUMENT;
    x86_uart16550_write(uart->chip, (lib_u8)(port - MYPC110_UART_BASE), (lib_u8)value);
    mypc110_uart_refresh_irq(uart);
    return LIB_STATUS_OK;
}

lib_status mypc110_uart_create(mypc110_uart *uart)
{
    if (uart == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_uart16550_create(MYPC110_CPU_CLOCK_HZ,
        MYPC110_UART_INPUT_CLOCK_HZ, &uart->chip);
}

void mypc110_uart_destroy(mypc110_uart *uart)
{
    if (uart == LIB_NULL) return;
    x86_uart16550_destroy(uart->chip);
    uart->chip = LIB_NULL;
}

lib_status mypc110_uart_bind_irq(mypc110_uart *uart,
    x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave)
{
    if (uart == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_nxvm_board_pic_irq_source_bind(&uart->irq, master, slave, 4u);
}

lib_status mypc110_uart_install_routes(x86_core_machine *machine, mypc110_uart *uart)
{
    x86_core_machine_port_route routes[8];
    if (machine == LIB_NULL || uart == LIB_NULL || uart->chip == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_u8 i = 0u; i < 8u; ++i)
        routes[i] = (x86_core_machine_port_route){(lib_u16)(MYPC110_UART_BASE + i),
            mypc110_uart_read, mypc110_uart_write, uart, LIB_FALSE, 0u};
    return x86_core_machine_install_port_routes(machine, routes, 8u);
}

void mypc110_uart_reset(mypc110_uart *uart)
{
    if (uart == LIB_NULL) return;
    x86_uart16550_reset(uart->chip);
    mypc110_uart_refresh_irq(uart);
}

void mypc110_uart_advance(mypc110_uart *uart, lib_u64 ticks)
{
    if (uart == LIB_NULL) return;
    x86_uart16550_advance(uart->chip, ticks);
    mypc110_uart_refresh_irq(uart);
}
