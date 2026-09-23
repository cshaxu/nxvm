#include <assert.h>

#include "core/controller.h"

int main(void)
{
    core_controller controller;
    lib_u32 buttons;
    lib_u32 index;

    core_controller_reset(&controller);
    for (buttons = 0u; buttons < 256u; ++buttons) {
        core_controller_set_buttons(&controller, (lib_u8)buttons);
        core_controller_write_strobe(&controller, 1u);
        assert(core_controller_read(&controller) == (buttons & 1u));
        core_controller_write_strobe(&controller, 0u);
        for (index = 0u; index < 8u; ++index)
            assert(core_controller_read(&controller) == ((buttons >> index) & 1u));
        assert(core_controller_read(&controller) == 1u);
    }
    core_controller_set_buttons(&controller, 1u);
    core_controller_write_strobe(&controller, 1u);
    core_controller_set_buttons(&controller, 0u);
    assert(core_controller_read(&controller) == 0u);
    /* While strobe is high, A is live and repeated writes do not create a
     * serial snapshot.  A falling edge freezes the complete bitmap until the
     * next falling edge, even if live input subsequently changes. */
    core_controller_set_buttons(&controller, 0x81u);
    core_controller_write_strobe(&controller, 1u);
    assert(core_controller_read(&controller) == 1u);
    core_controller_set_buttons(&controller, 0x02u);
    core_controller_write_strobe(&controller, 1u);
    assert(core_controller_read(&controller) == 0u);
    core_controller_write_strobe(&controller, 0u);
    core_controller_set_buttons(&controller, 0xffu);
    assert(core_controller_read(&controller) == 0u);
    assert(core_controller_read(&controller) == 1u);
    for (index = 2u; index < 8u; ++index) assert(core_controller_read(&controller) == 0u);
    assert(core_controller_read(&controller) == 1u);
    core_controller_reset(&controller);
    assert(core_controller_read(&controller) == 0u);
    return 0;
}
