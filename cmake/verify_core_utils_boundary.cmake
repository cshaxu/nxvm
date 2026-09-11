if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

foreach(file IN ITEMS
    "${PROJECT_SOURCE_DIR}/src/core/utils/wait.c"
    "${PROJECT_SOURCE_DIR}/src/core/utils/wait.h"
    "${PROJECT_SOURCE_DIR}/src/core/utils/wait_provider.h"
    "${PROJECT_SOURCE_DIR}/src/core/product/wait.c"
    "${PROJECT_SOURCE_DIR}/src/core/product/wait.h"
    "${PROJECT_SOURCE_DIR}/src/core/product/wait_provider.h")
    if(EXISTS "${file}")
        message(FATAL_ERROR "Core retained a polling wait helper: ${file}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/core/product/debug/debug.c" debug_source)
if(debug_source MATCHES "core_utils_wait|wait_scope|Sleep\\(|host_sync_")
    message(FATAL_ERROR "Core debugger retains a host wait implementation")
endif()
file(READ "${PROJECT_SOURCE_DIR}/src/core/product/debug/debug_target.h" target_source)
string(FIND "${target_source}" "wait_for_completion" completion_position)
if(completion_position EQUAL -1)
    message(FATAL_ERROR "Core debugger target lacks completion contract")
endif()

file(GLOB_RECURSE peer_sources
    "${PROJECT_SOURCE_DIR}/src/vm/machine/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/machine/*.h"
    "${PROJECT_SOURCE_DIR}/src/vm/platform/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/platform/*.h"
    "${PROJECT_SOURCE_DIR}/src/vm/profile/*.c"
    "${PROJECT_SOURCE_DIR}/src/vm/profile/*.h"
    "${PROJECT_SOURCE_DIR}/src/vdm/machine/*.c"
    "${PROJECT_SOURCE_DIR}/src/vdm/machine/*.h"
    "${PROJECT_SOURCE_DIR}/src/vdm/platform/*.c"
    "${PROJECT_SOURCE_DIR}/src/vdm/platform/*.h"
    "${PROJECT_SOURCE_DIR}/src/vdm/profile/*.c"
    "${PROJECT_SOURCE_DIR}/src/vdm/profile/*.h")
foreach(file IN LISTS peer_sources)
    file(READ "${file}" source)
    if(source MATCHES "#include[ \t]+\"core/product/")
        message(FATAL_ERROR "M5 T234 peer imports core/product: ${file}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_source)
foreach(forbidden IN ITEMS
    "target_link_libraries(vm-machine PUBLIC\n    core-machine\n    vm-profile)"
    "target_link_libraries(vm-profile PUBLIC\n    core-product-utils)"
    "target_link_libraries(core-product-utils PUBLIC\n    core-machine-boundary)"
    "target_link_libraries(vm-platform-requests PUBLIC core-machine)"
    "target_link_libraries(vm-product PUBLIC\n    core-product-utils\n    core-machine)")
    string(FIND "${cmake_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "M5 T234 forbidden peer target edge remains: ${forbidden}")
    endif()
endforeach()

message(STATUS "M5 T526 core debugger completion boundary: OK")
