if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(public_header "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session_interface.h")
set(private_header "${PROJECT_SOURCE_DIR}/src/vm/composition/session/session_private.h")
foreach(file IN ITEMS "${public_header}" "${private_header}")
    if(NOT EXISTS "${file}")
        message(FATAL_ERROR "M5 T234 S6 missing session layout boundary file: ${file}")
    endif()
endforeach()

file(READ "${public_header}" public_source)
if(public_source MATCHES "typedef[ \t\r\n]+struct[ \t\r\n]+vm_session[ \t\r\n]*\\{")
    message(FATAL_ERROR "M5 T234 S6 public session header exposes the complete layout")
endif()

file(READ "${private_header}" private_source)
if(NOT private_source MATCHES "struct[ \t\r\n]+vm_session[ \t\r\n]*\\{")
    message(FATAL_ERROR "M5 T234 S6 private session header lacks the complete layout")
endif()

file(GLOB_RECURSE source_files
    "${PROJECT_SOURCE_DIR}/src/*.c" "${PROJECT_SOURCE_DIR}/src/*.h")
foreach(file IN LISTS source_files)
    if(file STREQUAL private_header)
        continue()
    endif()
    file(READ "${file}" source)
    if(source MATCHES "#include[ \t]+\"vm/composition/session/session_private.h\"" AND
        NOT file MATCHES "/src/vm/composition/" AND
        NOT file STREQUAL "${PROJECT_SOURCE_DIR}/src/vm/composition/session/console_machine_adapter.c")
        message(FATAL_ERROR "M5 T234 S6 private session header escapes composition: ${file}")
    endif()
endforeach()

message(STATUS "M5 T234 S6 session layout boundary: OK")
