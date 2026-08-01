#ifndef NXVM_RUNTIME_DOS_MINIMAL_H
#define NXVM_RUNTIME_DOS_MINIMAL_H

#include <stdint.h>

#include "core/machine/presentation.h"
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_runtime_dos_minimal nxvm_runtime_dos_minimal;

typedef struct nxvm_runtime_text_snapshot {
    uint64_t pit_ticks;
    uint8_t keyboard_irq_pending;
    nxvm_core_text_snapshot text;
} nxvm_runtime_text_snapshot;

nxvm_core_status nxvm_runtime_dos_minimal_create(
    nxvm_runtime_dos_minimal **out_session);
nxvm_core_status nxvm_runtime_dos_minimal_reset(
    nxvm_runtime_dos_minimal *session);
nxvm_core_status nxvm_runtime_dos_minimal_tick(
    nxvm_runtime_dos_minimal *session,
    uint32_t ticks);
nxvm_core_status nxvm_runtime_dos_minimal_inject_key(
    nxvm_runtime_dos_minimal *session,
    uint8_t scan_code);
nxvm_core_status nxvm_runtime_dos_minimal_write_text(
    nxvm_runtime_dos_minimal *session,
    uint16_t cell,
    uint8_t character,
    uint8_t attribute);
nxvm_core_status nxvm_runtime_dos_minimal_get_snapshot(
    const nxvm_runtime_dos_minimal *session,
    nxvm_runtime_text_snapshot *out_snapshot);
nxvm_core_status nxvm_runtime_dos_minimal_port_read(
    nxvm_runtime_dos_minimal *session,
    uint16_t port,
    uint32_t *out_value);
void nxvm_runtime_dos_minimal_destroy(nxvm_runtime_dos_minimal *session);

#ifdef __cplusplus
}
#endif

#endif
