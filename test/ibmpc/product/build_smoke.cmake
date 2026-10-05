cmake_minimum_required(VERSION 3.23)
file(MAKE_DIRECTORY "${PROBE_ROOT}")
set(input "${PROBE_ROOT}/input")
set(output "${PROBE_ROOT}/firmware.c")
file(WRITE "${input}" "AB")
set(arguments "-DINPUT_BIOS_0=${input}" -DINPUT_BIOS_1=LIB_NULL
    -DINPUT_VIDEO=LIB_NULL -DINPUT_CMOS=LIB_NULL -DINPUT_FONT=LIB_NULL "-DOUTPUT=${output}")
execute_process(COMMAND "${CMAKE_COMMAND}" ${arguments}
    -P "${IBMPC_ROOT}/product/embed_firmware.cmake" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Firmware embedding failed")
endif()
file(READ "${output}" generated)
if(NOT generated MATCHES "0x41,0x42," OR NOT generated MATCHES "const vm_machine_assets vm_app_firmware")
    message(FATAL_ERROR "Firmware byte/value contract changed")
endif()
file(WRITE "${input}" "")
execute_process(COMMAND "${CMAKE_COMMAND}" ${arguments}
    -P "${IBMPC_ROOT}/product/embed_firmware.cmake" RESULT_VARIABLE result
    OUTPUT_QUIET ERROR_QUIET)
if(result EQUAL 0)
    message(FATAL_ERROR "Empty firmware input accepted")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -DPROJECT_BUILD_TYPE=Debug
    -P "${IBMPC_ROOT}/product/verify_artifact_optimized.cmake"
    RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET)
if(result EQUAL 0)
    message(FATAL_ERROR "Non-Release publication accepted")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -DPROJECT_BUILD_TYPE=Release
    -P "${IBMPC_ROOT}/product/verify_artifact_optimized.cmake" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Release publication rejected")
endif()
file(REMOVE "${input}" "${output}")
message(STATUS "ibmpc build byte embedding and publication contract: OK")
