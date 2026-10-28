execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPROJECT_ARTIFACT_PATH=${PROJECT_ARTIFACT_PATH}"
    "-DPROJECT_ARTIFACT_ARCHITECTURE=${PROJECT_ARTIFACT_ARCHITECTURE}"
    "-DDEPLOYED_PATH=${DEPLOYED_PATH}"
    "-DEXPECTED_BIOS=${EXPECTED_BIOS}"
    "-DOTHER_BIOS=${OTHER_BIOS}"
    "-DFONT_FILE=${FONT_FILE}"
    "-DSOURCE_ROOT=${SOURCE_ROOT}"
    -P "${CMAKE_CURRENT_LIST_DIR}/artifact_firmware.cmake"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "PC110 artifact is missing EXPECTED_BIOS")
    message(FATAL_ERROR "Wrong firmware was not rejected for the expected reason: ${output} ${error}")
endif()
message(STATUS "PC110 wrong-firmware rejection verified")
