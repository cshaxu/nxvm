#ifndef VDM_COMPOSITION_INPUT_EVENT_H
#define VDM_COMPOSITION_INPUT_EVENT_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vdm_composition_input_event {
    type_unsigned_64 timestamp;
    type_unsigned_8 scan_code;
} vdm_composition_input_event;

#ifdef __cplusplus
}
#endif

#endif
