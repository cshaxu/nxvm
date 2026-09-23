if(NOT DEFINED PROJECT_ARTIFACT_PATH OR
   NOT DEFINED PROJECT_ARTIFACT_ARCHITECTURE OR
   NOT DEFINED PROJECT_ARTIFACT_FILENAME OR
   NOT DEFINED PROJECT_PRODUCT_PROFILE OR
   NOT DEFINED PROJECT_RUNTIME_INI_SOURCE_PATH OR
   NOT DEFINED PROJECT_SOURCE_DIR OR
   NOT DEFINED PROJECT_BINARY_DIR)
    message(FATAL_ERROR "Current artifact deployment inputs are required.")
endif()

include("${PROJECT_SOURCE_DIR}/cmake/nxvm/verify_current_artifact_architecture.cmake")

set(project_runtime_ini_contents "")
file(READ "${PROJECT_RUNTIME_INI_SOURCE_PATH}" project_runtime_ini_contents)
string(REPLACE "../../../nxvm-assets/" "../../../../nxvm-assets/"
    project_runtime_ini_contents "${project_runtime_ini_contents}")

set(project_artifact_directory
    "${PROJECT_SOURCE_DIR}/assets/binary-nxvm/${PROJECT_PRODUCT_PROFILE}")
file(MAKE_DIRECTORY "${project_artifact_directory}")
file(COPY_FILE "${PROJECT_ARTIFACT_PATH}"
    "${project_artifact_directory}/${PROJECT_ARTIFACT_FILENAME}"
    ONLY_IF_DIFFERENT RESULT project_artifact_copy_result)
if(NOT project_artifact_copy_result STREQUAL "0")
    message(FATAL_ERROR "Current artifact deployment failed: ${project_artifact_copy_result}")
endif()
file(WRITE "${project_artifact_directory}/NXVM.ini"
    "${project_runtime_ini_contents}")
