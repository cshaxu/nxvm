#include "lib/types/types_interface.h"
#include "emulator/product/composition_interface.h"

struct emulator_product {
    emulator_product_machine machine;
    emulator_machine *emulator_machine;
    emulator_session *session;
    emulator_ui *ui;
};

static emulator_session_machine_state emulator_product_machine_state(
    const emulator_product *product, emulator_machine_state state)
{
    if (product->machine.map_state != LIB_NULL)
        return product->machine.map_state(product->machine.state_context, state);
    switch (state) {
    case EMULATOR_MACHINE_RUNNING: return EMULATOR_SESSION_MACHINE_RUNNING;
    case EMULATOR_MACHINE_PAUSED: return EMULATOR_SESSION_MACHINE_PAUSED;
    case EMULATOR_MACHINE_RESET_COMPLETED:
        return EMULATOR_SESSION_MACHINE_RESET_COMPLETED;
    case EMULATOR_MACHINE_ERROR: return EMULATOR_SESSION_MACHINE_ERROR;
    default: return EMULATOR_SESSION_MACHINE_STOPPED;
    }
}

static void emulator_product_machine_state_completed(void *context,
    emulator_machine_state state, lib_u32 run_generation)
{
    emulator_product *product = context;

    if (product == LIB_NULL || product->session == LIB_NULL)
        return;
    (void)emulator_session_enqueue_runtime_completed(product->session,
        emulator_product_machine_state(product, state), run_generation);
}

static void emulator_product_machine_frame_published(void *context, lib_u32 sequence,
    lib_bool graphics, lib_u32 run_generation)
{
    emulator_product *product = context;

    if (product == LIB_NULL || product->session == LIB_NULL)
        return;
    (void)emulator_session_enqueue_frame_completed(product->session,
        sequence, graphics, run_generation);
}

