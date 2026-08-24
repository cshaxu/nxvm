if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/console_machine_adapter.c"
    adapter_source)

string(FIND "${adapter_source}" "core_product_session_manager_apply_selected"
    selected_operation_position)
string(FIND "${adapter_source}" "vm_session_machine_apply(context"
    adapter_apply_position)
if(selected_operation_position EQUAL -1 OR adapter_apply_position EQUAL -1)
    message(FATAL_ERROR "Selected-session operations are not converged in composition")
endif()

foreach(obsolete_helper
    "vm_session_provider_selected"
    "core_product_session_manager_borrow_selected"
    "vm_session_machine_borrow_selected"
    "vm_session_machine_selection"
    "selected_session.h")
    string(FIND "${adapter_source}" "${obsolete_helper}" obsolete_position)
    if(NOT obsolete_position EQUAL -1)
        message(FATAL_ERROR "Console adapter retained obsolete selected-session wrapper")
    endif()
endforeach()

foreach(removed_file
    "${PROJECT_SOURCE_DIR}/src/vm/composition/session/selected_session.c"
    "${PROJECT_SOURCE_DIR}/src/vm/composition/session/selected_session.h")
    if(EXISTS "${removed_file}")
        message(FATAL_ERROR "Console selected-session helper remains unnecessarily split")
    endif()
endforeach()

foreach(required
    "vm_session_control_request_pause"
    "vm_session_control_wait_for_pause"
    "core_product_debugger_run")
    string(FIND "${adapter_source}" "${required}" debug_position)
    if(debug_position EQUAL -1)
        message(FATAL_ERROR "Console debugger callback lost required behavior: ${required}")
    endif()
endforeach()

message("M5:T205:S1:CONSOLE-ADAPTER-CLOSURE:OK")
