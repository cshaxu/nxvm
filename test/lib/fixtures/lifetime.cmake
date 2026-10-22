# Exercise the real fixture normally, then its isolated stop/join faults.
execute_process(COMMAND "${PROGRAM}" RESULT_VARIABLE result)
if(NOT result STREQUAL "0")
    message(FATAL_ERROR "Normal lifecycle fixture failed: ${result}")
endif()
foreach(fault RANGE 1 4)
    execute_process(COMMAND "${PROGRAM}" "${fault}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(NOT result STREQUAL "6" OR NOT output STREQUAL "fatal lifetime boundary\n")
        message(FATAL_ERROR "Fatal boundary ${fault}: ${result}; ${output}; ${error}")
    endif()
endforeach()
