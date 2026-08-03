#ifndef NTVDM64_VDM_COMPOSITION_INPUT_EVENT_H
#define NTVDM64_VDM_COMPOSITION_INPUT_EVENT_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vdm_composition_input_event {
    uint64_t timestamp;
    uint8_t scan_code;
} vdm_composition_input_event;

#ifdef __cplusplus
}
#endif

#endif