lib_status emulator_product_create(const emulator_product_machine *machine,
    emulator_product **out_product)
{
    emulator_product *product;

    if (out_product == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_product = LIB_NULL;
    if (machine == LIB_NULL || machine->machine == LIB_NULL ||
        machine->bind == LIB_NULL || machine->destroy == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    product = lib_allocate_zero(1u, sizeof(*product));
    if (product == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    product->machine = *machine;
    *out_product = product;
    return LIB_STATUS_OK;
}

lib_status emulator_product_destroy(emulator_product *product)
{
    lib_status shutdown_status;

    if (product == LIB_NULL) return LIB_STATUS_OK;
    shutdown_status = emulator_machine_shutdown(product->emulator_machine);
    if (shutdown_status != LIB_STATUS_OK)
        return shutdown_status;
    shutdown_status = emulator_ui_destroy(product->ui);
    if (shutdown_status != LIB_STATUS_OK)
        return shutdown_status;
    product->ui = LIB_NULL;
    emulator_session_destroy(product->session);
    product->session = LIB_NULL;
    shutdown_status = product->machine.bind(product->machine.machine, LIB_NULL);
    if (shutdown_status != LIB_STATUS_OK)
        return shutdown_status;
    shutdown_status = emulator_machine_destroy(product->emulator_machine);
    if (shutdown_status != LIB_STATUS_OK)
        return shutdown_status;
    product->emulator_machine = LIB_NULL;
    shutdown_status = product->machine.destroy(product->machine.machine);
    if (shutdown_status != LIB_STATUS_OK)
        return shutdown_status;
    lib_release(product);
    return LIB_STATUS_OK;
}

lib_status emulator_product_compose_machine(emulator_product *product)
{
    emulator_machine_driver driver;
    void *machine = LIB_NULL;
    emulator_machine *emulator_machine = LIB_NULL;
    lib_status status;

    if (product == LIB_NULL || product->emulator_machine != LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    machine = product->machine.machine;
    driver = product->machine.driver;
    status = emulator_machine_create(&emulator_machine, &driver);
    if (status == LIB_STATUS_OK) {
        status = product->machine.bind(machine, emulator_machine);
    }
    if (status != LIB_STATUS_OK) {
        (void)product->machine.bind(machine, LIB_NULL);
        lib_status cleanup_status = emulator_machine_destroy(emulator_machine);

        if (cleanup_status != LIB_STATUS_OK) {
            product->emulator_machine = emulator_machine;
            return cleanup_status;
        }
        return status;
    }
    product->emulator_machine = emulator_machine;
    return LIB_STATUS_OK;
}

lib_status emulator_product_compose_control(emulator_product *product,
    const emulator_session_options *options)
{
    emulator_session_options resolved;
    emulator_session *session = LIB_NULL;
    lib_status status;

    if (product == LIB_NULL || options == LIB_NULL ||
        product->emulator_machine == LIB_NULL ||
        product->session != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    resolved = *options;
    resolved.machine = product->emulator_machine;
    status = emulator_session_create(&session, &resolved);
    if (status != LIB_STATUS_OK) return status;
    product->session = session;
    emulator_machine_set_state_sink(resolved.machine,
        emulator_product_machine_state_completed, product);
    emulator_machine_set_frame_sink(resolved.machine,
        emulator_product_machine_frame_published, product);
    return LIB_STATUS_OK;
}

lib_status emulator_product_compose_ui(emulator_product *product,
    const emulator_ui_options *options)
{
    emulator_ui *ui = LIB_NULL;
    lib_status status;

    if (product == LIB_NULL || options == LIB_NULL || product->session == LIB_NULL ||
        product->ui != LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    status = emulator_ui_create(&ui, options);
    if (status != LIB_STATUS_OK) return status;
    product->ui = ui;
    status = emulator_session_bind_ui(product->session, ui);
    if (status != LIB_STATUS_OK) {
        lib_status cleanup_status = emulator_ui_destroy(ui);

        if (cleanup_status != LIB_STATUS_OK)
            return cleanup_status;
        product->ui = LIB_NULL;
        return status;
    }
    return LIB_STATUS_OK;
}

lib_status emulator_product_publish_initial_state(emulator_product *product)
{
    emulator_machine_state state;

    if (product == LIB_NULL || product->emulator_machine == LIB_NULL ||
        product->session == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    state = emulator_machine_state_get(product->emulator_machine);
    if (state != EMULATOR_MACHINE_STOPPED) return LIB_STATUS_INVALID_STATE;
    return emulator_session_enqueue_runtime_completed(product->session,
        emulator_product_machine_state(product, state),
        emulator_machine_run_generation(product->emulator_machine)) ?
        LIB_STATUS_OK : LIB_STATUS_INTERNAL_ERROR;
}

lib_i32 emulator_product_run(const emulator_product_definition *definition)
{
    emulator_product *product = LIB_NULL;
    emulator_session_options session_options = {0};
    emulator_ui_options ui_options = {0};
    lib_status status = LIB_STATUS_OK;
    lib_status destroy_status;

    if (definition == LIB_NULL || definition->machine.machine == LIB_NULL ||
        definition->machine.bind == LIB_NULL ||
        definition->machine.destroy == LIB_NULL ||
        definition->configure_control == LIB_NULL || definition->configure_ui == LIB_NULL)
        return 1;
    status = emulator_product_create(&definition->machine, &product);
    if (status == LIB_STATUS_OK) status = emulator_product_compose_machine(product);
    if (status == LIB_STATUS_OK)
        status = definition->configure_control(definition->context,
            product->emulator_machine, &session_options);
    if (status == LIB_STATUS_OK) status = emulator_product_compose_control(product,
        &session_options);
    if (status == LIB_STATUS_OK) status = emulator_product_publish_initial_state(product);
    if (status == LIB_STATUS_OK)
        status = definition->configure_ui(definition->context, &ui_options);
    if (status == LIB_STATUS_OK) {
        ui_options.event_context = product->session;
        ui_options.event_sink = emulator_session_enqueue_ui_event;
        status = emulator_product_compose_ui(product, &ui_options);
    }
    if (status == LIB_STATUS_OK && !emulator_session_run(product->session))
        status = LIB_STATUS_INTERNAL_ERROR;
    if (product != LIB_NULL)
        destroy_status = emulator_product_destroy(product);
    else {
        (void)definition->machine.bind(definition->machine.machine, LIB_NULL);
        destroy_status = definition->machine.destroy(definition->machine.machine);
    }
    return status == LIB_STATUS_OK && destroy_status == LIB_STATUS_OK ? 0 : 1;
}
