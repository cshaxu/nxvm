#ifndef EMULATOR_PRODUCT_COMPOSITION_INTERFACE_H
#define EMULATOR_PRODUCT_COMPOSITION_INTERFACE_H
#include "lib/types/types_interface.h"


#include "emulator/session/session_interface.h"
#include "emulator/ui/ui_interface.h"
#include "emulator/product/machine_interface.h"
typedef struct emulator_product emulator_product;

/* Product-specific code supplies concrete machine and command/UI policy.  The
 * Emulator product owns the invariant process order: startup text, machine,
 * Session, UI, run, and reverse-order teardown. */
/* The product receives this opaque-machine borrow while emulator_product_run()
 * owns the composition. It must not retain or use it after that function
 * returns. */
typedef lib_status (*emulator_product_configure_control)(void *context,
    emulator_machine *machine, emulator_session_options *out_options);
typedef lib_status (*emulator_product_configure_ui)(void *context,
    emulator_ui_options *out_options);

typedef struct emulator_product_definition {
    const char *name;
    emulator_product_machine machine;
    void *context;
    emulator_product_configure_control configure_control;
    emulator_product_configure_ui configure_ui;
} emulator_product_definition;

lib_status emulator_product_create(const emulator_product_machine *machine,
    emulator_product **out_product);
lib_status emulator_product_destroy(emulator_product *product);
lib_status emulator_product_compose_machine(emulator_product *product);
lib_status emulator_product_compose_control(emulator_product *product,
    const emulator_session_options *options);
lib_status emulator_product_compose_ui(emulator_product *product,
    const emulator_ui_options *options);
lib_status emulator_product_publish_initial_state(emulator_product *product);
lib_i32 emulator_product_run(const emulator_product_definition *definition);

#endif
