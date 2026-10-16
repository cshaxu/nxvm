if(NOT DEFINED CORPUS_ROOT OR NOT DEFINED PROBE_ROOT)
    message(FATAL_ERROR "CORPUS_ROOT and PROBE_ROOT required")
endif()
get_filename_component(CORPUS_ROOT "${CORPUS_ROOT}" ABSOLUTE)
get_filename_component(PROBE_ROOT "${PROBE_ROOT}" ABSOLUTE)
file(MAKE_DIRECTORY "${PROBE_ROOT}/cpu")
file(WRITE "${PROBE_ROOT}/CMakeLists.txt" "")
function(expect_rejected source marker)
    file(WRITE "${PROBE_ROOT}/cpu/probe.c" "${source}")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DCORPUS_ROOT=${PROBE_ROOT}"
        -P "${CORPUS_ROOT}/verify_corpus.cmake" RESULT_VARIABLE result
        OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(result EQUAL 0 OR NOT "${output}${error}" MATCHES "${marker}")
        message(FATAL_ERROR "Expected ${marker}: ${output}${error}")
    endif()
endfunction()
expect_rejected("#include \"outer/probe_interface.h\"\n" "Forbidden x86-core include")
expect_rejected("void *p = malloc(1);\n" "Raw CRT/platform")
expect_rejected("#include \"x86-core/machine/machine_interface.h\"\n"
    "CPU/FPU cannot depend on Machine")
expect_rejected("#include \"x86-core/fpu/fpu.h\"\n" "Private chip header")
file(REMOVE "${PROBE_ROOT}/cpu/probe.c")
file(MAKE_DIRECTORY "${PROBE_ROOT}/machine")
file(WRITE "${PROBE_ROOT}/machine/probe.c"
    "#include \"x86-core/cpu/cpu.h\"\n")
execute_process(COMMAND "${CMAKE_COMMAND}" "-DCORPUS_ROOT=${PROBE_ROOT}"
    -P "${CORPUS_ROOT}/verify_corpus.cmake" RESULT_VARIABLE result
    OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT "${output}${error}" MATCHES "Private chip header")
    message(FATAL_ERROR "Private CPU header not rejected: ${output}${error}")
endif()
file(REMOVE "${PROBE_ROOT}/machine/probe.c")
file(WRITE "${PROBE_ROOT}/CMakeLists.txt" "target_link_libraries(x86-core-cpu PRIVATE app-probe)\n")
expect_rejected("" "Forbidden x86-core build dependency")
foreach(link IN ITEMS
        "target_link_libraries(x86-core-fpu PRIVATE x86-core-cpu)"
        "TARGET_LINK_LIBRARIES(x86-core-cpu PRIVATE base)")
    file(WRITE "${PROBE_ROOT}/CMakeLists.txt" "${link}\n")
    expect_rejected("" "Forbidden x86-core build dependency")
endforeach()
file(WRITE "${PROBE_ROOT}/CMakeLists.txt" "include(nested.cmake)\n")
file(WRITE "${PROBE_ROOT}/nested.cmake" "target_link_libraries(x86-core-cpu PRIVATE storage)\n")
expect_rejected("" "Forbidden x86-core build dependency")
file(REMOVE "${PROBE_ROOT}/nested.cmake")
file(WRITE "${PROBE_ROOT}/CMakeLists.txt" "")
file(WRITE "${PROBE_ROOT}/MANIFEST.sha256" "# sha256-manifest-v1\n")
execute_process(COMMAND "${CMAKE_COMMAND}" "-DCORPUS_ROOT=${PROBE_ROOT}"
    -P "${CORPUS_ROOT}/verify_manifest.cmake" RESULT_VARIABLE result
    OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT "${output}${error}" MATCHES "manifest missing path")
    message(FATAL_ERROR "Unlisted source not rejected: ${output}${error}")
endif()
message(STATUS "x86-core verifier negatives: OK")
