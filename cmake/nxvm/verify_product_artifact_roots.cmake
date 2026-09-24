if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

set(project_artifact_roots
    "assets/nxvm"
    "assets/mynes")
foreach(project_artifact_root IN LISTS project_artifact_roots)
    if(NOT IS_DIRECTORY "${PROJECT_SOURCE_DIR}/${project_artifact_root}")
        message(FATAL_ERROR "Missing product artifact root: ${project_artifact_root}")
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
    "cmake/nxvm/deploy_current_artifact.cmake"
    "cmake/nxvm/verify_current_artifact_optimized.cmake"
    "cmake/nxvm/verify_t533_integration_ini_boundary.cmake"
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

message(STATUS "M6:T41:S10:PRODUCT-ARTIFACT-ROOTS:OK")