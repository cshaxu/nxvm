#ifndef EMULATOR_SESSION_PRESENTATION_PLAN_H
#define EMULATOR_SESSION_PRESENTATION_PLAN_H

#include "emulator/session/session_interface.h"

/* Pure product-policy derivation.  It neither creates components nor touches
 * host Console registration; the reconciler applies the returned facts. */
emulator_session_presentation_plan emulator_session_derive_presentation(
    emulator_session_display display, lib_bool console_control,
    emulator_session_machine_state state, lib_bool frame_available,
    lib_bool graphics);

#endif
