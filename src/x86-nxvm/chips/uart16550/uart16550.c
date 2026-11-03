#include "x86-nxvm/chips/uart16550/uart16550_interface.h"

#define UART_FIFO_BYTES 16u
struct x86_uart16550 {
    lib_u32 ticks_per_second;
    lib_u32 input_clock_hz;
    lib_u16 divisor;
    lib_u8 ier, lcr, mcr, scr, fcr, lsr_error, msr_delta, modem;
    lib_u8 rx[UART_FIFO_BYTES], tx[UART_FIFO_BYTES];
    lib_u8 rx_head, rx_count, tx_head, tx_count, transmitting;
    lib_bool shift_busy, thre_pending, receive_timeout;
    lib_u64 tx_ticks, rx_ticks;
};

static lib_u8 uart_capacity(const x86_uart16550 *uart)
{ return (uart->fcr & 1u) != 0u ? UART_FIFO_BYTES : 1u; }

static lib_u8 uart_trigger(const x86_uart16550 *uart)
{
    static const lib_u8 triggers[] = {1u, 4u, 8u, 14u};
    return (uart->fcr & 1u) != 0u ? triggers[(uart->fcr >> 6u) & 3u] : 1u;
}

/* The board supplies its oscillator; character timing uses rounded guest ticks. */
static lib_u64 uart_character_ticks(const x86_uart16550 *uart)
{
    const lib_u64 divisor = uart->divisor != 0u ? uart->divisor : 1u;
    const lib_u64 data_bits = 5u + (uart->lcr & 3u);
    const lib_u64 half_bits = 2u + 2u * data_bits +
        ((uart->lcr & 8u) != 0u ? 2u : 0u) +
        ((uart->lcr & 4u) == 0u ? 2u : data_bits == 5u ? 3u : 4u);
    const lib_u64 numerator = (lib_u64)uart->ticks_per_second * divisor *
        8u * half_bits;
    const lib_u64 ticks = (numerator + uart->input_clock_hz - 1u) / uart->input_clock_hz;
    return ticks != 0u ? ticks : 1u;
}

static void uart_restart_receive_timeout(x86_uart16550 *uart)
{
    uart->receive_timeout = LIB_FALSE;
    uart->rx_ticks = uart->rx_count != 0u && (uart->fcr & 1u) != 0u ?
        4u * uart_character_ticks(uart) : 0u;
}

static void uart_receive(x86_uart16550 *uart, lib_u8 value)
{
    if (uart->rx_count == uart_capacity(uart)) {
        uart->lsr_error |= 2u;
        return;
    }
    uart->rx[(uart->rx_head + uart->rx_count) % UART_FIFO_BYTES] = value;
    ++uart->rx_count;
    uart_restart_receive_timeout(uart);
}

static void uart_start_transmit(x86_uart16550 *uart)
{
    if (uart->shift_busy || uart->tx_count == 0u) return;
    uart->transmitting = uart->tx[uart->tx_head];
    uart->tx_head = (lib_u8)((uart->tx_head + 1u) % UART_FIFO_BYTES);
    --uart->tx_count;
    uart->shift_busy = LIB_TRUE;
    uart->tx_ticks = uart_character_ticks(uart);
    if (uart->tx_count == 0u) uart->thre_pending = LIB_TRUE;
}

static lib_u8 uart_interrupt(const x86_uart16550 *uart)
{
    if ((uart->ier & 4u) != 0u && uart->lsr_error != 0u) return 6u;
    if ((uart->ier & 1u) != 0u && uart->rx_count >= uart_trigger(uart)) return 4u;
    if ((uart->ier & 1u) != 0u && uart->receive_timeout) return 12u;
    if ((uart->ier & 2u) != 0u && uart->thre_pending) return 2u;
    if ((uart->ier & 8u) != 0u && uart->msr_delta != 0u) return 0u;
    return 1u;
}

