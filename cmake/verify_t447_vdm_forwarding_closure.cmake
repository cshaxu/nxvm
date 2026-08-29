if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required.")
endif()

foreach(path IN ITEMS
    "src/vdm/composition/session.c"
    "src/vdm/composition/session.h")
    if(EXISTS "${PROJECT_SOURCE_DIR}/${path}")
        message(FATAL_ERROR "T447 S9 retired VDM forwarding path remains: ${path}")
    endif()
endforeach()

file(GLOB_RECURSE vdm_sources
    "${PROJECT_SOURCE_DIR}/src/vdm/*.c"
    "${PROJECT_SOURCE_DIR}/src/vdm/*.h"
    "${PROJECT_SOURCE_DIR}/test/*vdm*.c"
    "${PROJECT_SOURCE_DIR}/test/*dos_minimal*.c")
foreach(file IN LISTS vdm_sources)
    file(READ "${file}" source)
    if(source MATCHES "vdm_session")
        message(FATAL_ERROR "T447 S9 obsolete VDM session facade remains: ${file}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_source)
if(cmake_source MATCHES "src/vdm/composition/session\\.c" OR
        cmake_source MATCHES "add_executable[ \\t\\r\\n]*\\([ \\t\\r\\n]*nxvdm")
    message(FATAL_ERROR "T447 S9 VDM build boundary regressed.")
endif()

message(STATUS "M5 T447 S9 VDM forwarding closure: OK")
