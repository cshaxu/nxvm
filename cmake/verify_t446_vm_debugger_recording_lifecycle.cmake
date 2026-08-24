if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/debug.c" debug_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/debug.h" debug_header)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session.h" session_header)

foreach(required "debug_record_close" "debug_record_write_failed"
        "vm_machine_debug_record_status" "vm_machine_debug_finalize")
    string(FIND "${debug_source}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "VM debugger recorder lifecycle lacks ${required}")
    endif()
endforeach()

foreach(required "type_status record_status" "type_status vm_machine_debug_record_start"
        "type_status vm_machine_debug_record_stop")
    string(FIND "${debug_header}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "VM debugger recorder status boundary lacks ${required}")
    endif()
endforeach()

string(FIND "${session_header}" "core_product_debugger *debugger;"
    session_debugger_position)
if(session_debugger_position EQUAL -1)
    message(FATAL_ERROR "VM session no longer owns its debugger lifecycle handle")
endif()

string(REGEX MATCHALL "static[^\n]*dumpSegRec|static[^\n]*asmSegRec|static[^\n]*uasmSegRec"
    shared_cursor_state "${debug_source}")
if(shared_cursor_state)
    message(FATAL_ERROR "Debugger cursor state must remain instance-owned")
endif()

message("M5:T446:S1:VM-DEBUGGER-RECORDING-LIFECYCLE-STATIC:OK")
