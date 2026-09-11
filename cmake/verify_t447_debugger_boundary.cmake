if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/debug/debug.h" interface)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/machine_private.h" session)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/product/machine_adapter.c" product)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/debug_target.c" composition)

foreach(forbidden
    "typedef struct core_debug_context {"
    "debugger_context"
    "core_debug_main")
    string(FIND "${interface}\n${session}\n${product}\n${composition}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T447 debugger boundary retains ${forbidden}")
    endif()
endforeach()

foreach(required
    "typedef struct core_debugger core_debugger"
    "core_debugger_create"
    "core_debugger_destroy"
    "core_debugger_run")
    string(FIND "${interface}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T447 debugger boundary lacks ${required}")
    endif()
endforeach()

string(FIND "${composition}" "core_debugger_run" position)
if(position EQUAL -1)
    message(FATAL_ERROR "T447 debugger boundary lacks composition debugger invocation")
endif()

message("M5:T447:S3:DEBUGGER-BOUNDARY:OK")
