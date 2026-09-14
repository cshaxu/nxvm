#ifndef COMMON_SESSION_PRESENTATION_PLAN_H
#define COMMON_SESSION_PRESENTATION_PLAN_H

#include "common/session/session_interface.h"

/* Pure product-policy derivation.  It neither creates components nor touches
 * host Console registration; the reconciler applies the returned facts. */
common_session_presentation_plan common_session_derive_presentation(
    common_session_display display, lib_bool console_control,
    common_session_machine_state state, lib_bool frame_available,
    lib_bool graphics);

#endif
