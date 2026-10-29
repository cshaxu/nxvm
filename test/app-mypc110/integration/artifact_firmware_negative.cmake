set(missing_bios "${CMAKE_CURRENT_BINARY_DIR}/mypc110-absent-firmware-${PROJECT_ARTIFACT_ARCHITECTURE}.bin")
file(WRITE "${missing_bios}" "not a PC110 BIOS: absent fixture 6ec073d4")
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPROJECT_ARTIFACT_PATH=${PROJECT_ARTIFACT_PATH}"
    "-DPROJECT_ARTIFACT_ARCHITECTURE=${PROJECT_ARTIFACT_ARCHITECTURE}"
    "-DDEPLOYED_PATH=${DEPLOYED_PATH}"
    "-DEXPECTED_BIOS=${missing_bios}"
    "-DOTHER_BIOS=${OTHER_BIOS}"
    "-DFONT_FILE=${FONT_FILE}"
    "-DSOURCE_ROOT=${SOURCE_ROOT}"
    -P "${CMAKE_CURRENT_LIST_DIR}/artifact_firmware.cmake"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
file(REMOVE "${missing_bios}")
if(result EQUAL 0 OR NOT error MATCHES "PC110 artifact is missing EXPECTED_BIOS")
    message(FATAL_ERROR "Wrong firmware was not rejected for the expected reason: ${output} ${error}")
endif()
message(STATUS "PC110 wrong-firmware rejection verified")
