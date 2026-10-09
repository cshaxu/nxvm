#include "lib/types/file.h"

#include "emulator/product/composition_interface.h"
#include "emulator/product/monitor_interface.h"

lib_i32 emulator_product_run(const emulator_product_definition *definition)
{
    emulator_product *product = LIB_NULL;
    emulator_session_options session_options = {0};
    emulator_ui_options ui_options = {0};
    lib_u8 startup[EMULATOR_SESSION_TEXT_CAPACITY];
    lib_status status = LIB_STATUS_OK;
    lib_status destroy_status;

    if (definition == LIB_NULL || definition->name == LIB_NULL ||
        definition->machine.machine == LIB_NULL ||
        definition->machine.bind == LIB_NULL ||
        definition->machine.destroy == LIB_NULL ||
        definition->configure_control == LIB_NULL || definition->configure_ui == LIB_NULL)
        return 1;
    if (emulator_product_monitor_format_startup(definition->name, (char *)startup,
            sizeof(startup)) != LIB_STATUS_OK) return 1;
    lib_c_printf("%s", startup);
    status = emulator_product_create(&definition->machine, &product);
    if (status == LIB_STATUS_OK) status = emulator_product_compose_machine(product);
    if (status == LIB_STATUS_OK)
        status = definition->configure_control(definition->context, product,
            &session_options);
    if (status == LIB_STATUS_OK) status = emulator_product_compose_control(product,
        &session_options);
    if (status == LIB_STATUS_OK) status = emulator_product_publish_initial_state(product);
    if (status == LIB_STATUS_OK)
        status = definition->configure_ui(definition->context, product, &ui_options);
    if (status == LIB_STATUS_OK) {
        ui_options.event_context = emulator_product_session(product);
        ui_options.event_sink = emulator_session_enqueue_ui_event;
        status = emulator_product_compose_ui(product, &ui_options);
    }
    if (status == LIB_STATUS_OK && !emulator_session_run(emulator_product_session(product)))
        status = LIB_STATUS_INTERNAL_ERROR;
    if (product != LIB_NULL)
        destroy_status = emulator_product_destroy(product);
    else {
        (void)definition->machine.bind(definition->machine.machine, LIB_NULL);
        destroy_status = definition->machine.destroy(definition->machine.machine);
    }
    return status == LIB_STATUS_OK && destroy_status == LIB_STATUS_OK ? 0 : 1;
}
