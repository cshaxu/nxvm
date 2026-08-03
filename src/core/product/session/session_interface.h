#ifndef NTVDM64_CORE_PRODUCT_SESSION_INTERFACE_H
#define NTVDM64_CORE_PRODUCT_SESSION_INTERFACE_H

#include "type.h"

typedef uint32_t core_product_session_id;

typedef enum core_product_session_state {
    CORE_PRODUCT_SESSION_STATE_READY,
    CORE_PRODUCT_SESSION_STATE_RUNNING,
    CORE_PRODUCT_SESSION_STATE_PAUSED,
    CORE_PRODUCT_SESSION_STATE_STOPPED,
    CORE_PRODUCT_SESSION_STATE_FAULTED
} core_product_session_state;

typedef enum core_product_session_display {
    CORE_PRODUCT_SESSION_DISPLAY_UNKNOWN,
    CORE_PRODUCT_SESSION_DISPLAY_CONSOLE,
    CORE_PRODUCT_SESSION_DISPLAY_WINDOW
} core_product_session_display;

typedef struct core_product_session_snapshot {
    core_product_session_id id;
    C_INT selected;
    core_product_session_state state;
    core_product_session_display display;
} core_product_session_snapshot;

typedef struct core_product_session_manager core_product_session_manager;

#endif
