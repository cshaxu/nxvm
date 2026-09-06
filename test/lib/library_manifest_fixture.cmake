if(NOT DEFINED LIBRARY_ROOT OR NOT DEFINED VERIFY_SCRIPT OR NOT DEFINED WORK_ROOT)
    message(FATAL_ERROR "library manifest fixture needs LIBRARY_ROOT, VERIFY_SCRIPT and WORK_ROOT")
endif()

file(REMOVE_RECURSE "${WORK_ROOT}")
file(COPY "${LIBRARY_ROOT}/" DESTINATION "${WORK_ROOT}/library")
set(fixture_root "${WORK_ROOT}/library")

function(expect_manifest result_name expected_text)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -DLIBRARY_ROOT=${fixture_root} -P "${VERIFY_SCRIPT}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error)
    if(${result_name})
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "expected clean manifest: ${output}${error}")
        endif()
    elseif(result EQUAL 0)
        message(FATAL_ERROR "expected manifest failure containing: ${expected_text}")
    elseif(NOT "${output}${error}" MATCHES "${expected_text}")
        message(FATAL_ERROR "unexpected manifest failure: ${output}${error}")
    endif()
endfunction()

expect_manifest(TRUE "")

file(REMOVE "${fixture_root}/host/clock.h")
expect_manifest(FALSE "missing library source: host/clock.h")
file(COPY "${LIBRARY_ROOT}/host/clock.h" DESTINATION "${fixture_root}/host")

file(WRITE "${fixture_root}/unexpected.txt" "not part of the corpus\n")
expect_manifest(FALSE "library manifest missing path: unexpected.txt")
file(REMOVE "${fixture_root}/unexpected.txt")

file(APPEND "${fixture_root}/host/clock.c" "\n")
expect_manifest(FALSE "library manifest mismatch: host/clock.c")

file(REMOVE_RECURSE "${WORK_ROOT}")
