#include "ibmpc/machine/mouse_mapper_interface.h"

lib_i32 main(void)
{
    static const struct {
        lib_i16 x;
        lib_i16 y;
        lib_u8 buttons;
        lib_i16 expected_y;
        lib_u8 expected_buttons;
    } cases[] = {
        {0, 0, 0u, 0, 0u}, {7, 8, 0xffu, -8, 7u},
        {-7, -8, 2u, 8, 2u}, {0, LIB_INT16_MIN, 0u, LIB_INT16_MAX, 0u}
    };
    vm_profile_default_mouse_report report;

    if (vm_profile_default_mouse_map_host_relative(0, 0, 0u, LIB_NULL) !=
        LIB_STATUS_INVALID_ARGUMENT) return 1;
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (vm_profile_default_mouse_map_host_relative(cases[index].x,
                cases[index].y, cases[index].buttons, &report) != LIB_STATUS_OK ||
            report.delta_x != cases[index].x ||
            report.delta_y != cases[index].expected_y ||
            report.buttons != cases[index].expected_buttons) return 1;
    }
    return 0;
}
