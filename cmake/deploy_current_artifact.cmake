if(NOT DEFINED PROJECT_ARTIFACT_PATH OR
   NOT DEFINED PROJECT_ARTIFACT_ARCHITECTURE OR
   NOT DEFINED PROJECT_ARTIFACT_FILENAME OR
   NOT DEFINED PROJECT_SOURCE_DIR OR
   NOT DEFINED PROJECT_BINARY_DIR)
    message(FATAL_ERROR "Current artifact deployment inputs are required.")
endif()

include("${PROJECT_SOURCE_DIR}/cmake/verify_current_artifact_architecture.cmake")

foreach(project_artifact_directory IN ITEMS
    "${PROJECT_BINARY_DIR}/output"
    "${PROJECT_SOURCE_DIR}/build/output"
    "${PROJECT_SOURCE_DIR}/assets/sessions")
    file(MAKE_DIRECTORY "${project_artifact_directory}")
    file(COPY_FILE "${PROJECT_ARTIFACT_PATH}"
        "${project_artifact_directory}/${PROJECT_ARTIFACT_FILENAME}"
        ONLY_IF_DIFFERENT RESULT project_artifact_copy_result)
    if(NOT project_artifact_copy_result STREQUAL "0")
        message(FATAL_ERROR "Current artifact deployment failed: ${project_artifact_copy_result}")
    endif()
endforeach()
