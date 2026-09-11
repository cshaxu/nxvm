if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(obsolete_adapter
    "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/console_machine_adapter.c")
if(EXISTS "${obsolete_adapter}")
    message(FATAL_ERROR "Retired Console-to-composition adapter remains")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/product/machine_adapter.c" product_source)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/debug_target.c"
    debug_source)

foreach(obsolete_helper
    "vm_machine_provider_selected"
    "core_product_session_manager_borrow_selected"
    "vm_machine_borrow_selected"
    "vm_machine_selection"
    "selected_session.h")
    string(FIND "${product_source}\n${debug_source}" "${obsolete_helper}" obsolete_position)
    if(NOT obsolete_position EQUAL -1)
        message(FATAL_ERROR "Console adapter retained obsolete selected-session wrapper")
    endif()
endforeach()

foreach(removed_file
    "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/selected_session.c"
    "${PROJECT_SOURCE_DIR}/src/vm/machine/runtime/selected_session.h")
    if(EXISTS "${removed_file}")
        message(FATAL_ERROR "Console selected-session helper remains unnecessarily split")
    endif()
endforeach()

foreach(required
    "vm_machine_run_debugger"
    "vm_machine_request_pause"
    "vm_machine_control_wait_for_pause"
    "core_debugger_run")
    string(FIND "${product_source}\n${debug_source}" "${required}" debug_position)
    if(debug_position EQUAL -1)
        message(FATAL_ERROR "Console debugger callback lost required behavior: ${required}")
    endif()
endforeach()

message("M5:T526:PRODUCT-DEBUGGER-ADAPTER-CLOSURE:OK")
