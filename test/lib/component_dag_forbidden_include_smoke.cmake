if(NOT DEFINED LIBRARY_ROOT OR NOT DEFINED LIBRARY_VERIFIER OR
   NOT DEFINED LIBRARY_EXPECTED_ERROR)
    message(FATAL_ERROR "Fixture root, verifier, and expected error are required")
endif()
execute_process(
    COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT:PATH=${LIBRARY_ROOT}
        -P "${LIBRARY_VERIFIER}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
if(result EQUAL 0)
    message(FATAL_ERROR "Forbidden source include passed the DAG gate")
endif()
if(NOT "${output}${error}" MATCHES "${LIBRARY_EXPECTED_ERROR}")
    message(FATAL_ERROR "Fixture failed for the wrong reason: ${output}${error}")
endif()
