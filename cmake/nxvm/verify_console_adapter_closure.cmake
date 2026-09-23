if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(obsolete_adapter
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/console_machine_adapter.c")
if(EXISTS "${obsolete_adapter}")
    message(FATAL_ERROR "Retired Console-to-composition adapter remains")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/product/composition.c" app_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/product/command.c" product_source)
file(READ "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/debug_adapter.c"
    debug_source)

foreach(obsolete_helper
    "vm_machine_provider_selected"
    "core_product_session_manager_borrow_selected"
    "vm_machine_borrow_selected"
    "vm_machine_selection"
    "selected_session.h")
    string(FIND "${app_source}\n${product_source}\n${debug_source}" "${obsolete_helper}" obsolete_position)
    if(NOT obsolete_position EQUAL -1)
        message(FATAL_ERROR "Console adapter retained obsolete selected-session wrapper")
    endif()
endforeach()

foreach(removed_file
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/selected_session.c"
    "${PROJECT_SOURCE_DIR}/src/app-nxvm/machine/selected_session.h")
    if(EXISTS "${removed_file}")
        message(FATAL_ERROR "Console selected-session helper remains unnecessarily split")
    endif()
endforeach()

foreach(required
    "x86_debug_submit_line"
    "x86_debug_open"
    "vm_machine_debug_execute"
    "vm_app_compose_ui"
    "common_ui_create"
    "common_ui_destroy")
    string(FIND "${app_source}\n${product_source}\n${debug_source}" "${required}" debug_position)
    if(debug_position EQUAL -1)
        message(FATAL_ERROR "Console debugger callback lost required behavior: ${required}")
    endif()
endforeach()

foreach(forbidden "common_ui_create(" "common_ui_destroy(")
    string(FIND "${product_source}" "${forbidden}" product_ui_position)
    if(NOT product_ui_position EQUAL -1)
        message(FATAL_ERROR "Product retains Common UI lifetime ownership: ${forbidden}")
    endif()
endforeach()

message("M5:T526:PRODUCT-DEBUGGER-ADAPTER-CLOSURE:OK")
