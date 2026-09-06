#ifndef LIB_HOST_SYNC_H
#define LIB_HOST_SYNC_H

#include "lib/base/base.h"

typedef int (*host_sync_cancel_predicate)(void *context);

typedef enum host_sync_wait_result {
    HOST_SYNC_WAIT_COMPLETED,
    HOST_SYNC_WAIT_CANCELLED,
    HOST_SYNC_WAIT_INVALID_ARGUMENT
} host_sync_wait_result;

void host_sync_sleep_milliseconds(lib_u32 milliseconds);
void host_sync_yield(void);
host_sync_wait_result host_sync_wait_milliseconds(lib_u32 milliseconds,
    host_sync_cancel_predicate cancelled, void *context);

#endif
