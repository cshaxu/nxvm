if(NOT DEFINED X86_ROOT OR NOT DEFINED PROBE_ROOT)
    message(FATAL_ERROR "X86_ROOT and PROBE_ROOT are required")
endif()

function(expect_rejected suffix appended)
    set(probe "${PROBE_ROOT}/${suffix}")
    file(REMOVE_RECURSE "${probe}")
    file(COPY "${X86_ROOT}/" DESTINATION "${probe}")
    file(APPEND "${probe}/CMakeLists.txt" "\n${appended}\n")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DX86_ROOT=${probe}"
        -P "${probe}/verify_corpus.cmake" RESULT_VARIABLE result
        OUTPUT_QUIET ERROR_VARIABLE error)
    if(result EQUAL 0 OR NOT error MATCHES "Forbidden x86 edge")
        message(FATAL_ERROR "x86 dependency gate accepted ${suffix}: ${error}")
    endif()
endfunction()

expect_rejected("outer-product" "target_link_libraries(x86-xasm32 PRIVATE x86-debug)")
expect_rejected("outer-emulator" "target_link_libraries(x86-xasm32 PRIVATE emulator-machine)")
message(STATUS "x86 dependency negative coverage: OK")