lib_status x86_uart16550_create(lib_u32 ticks_per_second, lib_u32 input_clock_hz,
    x86_uart16550 **out_uart)
{
    x86_uart16550 *uart;
    if (out_uart == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_uart = LIB_NULL;
    if (ticks_per_second == 0u || input_clock_hz == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    uart = lib_allocate_zero(1u, sizeof(*uart));
    if (uart == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    uart->ticks_per_second = ticks_per_second;
    uart->input_clock_hz = input_clock_hz;
    x86_uart16550_reset(uart);
    *out_uart = uart;
    return LIB_STATUS_OK;
}

void x86_uart16550_destroy(x86_uart16550 *uart) { lib_release(uart); }

void x86_uart16550_reset(x86_uart16550 *uart)
{
    lib_u32 ticks_per_second, input_clock_hz;
    if (uart == LIB_NULL) return;
    ticks_per_second = uart->ticks_per_second;
    input_clock_hz = uart->input_clock_hz;
    *uart = (x86_uart16550){0};
    uart->ticks_per_second = ticks_per_second;
    uart->input_clock_hz = input_clock_hz;
    uart->thre_pending = LIB_TRUE;
}

lib_u8 x86_uart16550_read(x86_uart16550 *uart, lib_u8 offset)
{
    lib_u8 result;
    if (uart == LIB_NULL || offset > 7u) return 0xffu;
    if ((uart->lcr & 0x80u) != 0u && offset < 2u)
        return offset == 0u ? (lib_u8)uart->divisor : (lib_u8)(uart->divisor >> 8u);
    switch (offset) {
    case 0u:
        if (uart->rx_count == 0u) return 0u;
        result = uart->rx[uart->rx_head];
        uart->rx_head = (lib_u8)((uart->rx_head + 1u) % UART_FIFO_BYTES);
        --uart->rx_count;
        uart_restart_receive_timeout(uart);
        return result;
    case 1u: return uart->ier;
    case 2u:
        result = uart_interrupt(uart);
        if (result == 2u) uart->thre_pending = LIB_FALSE;
        return (lib_u8)(result | ((uart->fcr & 1u) != 0u ? 0xc0u : 0u));
    case 3u: return uart->lcr;
    case 4u: return uart->mcr;
    case 5u:
        result = (lib_u8)(uart->lsr_error | (uart->rx_count != 0u ? 1u : 0u) |
            (uart->tx_count == 0u ? 0x20u : 0u) |
            (!uart->shift_busy && uart->tx_count == 0u ? 0x40u : 0u));
        uart->lsr_error = 0u;
        return result;
    case 6u:
        result = (lib_u8)(uart->modem | uart->msr_delta);
        uart->msr_delta = 0u;
        return result;
    default: return uart->scr;
    }
}

void x86_uart16550_write(x86_uart16550 *uart, lib_u8 offset, lib_u8 value)
{
    if (uart == LIB_NULL || offset > 7u) return;
    if ((uart->lcr & 0x80u) != 0u && offset < 2u) {
        if (offset == 0u) uart->divisor = (lib_u16)((uart->divisor & 0xff00u) | value);
        else uart->divisor = (lib_u16)((uart->divisor & 0x00ffu) | ((lib_u16)value << 8u));
        return;
    }
    switch (offset) {
    case 0u:
        if (uart->tx_count < uart_capacity(uart)) {
            uart->tx[(uart->tx_head + uart->tx_count) % UART_FIFO_BYTES] = value;
            ++uart->tx_count;
            uart->thre_pending = LIB_FALSE;
            uart_start_transmit(uart);
        }
        break;
    case 1u:
        if ((uart->ier & 2u) == 0u && (value & 2u) != 0u && uart->tx_count == 0u)
            uart->thre_pending = LIB_TRUE;
        uart->ier = value & 0x0fu;
        break;
    case 2u:
        if (((uart->fcr ^ value) & 1u) != 0u || (value & 2u) != 0u) {
            uart->rx_head = uart->rx_count = 0u;
            uart_restart_receive_timeout(uart);
        }
        if (((uart->fcr ^ value) & 1u) != 0u || (value & 4u) != 0u) {
            uart->tx_head = uart->tx_count = 0u;
            uart->thre_pending = LIB_TRUE;
        }
        uart->fcr = value & 0xc1u;
        break;
    case 3u: uart->lcr = value; break;
    case 4u: {
        const lib_u8 old = uart->modem;
        uart->mcr = value & 0x1fu;
        uart->modem = (value & 0x10u) != 0u ? (lib_u8)(
            ((value & 2u) != 0u ? 0x10u : 0u) |
            ((value & 1u) != 0u ? 0x20u : 0u) |
            ((value & 4u) != 0u ? 0x40u : 0u) |
            ((value & 8u) != 0u ? 0x80u : 0u)) : 0u;
        uart->msr_delta |= (lib_u8)(((old ^ uart->modem) >> 4u) & 0x0bu);
        if ((old & 0x40u) != 0u && (uart->modem & 0x40u) == 0u) uart->msr_delta |= 4u;
        break;
    }
    case 7u: uart->scr = value; break;
    default: break;
    }
}

void x86_uart16550_advance(x86_uart16550 *uart, lib_u64 ticks)
{
    if (uart == LIB_NULL) return;
    while (ticks != 0u) {
        lib_u64 step = ticks;
        if (uart->shift_busy && uart->tx_ticks < step) step = uart->tx_ticks;
        if (uart->rx_ticks != 0u && uart->rx_ticks < step) step = uart->rx_ticks;
        ticks -= step;
        if (uart->rx_ticks != 0u) {
            uart->rx_ticks -= step;
            if (uart->rx_ticks == 0u) uart->receive_timeout = LIB_TRUE;
        }
        if (uart->shift_busy) {
            uart->tx_ticks -= step;
            if (uart->tx_ticks == 0u) {
                uart->shift_busy = LIB_FALSE;
                if ((uart->mcr & 0x10u) != 0u) uart_receive(uart, uart->transmitting);
                uart_start_transmit(uart);
            }
        }
    }
}

lib_bool x86_uart16550_next_event(const x86_uart16550 *uart, lib_u64 *out_ticks)
{
    lib_u64 ticks;
    if (uart == LIB_NULL || out_ticks == LIB_NULL ||
        (!uart->shift_busy && uart->rx_ticks == 0u)) return LIB_FALSE;
    ticks = uart->shift_busy ? uart->tx_ticks : uart->rx_ticks;
    if (uart->rx_ticks != 0u && uart->rx_ticks < ticks) ticks = uart->rx_ticks;
    *out_ticks = ticks;
    return LIB_TRUE;
}

lib_bool x86_uart16550_irq(const x86_uart16550 *uart)
{ return uart != LIB_NULL && uart_interrupt(uart) != 1u; }

lib_bool x86_uart16550_out2(const x86_uart16550 *uart)
{ return uart != LIB_NULL && (uart->mcr & 8u) != 0u; }
