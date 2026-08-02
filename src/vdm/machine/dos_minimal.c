#include "vdm/machine/dos_minimal.h"

#include <stdlib.h>
#include <string.h>

#include "core/machine/machine_interface.h"

#define NXVM_RUNTIME_KEY_QUEUE_CAPACITY 16u

struct nxvm_runtime_dos_minimal {
    core_machine *machine;
    nxvm_runtime_text_snapshot snapshot;
    uint8_t key_queue[NXVM_RUNTIME_KEY_QUEUE_CAPACITY];
    uint8_t key_count;
};

static void nxvm_runtime_dos_minimal_clear(
    nxvm_runtime_dos_minimal *session)
{
    memset(&session->snapshot, 0, sizeof(session->snapshot));
    core_machine_text_snapshot_initialize(&session->snapshot.text);
    memset(session->snapshot.text.characters, ' ',
           sizeof(session->snapshot.text.characters));
    memset(session->snapshot.text.attributes, 0x07,
           sizeof(session->snapshot.text.attributes));
    session->key_count = 0u;
}

static nxvm_core_status nxvm_runtime_dos_minimal_port_read_handler(
    void *owner,
    uint16_t port,
    uint32_t *out_value)
{
    nxvm_runtime_dos_minimal *session = (nxvm_runtime_dos_minimal *)owner;

    if (port == 0x20u) {
        *out_value = session->snapshot.keyboard_irq_pending ? 0x02u : 0u;
    } else if (port == 0x40u) {
        *out_value = (uint8_t)session->snapshot.pit_ticks;
    } else if (port == 0x60u) {
        if (session->key_count == 0u) {
            return NXVM_CORE_STATUS_UNSUPPORTED;
        }
        *out_value = session->key_queue[0];
        memmove(session->key_queue, session->key_queue + 1u,
                --session->key_count);
        session->snapshot.keyboard_irq_pending =
            session->key_count != 0u;
    } else if (port == 0x64u) {
        *out_value = session->key_count != 0u ? 0x01u : 0u;
    } else {
        return NXVM_CORE_STATUS_UNSUPPORTED;
    }

    return NXVM_CORE_STATUS_OK;
}

static nxvm_core_status nxvm_runtime_dos_minimal_port_write_handler(
    void *owner,
    uint16_t port,
    uint32_t value)
{
    nxvm_runtime_dos_minimal *session = (nxvm_runtime_dos_minimal *)owner;

    if (port == 0x20u && value == 0x20u) {
        session->snapshot.keyboard_irq_pending = session->key_count != 0u;
        return NXVM_CORE_STATUS_OK;
    }

    return NXVM_CORE_STATUS_UNSUPPORTED;
}

static nxvm_core_status nxvm_runtime_dos_minimal_install_ports(
    nxvm_runtime_dos_minimal *session)
{
    const core_machine_port_provider ops = {
        nxvm_runtime_dos_minimal_port_read_handler,
        nxvm_runtime_dos_minimal_port_write_handler
    };
    nxvm_core_status status;

    status = core_machine_install_port_provider(session->machine, 0x20u, 0x20u,
                                            &ops, session);
    if (status != NXVM_CORE_STATUS_OK) {
        return status;
    }
    status = core_machine_install_port_provider(session->machine, 0x40u, 0x43u,
                                            &ops, session);
    if (status != NXVM_CORE_STATUS_OK) {
        return status;
    }
    return core_machine_install_port_provider(session->machine, 0x60u, 0x64u,
                                          &ops, session);
}

nxvm_core_status nxvm_runtime_dos_minimal_create(
    nxvm_runtime_dos_minimal **out_session)
{
    nxvm_runtime_dos_minimal *session;
    core_machine_config config = {
        CORE_MACHINE_PROFILE_TEST_MINIMAL,
        CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    nxvm_core_status status;

    if (out_session == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = NULL;
    session = (nxvm_runtime_dos_minimal *)calloc(1u, sizeof(*session));
    if (session == NULL) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }
    status = core_machine_create(&config, &session->machine);
    if (status == NXVM_CORE_STATUS_OK) {
        status = nxvm_runtime_dos_minimal_install_ports(session);
    }
    if (status == NXVM_CORE_STATUS_OK) {
        status = nxvm_runtime_dos_minimal_reset(session);
    }
    if (status != NXVM_CORE_STATUS_OK) {
        nxvm_runtime_dos_minimal_destroy(session);
        return status;
    }
    *out_session = session;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_runtime_dos_minimal_reset(
    nxvm_runtime_dos_minimal *session)
{
    nxvm_core_status status;

    if (session == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_reset(session->machine);
    if (status == NXVM_CORE_STATUS_OK) {
        nxvm_runtime_dos_minimal_clear(session);
    }
    return status;
}

nxvm_core_status nxvm_runtime_dos_minimal_tick(
    nxvm_runtime_dos_minimal *session,
    uint32_t ticks)
{
    if (session == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    session->snapshot.pit_ticks += ticks;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_runtime_dos_minimal_inject_key(
    nxvm_runtime_dos_minimal *session,
    uint8_t scan_code)
{
    if (session == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    if (session->key_count == NXVM_RUNTIME_KEY_QUEUE_CAPACITY) {
        return NXVM_CORE_STATUS_FAULT;
    }
    session->key_queue[session->key_count++] = scan_code;
    session->snapshot.keyboard_irq_pending = 1u;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_runtime_dos_minimal_write_text(
    nxvm_runtime_dos_minimal *session,
    uint16_t cell,
    uint8_t character,
    uint8_t attribute)
{
    if (session == NULL || cell >= CORE_MACHINE_TEXT_CELLS) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    session->snapshot.text.characters[cell] = character;
    session->snapshot.text.attributes[cell] = attribute;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_runtime_dos_minimal_get_snapshot(
    const nxvm_runtime_dos_minimal *session,
    nxvm_runtime_text_snapshot *out_snapshot)
{
    if (session == NULL || out_snapshot == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    *out_snapshot = session->snapshot;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_runtime_dos_minimal_port_read(
    nxvm_runtime_dos_minimal *session,
    uint16_t port,
    uint32_t *out_value)
{
    if (session == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    return core_machine_port_read(session->machine, port, out_value);
}

void nxvm_runtime_dos_minimal_destroy(nxvm_runtime_dos_minimal *session)
{
    if (session != NULL) {
        core_machine_destroy(session->machine);
        free(session);
    }
}
