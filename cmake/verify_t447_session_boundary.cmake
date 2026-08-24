if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/core/product/session/session_interface.h" interface)
file(READ "${PROJECT_SOURCE_DIR}/src/core/product/session/command.c" command)
file(READ "${PROJECT_SOURCE_DIR}/src/vm/product/console.c" console)

foreach(forbidden
    "core_product_session_manager_borrow_selected"
    "C_CHAR **arguments")
    string(FIND "${interface}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T447 session boundary retains ${forbidden}")
    endif()
endforeach()

foreach(required
    "core_product_session_manager_apply_selected"
    "core_product_session_manager_get_selected_snapshot"
    "const C_CHAR *const *arguments")
    string(FIND "${interface}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "T447 session boundary lacks ${required}")
    endif()
endforeach()

string(REGEX MATCHALL "core_product_session_command_execute" console_commands
    "${console}")
list(LENGTH console_commands console_command_count)
if(console_command_count LESS 2)
    message(FATAL_ERROR "T447 VM console does not route SESSION through Core command authority")
endif()

foreach(forbidden
    "core_product_session_manager_open_with_options(context->session_manager"
    "core_product_session_manager_select(sessionManager"
    "core_product_session_manager_close(sessionManager")
    string(FIND "${console}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "T447 VM console retains duplicate SESSION dispatch: ${forbidden}")
    endif()
endforeach()

string(FIND "${command}" "core_product_session_command_execute" position)
if(position EQUAL -1)
    message(FATAL_ERROR "T447 lacks Core SESSION command authority")
endif()

message("M5:T447:S2:SESSION-BOUNDARY:OK")
