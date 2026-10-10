#ifndef PRODUCT_SURFACE_ENTRY_INTERFACE_H
#define PRODUCT_SURFACE_ENTRY_INTERFACE_H

#include "product/surface/command_interface.h"
#include "product/surface/machine_interface.h"

/* App has already interpreted its configuration. Product sees only the two
 * choices required to compose Emulator Session/UI; this is not a Emulator UI
 * instance and Product remains its sole creator. */
typedef struct app_composed_ui {
    emulator_session_display display;
    lib_bool console_control;
} app_composed_ui;

typedef lib_status (*product_surface_extensions_configure)(
    app_composed_machine *machine,
    product_surface_command_extensions *out_extensions);

/* Both App-composed values are transferred to Product. It does not receive an
 * App configuration object, loader callback, or machine-construction API. */
typedef struct product_surface_definition {
    const char *name;
    const char *banner;
    app_composed_machine machine;
    app_composed_ui ui;
    product_surface_extensions_configure configure_extensions;
} product_surface_definition;

/* Sole PC process body. Returns the existing process success/failure code. */
lib_i32 product_surface_run(const product_surface_definition *definition);

#endif
