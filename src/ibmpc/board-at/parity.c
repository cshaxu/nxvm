#include "ibmpc/board-at/parity_interface.h"
#include "x86/core/memory_interface.h"
#include "x86/core/port_interface.h"

struct core_machine_at_parity {
    core_machine *core;
    x86_pit *pit;
    core_machine_planar_parity_config config;
    core_machine_at_speaker_lines speaker;
    void *speaker_context;
    lib_u8 port_b;
    lib_bool latched;
    lib_bool nmi_signaled;
};

void core_machine_at_parity_refresh_nmi(core_machine_at_parity *parity)
{
    if (parity != LIB_NULL && parity->config.memory_bytes != 0u &&
        parity->latched && (parity->port_b & 0x04u) != 0u &&
        !parity->nmi_signaled && core_machine_signal_nmi(parity->core))
        parity->nmi_signaled = LIB_TRUE;
}

lib_status core_machine_at_parity_report_fault(core_machine_at_parity *parity)
{
    if (parity == LIB_NULL || !core_machine_mutable_operation_is_allowed(parity->core) ||
        parity->config.memory_bytes == 0u) return LIB_STATUS_INVALID_STATE;
    parity->latched = LIB_TRUE;
    core_machine_at_parity_refresh_nmi(parity);
    return LIB_STATUS_OK;
}

static void parity_memory_fault(void *context, lib_u32 physical)
{
    (void)physical;
    (void)core_machine_at_parity_report_fault(context);
}

static lib_status parity_port_read(void *context, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    const core_machine_at_parity *parity = context;
    lib_u8 timer = 0u;
    if (parity == LIB_NULL || out_value == LIB_NULL || port != parity->config.port)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (parity->config.refresh_status_source ==
        CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE) {
        if (((tick / parity->config.refresh_status_toggle_ticks) & 1u) != 0u)
            timer |= 0x10u;
    } else if (x86_pit_get_output(parity->pit, 1u)) timer |= 0x10u;
    if (x86_pit_get_output(parity->pit, 2u)) timer |= 0x20u;
    *out_value = (parity->port_b & 0x0fu) | timer | (parity->latched ? 0x80u : 0u);
    return LIB_STATUS_OK;
}

static lib_status parity_port_write(void *context, lib_u16 port, lib_u32 value)
{
    core_machine_at_parity *parity = context;
    if (parity == LIB_NULL || port != parity->config.port)
        return LIB_STATUS_INVALID_ARGUMENT;
    parity->port_b = (lib_u8)value & 0x0fu;
    if (parity->speaker != LIB_NULL) parity->speaker(parity->speaker_context, parity->port_b);
    if ((parity->port_b & 0x04u) == 0u) {
        parity->latched = LIB_FALSE;
        parity->nmi_signaled = LIB_FALSE;
    } else core_machine_at_parity_refresh_nmi(parity);
    return LIB_STATUS_OK;
}

void core_machine_at_parity_reset(core_machine_at_parity *parity)
{
    if (parity == LIB_NULL) return;
    parity->port_b = 0x04u;
    parity->latched = LIB_FALSE;
    parity->nmi_signaled = LIB_FALSE;
    if (parity->speaker != LIB_NULL) parity->speaker(parity->speaker_context, parity->port_b);
}

void core_machine_at_parity_destroy(core_machine_at_parity *parity)
{
    if (parity == LIB_NULL) return;
    (void)core_machine_remove_port_routes(parity->core, parity);
    if (parity->config.memory_bytes != 0u)
        (void)core_machine_remove_memory_device_routes(parity->core, parity);
    lib_release(parity);
}

lib_status core_machine_at_parity_create(core_machine *machine, x86_pit *pit,
    const core_machine_planar_parity_config *config,
    core_machine_at_speaker_lines speaker, void *speaker_context,
    core_machine_at_parity **out_parity)
{
    core_machine_at_parity *parity;
    core_machine_port_route route;
    lib_status status;
    if (out_parity == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_parity = LIB_NULL;
    if (machine == LIB_NULL || !core_machine_configuration_is_open(machine))
        return LIB_STATUS_INVALID_STATE;
    if (pit == LIB_NULL || config == LIB_NULL || config->port != 0x61u ||
        (config->refresh_status_source != CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1 &&
        config->refresh_status_source != CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE) ||
        (config->refresh_status_source == CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE &&
        config->refresh_status_toggle_ticks == 0u)) return LIB_STATUS_INVALID_ARGUMENT;
    parity = lib_allocate_zero(1u, sizeof(*parity));
    if (parity == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    parity->core = machine;
    parity->pit = pit;
    parity->config = *config;
    parity->speaker = speaker;
    parity->speaker_context = speaker_context;
    if (config->memory_bytes != 0u) {
        const core_machine_memory_parity_config memory = {
            config->memory_bytes, parity_memory_fault
        };
        status = core_machine_install_memory_device_routes(machine, LIB_NULL, 0u,
            LIB_NULL, &memory, parity);
        if (status != LIB_STATUS_OK) {
            lib_release(parity);
            return status;
        }
    }
    route = (core_machine_port_route) {
        .address = config->port, .read = parity_port_read,
        .write = parity_port_write, .owner = parity
    };
    status = core_machine_install_port_routes(machine, &route, 1u);
    if (status != LIB_STATUS_OK) {
        if (config->memory_bytes != 0u) {
            lib_status rollback = core_machine_remove_memory_device_routes(machine, parity);
            if (rollback != LIB_STATUS_OK) status = rollback;
        }
        lib_release(parity);
        return status == LIB_STATUS_INVALID_STATE ? LIB_STATUS_INVALID_ARGUMENT : status;
    }
    core_machine_at_parity_reset(parity);
    *out_parity = parity;
    return LIB_STATUS_OK;
}

void core_machine_at_parity_observe(const core_machine_at_parity *parity,
    core_machine_planar_parity_observation *out_observation)
{
    if (out_observation == LIB_NULL) return;
    *out_observation = (core_machine_planar_parity_observation){0};
    if (parity == LIB_NULL) return;
    out_observation->configured = parity->config.memory_bytes != 0u;
    out_observation->enabled = out_observation->configured && (parity->port_b & 0x04u) != 0u;
    out_observation->latched = parity->latched;
    out_observation->nmi_signaled = parity->nmi_signaled;
}
