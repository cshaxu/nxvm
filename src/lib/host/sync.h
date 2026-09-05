#ifndef LIB_HOST_SYNC_H
#define LIB_HOST_SYNC_H

#include "type.h"

typedef C_INT (*host_sync_cancel_predicate)(C_VOID *context);

typedef enum host_sync_wait_result {
    HOST_SYNC_WAIT_COMPLETED,
    HOST_SYNC_WAIT_CANCELLED,
    HOST_SYNC_WAIT_INVALID_ARGUMENT
} host_sync_wait_result;

C_VOID host_sync_sleep_milliseconds(type_unsigned_32 milliseconds);
C_VOID host_sync_yield(C_VOID);
host_sync_wait_result host_sync_wait_milliseconds(type_unsigned_32 milliseconds,
    host_sync_cancel_predicate cancelled, C_VOID *context);

#endif
