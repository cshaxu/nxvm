#include "type.h"

#include "vdm/machine/dos_minimal.h"


#include <stdlib.h>

#include <string.h>


#include "core/machine/machine_interface.h"

#define NXVM_RUNTIME_KEY_QUEUE_CAPACITY 16u

struct core_product_runtime_dos_minimal {
    core_machine *machine;
    core_product_runtime_text_snapshot snapshot;
    uint8_t key_queue[NXVM_RUNTIME_KEY_QUEUE_CAPACITY];
    uint8_t key_count;
};

static C_VOID core_product_runtime_dos_minimal_clear(
    core_product_runtime_dos_minimal *session)
{
    STD_MEMSET(&session->snapshot, 0, sizeof(session->snapshot));
    core_machine_text_snapshot_initialize(&session->snapshot.text);
    STD_MEMSET(session->snapshot.text.characters, ' ',
           sizeof(session->snapshot.text.characters));
    STD_MEMSET(session->snapshot.text.attributes, 0x07,
           sizeof(session->snapshot.text.attributes));
    session->key_count = 0u;
}

static ntvdm64_status core_product_runtime_dos_minimal_port_read_handler(
    C_VOID *owner,
    uint16_t port,
    uint32_t *out_value)
{
    core_product_runtime_dos_minimal *session = (core_product_runtime_dos_minimal *)owner;

    if (port == 0x20u) {
        *out_value = session->snapshot.keyboard_irq_pending ? 0x02u : 0u;
    } else if (port == 0x40u) {
        *out_value = (uint8_t)session->snapshot.pit_ticks;
    } else if (port == 0x60u) {
        if (session->key_count == 0u) {
            return NTVDM64_STATUS_UNSUPPORTED;
        }
        *out_value = session->key_queue[0];
        memmove(session->key_queue, session->key_queue + 1u,
                --session->key_count);
        session->snapshot.keyboard_irq_pending =
            session->key_count != 0u;
    } else if (port == 0x64u) {
        *out_value = session->key_count != 0u ? 0x01u : 0u;
    } else {
        return NTVDM64_STATUS_UNSUPPORTED;
    }

    return NTVDM64_STATUS_OK;
}

static ntvdm64_status core_product_runtime_dos_minimal_port_write_handler(
    C_VOID *owner,
    uint16_t port,
    uint32_t value)
{
    core_product_runtime_dos_minimal *session = (core_product_runtime_dos_minimal *)owner;

    if (port == 0x20u && value == 0x20u) {
        session->snapshot.keyboard_irq_pending = session->key_count != 0u;
        return NTVDM64_STATUS_OK;
    }

    return NTVDM64_STATUS_UNSUPPORTED;
}

static ntvdm64_status core_product_runtime_dos_minimal_install_ports(
    core_product_runtime_dos_minimal *session)
{
    const core_machine_port_provider ops = {
        core_product_runtime_dos_minimal_port_read_handler,
        core_product_runtime_dos_minimal_port_write_handler
    };
    ntvdm64_status status;

    status = core_machine_install_port_provider(session->machine, 0x20u, 0x20u,
                                            &ops, session);
    if (status != NTVDM64_STATUS_OK) {
        return status;
    }
    status = core_machine_install_port_provider(session->machine, 0x40u, 0x43u,
                                            &ops, session);
    if (status != NTVDM64_STATUS_OK) {
        return status;
    }
    return core_machine_install_port_provider(session->machine, 0x60u, 0x64u,
                                          &ops, session);
}

ntvdm64_status core_product_runtime_dos_minimal_create(
    core_product_runtime_dos_minimal **out_session)
{
    core_product_runtime_dos_minimal *session;
    core_machine_config config = {
        CORE_MACHINE_PROFILE_TEST_MINIMAL,
        CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    ntvdm64_status status;

    if (out_session == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    *out_session = STD_NULL;
    session = (core_product_runtime_dos_minimal *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) {
        return NTVDM64_STATUS_NO_MEMORY;
    }
    status = core_machine_create(&config, &session->machine);
    if (status == NTVDM64_STATUS_OK) {
        status = core_product_runtime_dos_minimal_install_ports(session);
    }
    if (status == NTVDM64_STATUS_OK) {
        status = core_product_runtime_dos_minimal_reset(session);
    }
    if (status != NTVDM64_STATUS_OK) {
        core_product_runtime_dos_minimal_destroy(session);
        return status;
    }
    *out_session = session;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_runtime_dos_minimal_reset(
    core_product_runtime_dos_minimal *session)
{
    ntvdm64_status status;

    if (session == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_reset(session->machine);
    if (status == NTVDM64_STATUS_OK) {
        core_product_runtime_dos_minimal_clear(session);
    }
    return status;
}

ntvdm64_status core_product_runtime_dos_minimal_tick(
    core_product_runtime_dos_minimal *session,
    uint32_t ticks)
{
    if (session == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    session->snapshot.pit_ticks += ticks;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_runtime_dos_minimal_inject_key(
    core_product_runtime_dos_minimal *session,
    uint8_t scan_code)
{
    if (session == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    if (session->key_count == NXVM_RUNTIME_KEY_QUEUE_CAPACITY) {
        return NTVDM64_STATUS_FAULT;
    }
    session->key_queue[session->key_count++] = scan_code;
    session->snapshot.keyboard_irq_pending = 1u;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_runtime_dos_minimal_write_text(
    core_product_runtime_dos_minimal *session,
    uint16_t cell,
    uint8_t character,
    uint8_t attribute)
{
    if (session == STD_NULL || cell >= CORE_MACHINE_TEXT_CELLS) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    session->snapshot.text.characters[cell] = character;
    session->snapshot.text.attributes[cell] = attribute;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_runtime_dos_minimal_get_snapshot(
    const core_product_runtime_dos_minimal *session,
    core_product_runtime_text_snapshot *out_snapshot)
{
    if (session == STD_NULL || out_snapshot == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    *out_snapshot = session->snapshot;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_product_runtime_dos_minimal_port_read(
    core_product_runtime_dos_minimal *session,
    uint16_t port,
    uint32_t *out_value)
{
    if (session == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    return core_machine_bus_read(session->machine, port, out_value);
}

C_VOID core_product_runtime_dos_minimal_destroy(core_product_runtime_dos_minimal *session)
{
    if (session != STD_NULL) {
        core_machine_destroy(session->machine);
        STD_FREE(session);
    }
}
