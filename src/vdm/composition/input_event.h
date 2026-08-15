#ifndef VDM_COMPOSITION_INPUT_EVENT_H
#define VDM_COMPOSITION_INPUT_EVENT_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vdm_composition_input_event {
    type_unsigned_64 timestamp;
    /* Bytes at this skeleton boundary are emitted by a native keyboard, not
     * host Set-1 events or guest FIFO writes. */
    type_unsigned_8 native_byte;
} vdm_composition_input_event;

#ifdef __cplusplus
}
#endif

#endif
