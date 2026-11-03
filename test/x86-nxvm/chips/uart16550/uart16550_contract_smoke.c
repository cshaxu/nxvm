#include "lib/types/test.h"
#include "x86-nxvm/chips/uart16550/uart16550_interface.h"

int main(void)
{
    x86_uart16550 *uart = LIB_NULL;
    lib_u64 ticks;
    lib_test_assert(x86_uart16550_create(33000000u, 1843200u, &uart) == LIB_STATUS_OK);
    {
        x86_uart16550 *slower = LIB_NULL;
        lib_u64 normal_ticks, slower_ticks;
        lib_test_assert(x86_uart16550_create(33000000u, 921600u, &slower) == LIB_STATUS_OK);
        x86_uart16550_write(uart, 0u, 1u);
        x86_uart16550_write(slower, 0u, 1u);
        lib_test_assert(x86_uart16550_next_event(uart, &normal_ticks));
        lib_test_assert(x86_uart16550_next_event(slower, &slower_ticks));
        lib_test_assert(slower_ticks <= normal_ticks * 2u &&
            slower_ticks + 1u >= normal_ticks * 2u);
        x86_uart16550_destroy(slower);
        x86_uart16550_reset(uart);
    }
    lib_test_assert(x86_uart16550_read(uart, 5u) == 0x60u);
    lib_test_assert(x86_uart16550_read(uart, 2u) == 1u);
    x86_uart16550_write(uart, 3u, 0x80u);
    x86_uart16550_write(uart, 0u, 12u);
    x86_uart16550_write(uart, 1u, 1u);
    lib_test_assert(x86_uart16550_read(uart, 0u) == 12u);
    lib_test_assert(x86_uart16550_read(uart, 1u) == 1u);
    x86_uart16550_write(uart, 3u, 3u);
    x86_uart16550_write(uart, 2u, 0x41u);
    lib_test_assert(x86_uart16550_read(uart, 2u) == 0xc1u);
    x86_uart16550_write(uart, 4u, 0x1bu);
    lib_test_assert(x86_uart16550_out2(uart));
    lib_test_assert((x86_uart16550_read(uart, 6u) & 0xf0u) == 0xb0u);
    x86_uart16550_write(uart, 1u, 1u);
    for (lib_u8 i = 0u; i < 4u; ++i) x86_uart16550_write(uart, 0u, (lib_u8)(0x40u + i));
    for (lib_u8 i = 0u; i < 4u; ++i) {
        lib_test_assert(x86_uart16550_next_event(uart, &ticks));
        x86_uart16550_advance(uart, ticks);
    }
    lib_test_assert(x86_uart16550_irq(uart));
    lib_test_assert(x86_uart16550_read(uart, 2u) == 0xc4u);
    for (lib_u8 i = 0u; i < 4u; ++i)
        lib_test_assert(x86_uart16550_read(uart, 0u) == (lib_u8)(0x40u + i));
    lib_test_assert(!x86_uart16550_irq(uart));
    x86_uart16550_write(uart, 0u, 0x55u);
    lib_test_assert(x86_uart16550_next_event(uart, &ticks));
    x86_uart16550_advance(uart, ticks);
    lib_test_assert(!x86_uart16550_irq(uart));
    lib_test_assert(x86_uart16550_next_event(uart, &ticks));
    x86_uart16550_advance(uart, ticks);
    lib_test_assert(x86_uart16550_read(uart, 2u) == 0xccu);
    lib_test_assert(x86_uart16550_read(uart, 0u) == 0x55u);
    x86_uart16550_write(uart, 2u, 0x07u);
    x86_uart16550_write(uart, 1u, 4u);
    for (lib_u8 i = 0u; i < 17u; ++i) x86_uart16550_write(uart, 0u, i);
    for (lib_u8 i = 0u; i < 17u; ++i) {
        lib_test_assert(x86_uart16550_next_event(uart, &ticks));
        x86_uart16550_advance(uart, ticks);
    }
    lib_test_assert(x86_uart16550_read(uart, 2u) == 0xc6u);
    lib_test_assert((x86_uart16550_read(uart, 5u) & 2u) != 0u);
    lib_test_assert((x86_uart16550_read(uart, 5u) & 2u) == 0u);
    for (lib_u8 i = 0u; i < 16u; ++i)
        lib_test_assert(x86_uart16550_read(uart, 0u) == i);
    x86_uart16550_write(uart, 1u, 2u);
    lib_test_assert(x86_uart16550_irq(uart));
    lib_test_assert(x86_uart16550_read(uart, 2u) == 0xc2u);
    lib_test_assert(!x86_uart16550_irq(uart));
    x86_uart16550_reset(uart);
    lib_test_assert(x86_uart16550_read(uart, 5u) == 0x60u);
    lib_test_assert(!x86_uart16550_next_event(uart, &ticks));
    x86_uart16550_destroy(uart);
    return 0;
}
