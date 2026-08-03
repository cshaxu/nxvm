if(NOT DEFINED NTVDM64_SOURCE_DIR)
    message(FATAL_ERROR "NTVDM64_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE product_sources LIST_DIRECTORIES FALSE
    "${NTVDM64_SOURCE_DIR}/src/vm/*.c"
    "${NTVDM64_SOURCE_DIR}/src/vdm/*.c")

set(forbidden_lifecycle_call
    "core_machine_(cpu_state|cpu_execution|memory|port|pic|pit|dma|kbc|vadp|fpu)_(initialize|reset|refresh|finalize)[ \t\r\n]*\\(")

foreach(source_file IN LISTS product_sources)
    file(READ "${source_file}" source_text)
    string(REGEX MATCH "${forbidden_lifecycle_call}" match "${source_text}")
    if(match)
        file(RELATIVE_PATH relative_path "${NTVDM64_SOURCE_DIR}" "${source_file}")
        message(FATAL_ERROR "Product code owns generic core lifecycle: ${relative_path}: ${match}")
    endif()
endforeach()

message(STATUS "M5 T159 core lifecycle ownership: OK")
