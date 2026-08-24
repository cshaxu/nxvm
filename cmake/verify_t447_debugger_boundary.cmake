if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/product/debug/debug.h" interface)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.h" session)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/console_machine_adapter.c" adapter)

foreach(forbidden
    "typedef struct core_product_debug_context {"
    "debugger_context"
    "core_product_debug_main")
    string(FIND "${interface}\n${session}\n${adapter}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T447 debugger boundary retains ${forbidden}")
    endif()
endforeach()

foreach(required
    "typedef struct core_product_debugger core_product_debugger"
    "core_product_debugger_create"
    "core_product_debugger_destroy"
    "core_product_debugger_run")
    string(FIND "${interface}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T447 debugger boundary lacks ${required}")
    endif()
endforeach()

string(FIND "${adapter}" "core_product_debugger_run" position)
if(position EQUAL -1)
    message(FATAL_ERROR "T447 debugger boundary lacks VM debugger invocation")
endif()

message("M5:T447:S3:DEBUGGER-BOUNDARY:OK")
