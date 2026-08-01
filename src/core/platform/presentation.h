#ifndef NXVM_PLATFORM_PRESENTATION_H
#define NXVM_PLATFORM_PRESENTATION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_platform_input_event {
    uint64_t timestamp;
    uint8_t scan_code;
} nxvm_platform_input_event;

#ifdef __cplusplus
}
#endif

#endif
