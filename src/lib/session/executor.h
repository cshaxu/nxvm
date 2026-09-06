#ifndef LIB_SESSION_EXECUTOR_H
#define LIB_SESSION_EXECUTOR_H

#include "lib/base/base.h"

typedef enum lib_session_lifecycle {
    LIB_SESSION_STOPPED,
    LIB_SESSION_RUNNING,
    LIB_SESSION_PAUSED
} lib_session_lifecycle;

typedef struct lib_session_executor_sink {
    lib_session_lifecycle (*get_lifecycle)(void *context);
    int (*get_flip)(void *context);
    void (*start)(void *context);
    void (*stop)(void *context);
} lib_session_executor_sink;

typedef struct lib_session_executor lib_session_executor;

lib_status lib_session_executor_create(const lib_session_executor_sink *sink,
    void *context, lib_session_executor **out_executor);
void lib_session_executor_destroy(lib_session_executor *executor);
int lib_session_executor_is_running(const lib_session_executor *executor);
lib_session_lifecycle lib_session_executor_get_lifecycle(
    const lib_session_executor *executor);
int lib_session_executor_get_flip(const lib_session_executor *executor);
void lib_session_executor_start(const lib_session_executor *executor);
void lib_session_executor_stop(const lib_session_executor *executor);

#endif
