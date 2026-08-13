if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/console_machine_adapter.c"
    adapter_source)

file(GLOB composition_session_sources
    "${PROJECT_SOURCE_DIR}/src/vm/composition/session/*.c")
set(composition_session_source "")
foreach(source_file IN LISTS composition_session_sources)
    file(READ "${source_file}" source_text)
    string(APPEND composition_session_source "\n${source_text}")
endforeach()

string(REGEX MATCHALL "core_product_session_manager_borrow_selected"
    selected_borrow_calls "${composition_session_source}")
list(LENGTH selected_borrow_calls selected_borrow_call_count)
string(REGEX MATCHALL "vm_session_machine_borrow_selected\\(context\\)"
    adapter_helper_calls "${adapter_source}")
list(LENGTH adapter_helper_calls adapter_helper_call_count)
if(NOT selected_borrow_call_count EQUAL 1 OR adapter_helper_call_count EQUAL 0)
    message(FATAL_ERROR "Selected-session borrowing is not converged in composition")
endif()

foreach(obsolete_helper
    "vm_session_provider_selected"
    "vm_session_borrow_selected"
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
    "core_product_debug_context_initialize"
    "core_product_debug_main")
    string(FIND "${adapter_source}" "${required}" debug_position)
    if(debug_position EQUAL -1)
        message(FATAL_ERROR "Console debugger callback lost required behavior: ${required}")
    endif()
endforeach()

message("M5:T205:S1:CONSOLE-ADAPTER-CLOSURE:OK")
