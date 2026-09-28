#include "lib/types/test.h"
#include "x86/devices/ppi8255/ppi8255_interface.h"

int main(void)
{
    x86_ppi8255 *ppi = LIB_NULL;
    lib_u8 value = 0u;
    x86_ppi8255_pins pins;
    lib_test_assert(x86_ppi8255_create(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_ppi8255_create(&ppi) == LIB_STATUS_OK);
    for (lib_u32 control = 0x80u; control <= 0xffu; ++control) {
        lib_bool mode0 = (control & 0x64u) == 0u;
        lib_u8 masks[3] = {
            mode0 && (control & 0x10u) == 0u ? 0xffu : 0u,
            mode0 && (control & 0x02u) == 0u ? 0xffu : 0u,
            mode0 ? ((control & 0x01u) == 0u ? 0x0fu : 0u) |
                ((control & 0x08u) == 0u ? 0xf0u : 0u) : 0u
        };
        x86_ppi8255_reset(ppi);
        for (lib_u8 port = 0u; port < 3u; ++port) {
            lib_test_assert(x86_ppi8255_write(ppi, port, 0xa5u) == LIB_STATUS_OK);
        }
        lib_test_assert(x86_ppi8255_write(ppi, 3u, (lib_u8)control) == LIB_STATUS_OK);
        for (lib_u8 port = 0u; port < 3u; ++port) {
            lib_u8 expected = mode0 ? (0xa5u & masks[port]) | (0x5au & (lib_u8)~masks[port]) :
                port == 0u ? 0xa5u : port == 1u ? 0x5au : 0u;
            lib_test_assert(x86_ppi8255_read(ppi, port, 0x5au, &value) == LIB_STATUS_OK);
            lib_test_assert(value == expected);
            lib_test_assert(x86_ppi8255_output(ppi, port, &pins) == LIB_STATUS_OK);
            lib_test_assert(pins.latch == 0xa5u && pins.output_mask == masks[port]);
        }
    }
    x86_ppi8255_reset(ppi);
    lib_test_assert(x86_ppi8255_write(ppi, 3u, 0x80u) == LIB_STATUS_OK);
    for (lib_u8 bit = 0u; bit < 8u; ++bit) {
        lib_test_assert(x86_ppi8255_write(ppi, 3u, (lib_u8)(bit * 2u + 1u)) == LIB_STATUS_OK);
        lib_test_assert(x86_ppi8255_read(ppi, 2u, 0u, &value) == LIB_STATUS_OK);
        lib_test_assert(value == (1u << bit));
        lib_test_assert(x86_ppi8255_write(ppi, 3u, (lib_u8)(bit * 2u)) == LIB_STATUS_OK);
        lib_test_assert(x86_ppi8255_read(ppi, 2u, 0u, &value) == LIB_STATUS_OK && value == 0u);
    }
    lib_test_assert(x86_ppi8255_read(ppi, 3u, 0u, &value) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(x86_ppi8255_read(ppi, 4u, 0u, &value) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_ppi8255_write(ppi, 4u, 0u) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_ppi8255_output(ppi, 3u, &pins) == LIB_STATUS_INVALID_ARGUMENT);
    x86_ppi8255_destroy(ppi);
    x86_ppi8255_destroy(LIB_NULL);
    return 0;
}
