if(NOT DEFINED KVM_NAMING_ROOT OR NOT DEFINED KVM_NAMING_VERIFIER)
    message(FATAL_ERROR "KVM naming fixture root and verifier are required")
endif()
execute_process(
    COMMAND "${CMAKE_COMMAND}" -DKVM_NAMING_ROOT:PATH=${KVM_NAMING_ROOT}
        -P "${KVM_NAMING_VERIFIER}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
if(result EQUAL 0)
    message(FATAL_ERROR "Obsolete KVM naming fixture passed")
endif()
if(NOT "${output}${error}" MATCHES "Obsolete KVM spelling")
    message(FATAL_ERROR "KVM naming fixture failed for the wrong reason: ${output}${error}")
endif()
