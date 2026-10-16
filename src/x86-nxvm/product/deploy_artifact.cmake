if(NOT DEFINED PROJECT_ARTIFACT_PATH OR
   NOT DEFINED PROJECT_ARTIFACT_ARCHITECTURE OR
   NOT DEFINED PROJECT_ARTIFACT_FILENAME OR
   NOT DEFINED PROJECT_ARTIFACT_DIRECTORY)
    message(FATAL_ERROR "Current artifact deployment inputs are required.")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/verify_artifact_architecture.cmake")
set(project_artifact_directory "${PROJECT_ARTIFACT_DIRECTORY}")
file(MAKE_DIRECTORY "${project_artifact_directory}")
file(COPY_FILE "${PROJECT_ARTIFACT_PATH}"
    "${project_artifact_directory}/${PROJECT_ARTIFACT_FILENAME}"
    ONLY_IF_DIFFERENT RESULT project_artifact_copy_result)
if(NOT project_artifact_copy_result STREQUAL "0")
    message(FATAL_ERROR "Current artifact deployment failed: ${project_artifact_copy_result}")
endif()
