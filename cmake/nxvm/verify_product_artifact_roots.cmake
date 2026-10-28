if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(project_artifact_roots
    "assets/nxvm"
    "assets/my5160"
    "assets/my5170"
    "assets/mydeskpro386"
    "assets/mypc110"
    "assets/mynes")
foreach(project_artifact_root IN LISTS project_artifact_roots)
    if(NOT IS_DIRECTORY "${PROJECT_SOURCE_DIR}/${project_artifact_root}")
        message(FATAL_ERROR "Missing product artifact root: ${project_artifact_root}")
    endif()
endforeach()

file(GLOB pc110_executables RELATIVE "${PROJECT_SOURCE_DIR}/assets/mypc110"
    "${PROJECT_SOURCE_DIR}/assets/mypc110/*.exe")
set(expected_pc110_executables
    mypc110_x86.exe mypc110_x64.exe mypc110_tft_x86.exe mypc110_tft_x64.exe)
list(SORT pc110_executables)
list(SORT expected_pc110_executables)
if(NOT pc110_executables STREQUAL expected_pc110_executables)
    message(FATAL_ERROR "Expected exactly the four fixed MyPC110 releases")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/product/version.h" product_version_header)
if(NOT product_version_header MATCHES "X86_NXVM_PRODUCT_VERSION \"([0-9]+)\\.([0-9]+)\\.([0-9][0-9][0-9][0-9])\"")
    message(FATAL_ERROR "Core Product release version is missing")
endif()
set(product_version "${CMAKE_MATCH_1}_${CMAKE_MATCH_2}_${CMAKE_MATCH_3}")
foreach(app IN ITEMS my5160 my5170 mydeskpro386 nxvm)
    file(GLOB product_executables RELATIVE "${PROJECT_SOURCE_DIR}/assets/${app}"
        "${PROJECT_SOURCE_DIR}/assets/${app}/*.exe")
    set(expected_executables
        "${app}_${product_version}_x64.exe" "${app}_${product_version}_x86.exe")
    list(SORT product_executables)
    list(SORT expected_executables)
    if(NOT product_executables STREQUAL expected_executables)
        message(FATAL_ERROR "Expected only the current lowercase release pair: ${app}")
    endif()
endforeach()

foreach(retired_artifact_root IN ITEMS "assets/binary-nxvm" "assets/binary-mynes")
    if(EXISTS "${PROJECT_SOURCE_DIR}/${retired_artifact_root}")
        message(FATAL_ERROR "Retired product artifact root remains: ${retired_artifact_root}")
    endif()
endforeach()

set(project_live_path_authorities
    ".gitignore"
    "cmake/nxvm/NxvmProduct.cmake"
    "src/x86-nxvm/product/deploy_artifact.cmake"
    "src/x86-nxvm/product/verify_artifact_optimized.cmake"
    "cmake/nxvm/verify_integration_ini_boundary.cmake"
    "src/app-mynes/product/CMakeLists.txt"
    "tools/nxvm/README.md"
    "docs/rules/DOCUMENT.md"
    "docs/rules/EXECUTION.md"
    "docs/nxvm/design/ARCHITECTURE.md"
    "docs/nxvm/design/CODING.md")
foreach(project_live_path_authority IN LISTS project_live_path_authorities)
    file(READ "${PROJECT_SOURCE_DIR}/${project_live_path_authority}" project_live_path_content)
    if(project_live_path_content MATCHES "assets/binary-(nxvm|mynes)")
        message(FATAL_ERROR "Retired artifact path remains in ${project_live_path_authority}")
    endif()
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/src/x86-nxvm/product/deploy_artifact.cmake" project_deployment)
if(project_deployment MATCHES "file\\([ \t\r\n]*(WRITE|APPEND)[ \t\r\n]")
    message(FATAL_ERROR "EXE deployment must not write owner configuration files")
endif()

message(STATUS "PRODUCT-ARTIFACT-ROOTS:OK")
