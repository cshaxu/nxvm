if(NOT DEFINED PRODUCT_ROOT OR NOT DEFINED PROBE_ROOT)
    message(FATAL_ERROR "PRODUCT_ROOT and PROBE_ROOT are required")
endif()

function(expect_rejected suffix appended)
    set(probe "${PROBE_ROOT}/${suffix}")
    file(REMOVE_RECURSE "${probe}")
    file(COPY "${PRODUCT_ROOT}/" DESTINATION "${probe}")
    file(APPEND "${probe}/CMakeLists.txt" "\n${appended}\n")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DPRODUCT_ROOT=${probe}"
        -P "${probe}/verify_corpus.cmake" RESULT_VARIABLE result
        OUTPUT_QUIET ERROR_VARIABLE error)
    if(result EQUAL 0 OR NOT error MATCHES "Forbidden Product edge")
        message(FATAL_ERROR "Product dependency gate accepted ${suffix}: ${error}")
    endif()
endfunction()

expect_rejected("outer-product" "target_link_libraries(product-xasm32 PRIVATE product-debug)")
expect_rejected("outer-emulator" "target_link_libraries(product-xasm32 PRIVATE emulator-machine)")
message(STATUS "Product dependency negative coverage: OK")
