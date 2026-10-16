if(NOT DEFINED CORPUS_ROOT OR NOT DEFINED PROBE_ROOT)
    message(FATAL_ERROR "CORPUS_ROOT and PROBE_ROOT are required")
endif()
function(expect_rejected suffix appended)
    set(probe "${PROBE_ROOT}/${suffix}")
    file(REMOVE_RECURSE "${probe}")
    file(COPY "${CORPUS_ROOT}/" DESTINATION "${probe}")
    file(APPEND "${probe}/CMakeLists.txt" "\n${appended}\n")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DCORPUS_ROOT=${probe}"
        -P "${probe}/verify_corpus.cmake" RESULT_VARIABLE result
        OUTPUT_QUIET ERROR_VARIABLE error)
    if(result EQUAL 0 OR NOT error MATCHES "Forbidden x86-ibmpc edge")
        message(FATAL_ERROR "x86-ibmpc dependency gate accepted ${suffix}: ${error}")
    endif()
endfunction()
expect_rejected("base" "target_link_libraries(x86-ibmpc-product INTERFACE base)")
foreach(path IN ITEMS probe.c product/nested/probe.c)
    set(probe "${PROBE_ROOT}/layout")
    file(MAKE_DIRECTORY "${probe}/product/nested")
    file(WRITE "${probe}/CMakeLists.txt" "")
    file(WRITE "${probe}/${path}" "/* Unowned source must not be skipped. */\n")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DCORPUS_ROOT=${probe}"
        -P "${CORPUS_ROOT}/verify_corpus.cmake" RESULT_VARIABLE result
        OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(result EQUAL 0 OR NOT "${output}${error}" MATCHES "Noncanonical x86-ibmpc source")
        message(FATAL_ERROR "Unowned source was not rejected: ${output}${error}")
    endif()
    file(REMOVE "${probe}/${path}")
endforeach()
message(STATUS "x86-ibmpc dependency negative coverage: OK")

# Included and nested build descriptions obey the same package boundaries.
set(probe "${PROBE_ROOT}/build-bypass")
file(REMOVE_RECURSE "${probe}")
file(COPY "${CORPUS_ROOT}/" DESTINATION "${probe}")
file(MAKE_DIRECTORY "${probe}/nested")
foreach(build_file IN ITEMS nested/CMakeLists.txt nested/links.cmake)
    foreach(statement IN ITEMS
            "TaRgEt_LiNk_LiBrArIeS(x86-ibmpc-product PRIVATE base)"
            "LINK_LIBRARIES(base)"
            "set_property(TARGET x86-ibmpc-product APPEND PROPERTY LINK_LIBRARIES\n base)"
            "set_target_properties(x86-ibmpc-product PROPERTIES INTERFACE_LINK_LIBRARIES\n base)")
        file(APPEND "${probe}/CMakeLists.txt" "\ninclude(${build_file})\n")
        file(WRITE "${probe}/${build_file}" "${statement}\n")
        execute_process(COMMAND "${CMAKE_COMMAND}" "-DCORPUS_ROOT=${probe}"
            -P "${probe}/verify_corpus.cmake" RESULT_VARIABLE result
            OUTPUT_VARIABLE output ERROR_VARIABLE error)
        if(result EQUAL 0 OR NOT "${output}${error}" MATCHES
                "Forbidden x86-ibmpc edge|Use explicit x86-ibmpc target_link_libraries")
            message(FATAL_ERROR "Build bypass not rejected: ${statement}: ${output}${error}")
        endif()
    endforeach()
    file(REMOVE "${probe}/${build_file}")
endforeach()
set(probe "${PROBE_ROOT}/raw-null")
file(MAKE_DIRECTORY "${probe}/product")
file(WRITE "${probe}/product/probe.c" "void *pointer = NULL;\n")
execute_process(COMMAND "${CMAKE_COMMAND}" "-DCORPUS_ROOT=${probe}"
    -P "${CORPUS_ROOT}/verify_corpus.cmake" RESULT_VARIABLE result
    OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT "${output}${error}" MATCHES "Raw CRT vocabulary")
    message(FATAL_ERROR "Raw NULL not rejected: ${output}${error}")
endif()
