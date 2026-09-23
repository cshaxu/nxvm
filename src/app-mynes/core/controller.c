#include "core/controller.h"

void core_controller_reset(core_controller *controller)
{
    *controller = (core_controller) { 0 };
}

void core_controller_set_buttons(core_controller *controller, lib_u8 buttons)
{
    controller->held = buttons;
    controller->live = (lib_u8)(controller->held | controller->transient);
}

void core_controller_tap_buttons(core_controller *controller, lib_u8 buttons)
{
    controller->transient |= buttons;
    controller->live = (lib_u8)(controller->held | controller->transient);
}

void core_controller_clear_transient_buttons(core_controller *controller)
{
    controller->transient = 0u;
    controller->live = controller->held;
}

void core_controller_write_strobe(core_controller *controller, lib_u8 value)
{
    lib_bool strobe = (value & 1u) != 0u;

    if (controller->strobe && !strobe) {
        controller->captured = controller->live;
        controller->index = 0u;
        core_controller_clear_transient_buttons(controller);
    }
    controller->strobe = strobe;
}

lib_u8 core_controller_read(core_controller *controller)
{
    lib_u8 value;

    if (controller->strobe) return controller->live & 1u;
    value = controller->index < 8u ? (lib_u8)((controller->captured >> controller->index) & 1u) : 1u;
    if (controller->index < 8u) ++controller->index;
    return value;
}
