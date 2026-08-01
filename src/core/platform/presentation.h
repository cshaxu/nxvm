#ifndef NXVM_PLATFORM_PRESENTATION_H
#define NXVM_PLATFORM_PRESENTATION_H

#include <stdint.h>

#include "core/machine/status.h"
#include "vdm/machine/dos_minimal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_platform_input_event {
    uint64_t timestamp;
    uint8_t scan_code;
} nxvm_platform_input_event;

typedef struct nxvm_platform_text_snapshot {
    uint64_t timestamp;
    nxvm_runtime_text_snapshot text;
} nxvm_platform_text_snapshot;

#ifdef __cplusplus
}
#endif

#endif
