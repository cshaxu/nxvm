#ifndef CORE_CONTROLLER_H
#define CORE_CONTROLLER_H

#include "lib/types/types_interface.h"

typedef struct core_controller {
    lib_u8 live;
    lib_u8 held;
    lib_u8 transient;
    lib_u8 captured;
    lib_u8 index;
    lib_bool strobe;
} core_controller;

void core_controller_reset(core_controller *controller);
void core_controller_set_buttons(core_controller *controller, lib_u8 buttons);
/* Text-only host input has no release transition. It is sampled exactly once
 * by the next falling controller strobe, then removed from live input. */
void core_controller_tap_buttons(core_controller *controller, lib_u8 buttons);
void core_controller_clear_transient_buttons(core_controller *controller);
void core_controller_write_strobe(core_controller *controller, lib_u8 value);
lib_u8 core_controller_read(core_controller *controller);

#endif
