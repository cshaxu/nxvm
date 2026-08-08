#include "type.h"

#include "vdm/machine/dos_minimal.h"





#include "core/machine/machine_interface.h"

struct vdm_machine_dos_minimal {
    core_machine *machine;
    vdm_machine_text_snapshot snapshot;
};

static C_VOID vdm_machine_dos_minimal_clear(
    vdm_machine_dos_minimal *session)
{
    STD_MEMSET(&session->snapshot, 0, sizeof(session->snapshot));
    core_machine_text_snapshot_initialize(&session->snapshot.text);
    STD_MEMSET(session->snapshot.text.characters, ' ',
           sizeof(session->snapshot.text.characters));
    STD_MEMSET(session->snapshot.text.attributes, 0x07,
           sizeof(session->snapshot.text.attributes));
}

type_status vdm_machine_dos_minimal_create(
    vdm_machine_dos_minimal **out_session)
{
    vdm_machine_dos_minimal *session;
    core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    type_status status;

    if (out_session == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = STD_NULL;
    session = (vdm_machine_dos_minimal *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) {
        return TYPE_STATUS_NO_MEMORY;
    }
    status = core_machine_create(&config, &session->machine);
    if (status == TYPE_STATUS_OK) {
        status = core_machine_freeze_execution_providers(session->machine);
    }
    if (status == TYPE_STATUS_OK) {
        status = vdm_machine_dos_minimal_reset(session);
    }
    if (status != TYPE_STATUS_OK) {
        vdm_machine_dos_minimal_destroy(session);
        return status;
    }
    *out_session = session;
    return TYPE_STATUS_OK;
}

type_status vdm_machine_dos_minimal_reset(
    vdm_machine_dos_minimal *session)
{
    type_status status;

    if (session == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_reset(session->machine);
    if (status == TYPE_STATUS_OK) {
        vdm_machine_dos_minimal_clear(session);
    }
    return status;
}

type_status vdm_machine_dos_minimal_tick(
    vdm_machine_dos_minimal *session,
    uint32_t ticks)
{
    if (session == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    session->snapshot.pit_ticks += ticks;
    return TYPE_STATUS_OK;
}

type_status vdm_machine_dos_minimal_inject_key(
    vdm_machine_dos_minimal *session,
    uint8_t scan_code)
{
    type_status status;

    if (session == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_keyboard_submit_scan_code(session->machine, scan_code);
    if (status == TYPE_STATUS_OK) {
        session->snapshot.keyboard_irq_pending = 1u;
    }
    return status;
}

type_status vdm_machine_dos_minimal_write_text(
    vdm_machine_dos_minimal *session,
    uint16_t cell,
    uint8_t character,
    uint8_t attribute)
{
    if (session == STD_NULL || cell >= CORE_MACHINE_TEXT_CELLS) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    session->snapshot.text.characters[cell] = character;
    session->snapshot.text.attributes[cell] = attribute;
    return TYPE_STATUS_OK;
}

type_status vdm_machine_dos_minimal_get_snapshot(
    const vdm_machine_dos_minimal *session,
    vdm_machine_text_snapshot *out_snapshot)
{
    if (session == STD_NULL || out_snapshot == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_snapshot = session->snapshot;
    return TYPE_STATUS_OK;
}

type_status vdm_machine_dos_minimal_port_read(
    vdm_machine_dos_minimal *session,
    uint16_t port,
    uint32_t *out_value)
{
    type_status status;

    if (session == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_bus_read(session->machine, port, out_value);
    if (status == TYPE_STATUS_OK && port == 0x0060u) {
        session->snapshot.keyboard_irq_pending = 0u;
    }
    return status;
}

C_VOID vdm_machine_dos_minimal_destroy(vdm_machine_dos_minimal *session)
{
    if (session != STD_NULL) {
        core_machine_destroy(session->machine);
        STD_FREE(session);
    }
}

