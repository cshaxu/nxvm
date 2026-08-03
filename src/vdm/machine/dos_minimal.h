#ifndef NXVM_RUNTIME_DOS_MINIMAL_H
#define NXVM_RUNTIME_DOS_MINIMAL_H


#include "core/machine/presentation_interface.h"
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_product_runtime_dos_minimal core_product_runtime_dos_minimal;

typedef struct core_product_runtime_text_snapshot {
    uint64_t pit_ticks;
    uint8_t keyboard_irq_pending;
    core_machine_text_snapshot text;
} core_product_runtime_text_snapshot;

ntvdm64_status core_product_runtime_dos_minimal_create(
    core_product_runtime_dos_minimal **out_session);
ntvdm64_status core_product_runtime_dos_minimal_reset(
    core_product_runtime_dos_minimal *session);
ntvdm64_status core_product_runtime_dos_minimal_tick(
    core_product_runtime_dos_minimal *session,
    uint32_t ticks);
ntvdm64_status core_product_runtime_dos_minimal_inject_key(
    core_product_runtime_dos_minimal *session,
    uint8_t scan_code);
ntvdm64_status core_product_runtime_dos_minimal_write_text(
    core_product_runtime_dos_minimal *session,
    uint16_t cell,
    uint8_t character,
    uint8_t attribute);
ntvdm64_status core_product_runtime_dos_minimal_get_snapshot(
    const core_product_runtime_dos_minimal *session,
    core_product_runtime_text_snapshot *out_snapshot);
ntvdm64_status core_product_runtime_dos_minimal_port_read(
    core_product_runtime_dos_minimal *session,
    uint16_t port,
    uint32_t *out_value);
C_VOID core_product_runtime_dos_minimal_destroy(core_product_runtime_dos_minimal *session);

#ifdef __cplusplus
}
#endif

#endif
