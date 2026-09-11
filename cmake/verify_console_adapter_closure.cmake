if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(obsolete_adapter
    "${PROJECT_SOURCE_DIR}/src/vm/composition/session/console_machine_adapter.c")
if(EXISTS "${obsolete_adapter}")
    message(FATAL_ERROR "Retired Console-to-composition adapter remains")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/product/machine_adapter.c" product_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/debug_target.c"
    debug_source)

foreach(obsolete_helper
    "vm_session_provider_selected"
    "core_product_session_manager_borrow_selected"
    "vm_session_machine_borrow_selected"
    "vm_session_machine_selection"
    "selected_session.h")
    string(FIND "${product_source}\n${debug_source}" "${obsolete_helper}" obsolete_position)
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
    "vm_session_run_debugger"
    "vm_session_control_request_pause"
    "vm_session_control_wait_for_pause"
    "core_product_debugger_run")
    string(FIND "${product_source}\n${debug_source}" "${required}" debug_position)
    if(debug_position EQUAL -1)
        message(FATAL_ERROR "Console debugger callback lost required behavior: ${required}")
    endif()
endforeach()

message("M5:T526:PRODUCT-DEBUGGER-ADAPTER-CLOSURE:OK")
