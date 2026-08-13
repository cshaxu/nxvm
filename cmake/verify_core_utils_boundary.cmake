if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(utils_dir "${PROJECT_SOURCE_DIR}/src/core/utils")
foreach(file IN ITEMS wait.c wait.h wait_provider.h)
    if(NOT EXISTS "${utils_dir}/${file}")
        message(FATAL_ERROR "M5 T234 missing core/utils contract: ${file}")
    endif()
endforeach()

file(GLOB_RECURSE utils_sources "${utils_dir}/*.c" "${utils_dir}/*.h")
foreach(file IN LISTS utils_sources)
    file(READ "${file}" source)
    if(source MATCHES "#include[ \t]+\"(core/(machine|platform|product)|vm/|vdm/)")
        message(FATAL_ERROR "M5 T234 core/utils imports a policy owner: ${file}")
    endif()
endforeach()

foreach(file IN ITEMS
    "${PROJECT_SOURCE_DIR}/src/core/product/wait.c"
    "${PROJECT_SOURCE_DIR}/src/core/product/wait.h"
    "${PROJECT_SOURCE_DIR}/src/core/product/wait_provider.h")
    if(EXISTS "${file}")
        message(FATAL_ERROR "M5 T234 retained product-owned wait path: ${file}")
    endif()
endforeach()

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
    "core-product-utils\n    core-platform)"
    "target_link_libraries(vm-platform-requests PUBLIC core-machine)"
    "target_link_libraries(vm-product PUBLIC\n    core-product-utils\n    core-machine)")
    string(FIND "${cmake_source}" "${forbidden}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "M5 T234 forbidden peer target edge remains: ${forbidden}")
    endif()
endforeach()

message(STATUS "M5 T234 core utility boundary: OK")
