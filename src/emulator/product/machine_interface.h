#ifndef EMULATOR_PRODUCT_MACHINE_INTERFACE_H
#define EMULATOR_PRODUCT_MACHINE_INTERFACE_H

#include "lib/types/types_interface.h"
#include "emulator/machine/machine_interface.h"
#include "emulator/session/session_interface.h"

/* An App composes this opaque machine lifetime before entering Emulator
 * Product. The neutral shell neither reads configuration nor constructs the
 * private machine; it only creates Emulator around the complete driver and
 * releases this owned value after Emulator has stopped. */
typedef struct emulator_product_machine {
    void *machine;
    emulator_machine_driver driver;
    lib_status (*bind)(void *machine, emulator_machine *emulator);
    lib_status (*destroy)(void *machine);
    /* A product may translate a completed machine fact only at this one
     * composition boundary. It receives no Session/UI pointer and cannot
     * dispatch lifecycle work. NULL uses the direct neutral mapping. */
    void *state_context;
    emulator_session_machine_state (*map_state)(void *context,
        emulator_machine_state state);
} emulator_product_machine;

#endif
