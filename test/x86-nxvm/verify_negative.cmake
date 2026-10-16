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
    if(result EQUAL 0 OR NOT error MATCHES "Forbidden x86-nxvm edge")
        message(FATAL_ERROR "x86-nxvm dependency gate accepted ${suffix}: ${error}")
    endif()
endfunction()
expect_rejected("console" "target_link_libraries(x86-nxvm-product PRIVATE console)")
expect_rejected("chip-product" "target_link_libraries(x86-nxvm-chip-video PRIVATE x86-nxvm-product)")
expect_rejected("chip-peer" "target_link_libraries(x86-nxvm-chip-pit825x PRIVATE x86-nxvm-chip-pic8259)")
function(expect_source_rejected suffix source expected)
    set(probe "${PROBE_ROOT}/${suffix}")
    file(REMOVE_RECURSE "${probe}")
    file(COPY "${CORPUS_ROOT}/" DESTINATION "${probe}")
    file(WRITE "${probe}/chips/pit825x/probe.c" "${source}")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DCORPUS_ROOT=${probe}"
        -P "${probe}/verify_corpus.cmake" RESULT_VARIABLE result
        OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(result EQUAL 0 OR NOT "${output}${error}" MATCHES "${expected}")
        message(FATAL_ERROR "Expected rejection ${expected}, got ${result}: ${output}${error}")
    endif()
endfunction()
expect_source_rejected("chip-machine"
    "#include \"x86-nxvm/machine/machine_interface.h\"\n" "Forbidden x86-nxvm edge")
expect_source_rejected("outer-source"
    "#include \"outer/product/main.c\"\n" "Noncanonical")
expect_source_rejected("retired-fdd"
    "void x86_nxvm_machine_fdd_read_byte(void);\n" "Retired FDD access path")

# Test ownership includes nested registration files and component fixtures.
set(probe "${PROBE_ROOT}/test-ownership")
file(MAKE_DIRECTORY "${probe}/board-at" "${probe}/nested")
file(WRITE "${probe}/CMakeLists.txt" "include(\"\${CMAKE_CURRENT_LIST_DIR}/nested/tests.cmake\")\n")
foreach(build IN ITEMS
        "TARGET_LINK_LIBRARIES(probe PRIVATE app-probe)"
        "shared_register_test(x86-nxvm probe app-probe)")
    file(WRITE "${probe}/nested/tests.cmake" "${build}\n")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DTEST_ROOT=${probe}" -DTEST_LAYER=x86-nxvm
        -P "${CMAKE_CURRENT_LIST_DIR}/../verify_test_boundaries.cmake"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(result EQUAL 0 OR NOT "${output}${error}" MATCHES "Outward test link")
        message(FATAL_ERROR "Nested test link not rejected: ${output}${error}")
    endif()
endforeach()
file(WRITE "${probe}/nested/tests.cmake" "")
foreach(header IN ITEMS x86-nxvm/board-base/pic_bus_interface.h ../board-base/pic_fixture.h)
    file(WRITE "${probe}/board-at/probe.c" "#include \"${header}\"\n")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DTEST_ROOT=${probe}" -DTEST_LAYER=x86-nxvm
        -P "${CMAKE_CURRENT_LIST_DIR}/../verify_test_boundaries.cmake"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(result EQUAL 0 OR NOT "${output}${error}" MATCHES "Outward test component include")
        message(FATAL_ERROR "Outward fixture not rejected: ${output}${error}")
    endif()
endforeach()
file(REMOVE "${probe}/board-at/probe.c")
message(STATUS "x86-nxvm dependency negative coverage: OK")
