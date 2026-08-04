if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE source_files
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.h")
foreach(source IN LISTS source_files)
    file(READ "${source}" source_text)
    string(REGEX MATCH "[Qq][Dd][Xx]" qdx_residue "${source_text}")
    if(qdx_residue)
        message(FATAL_ERROR "T209 QDX source residue: ${source}")
    endif()
endforeach()

file(GLOB_RECURSE vm_profile_sources
    "${PROJECT_SOURCE_DIR}/src/vm/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/*.h")
foreach(source IN LISTS vm_profile_sources)
    file(READ "${source}" source_text)
    string(REGEX MATCH "insTable(_0f)?[ \\t]*\\[" raw_table_mutation
        "${source_text}")
    if(raw_table_mutation)
        message(FATAL_ERROR
            "T209 VM/profile raw CPU instruction-table access: ${source}")
    endif()
    string(FIND "${source_text}" "extension_context" extension_context)
    if(NOT extension_context EQUAL -1)
        message(FATAL_ERROR
            "T209 VM/profile extension-context dependency: ${source}")
    endif()
endforeach()

message(STATUS "M5 T209 firmware-interrupt portal closure verified")
