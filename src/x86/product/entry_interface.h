#ifndef X86_PRODUCT_ENTRY_INTERFACE_H
#define X86_PRODUCT_ENTRY_INTERFACE_H

#include "x86/product/command_interface.h"
#include "x86/product/machine_interface.h"

/* App has already interpreted its configuration. Product sees only the two
 * choices required to compose Emulator Session/UI; this is not a Emulator UI
 * instance and Product remains its sole creator. */
typedef struct app_composed_ui {
    emulator_session_display display;
    lib_bool console_control;
} app_composed_ui;

typedef lib_status (*x86_product_extensions_configure)(
    app_composed_machine *machine,
    x86_product_command_extensions *out_extensions);

/* Both App-composed values are transferred to Product. It does not receive an
 * App configuration object, loader callback, or machine-construction API. */
typedef struct x86_product_definition {
    const char *name;
    const char *banner;
    app_composed_machine machine;
    app_composed_ui ui;
    x86_product_extensions_configure configure_extensions;
} x86_product_definition;

/* Sole PC process body. Returns the existing process success/failure code. */
lib_i32 x86_product_run(const x86_product_definition *definition);

#endif
