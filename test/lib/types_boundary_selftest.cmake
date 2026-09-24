if(NOT DEFINED TEST_WORK_DIR)
    message(FATAL_ERROR "TEST_WORK_DIR is required")
endif()
set(probe "${TEST_WORK_DIR}/types-boundary-probe")
file(MAKE_DIRECTORY "${probe}")
function(check_source source expected)
    file(WRITE "${probe}/probe.c" "${source}\n")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DTEST_ROOTS=${probe}"
        -P "${CMAKE_CURRENT_LIST_DIR}/verify_types_boundary.cmake"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(expected STREQUAL "")
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Valid Types use rejected: ${output}${error}")
        endif()
    elseif(result EQUAL 0 OR NOT "${error}" MATCHES "${expected}")
        message(FATAL_ERROR "Expected rejection ${expected}: ${output}${error}")
    endif()
endfunction()

foreach(type IN ITEMS LONG LONGLONG LPCWSTR SIZE_T SHORT PCONSOLE_SCREEN_BUFFER_INFO
        PCONSOLE_SCREEN_BUFFER_INFOEX PINPUT_RECORD PRAWINPUTDEVICE PUINT
        PCRAWINPUTDEVICE PSMALL_RECT SC_SIZE SC_MOVE SC_KEYMENU)
    check_source("${type} value;" "bypasses Types:.*${type}")
endforeach()
check_source("#include <windows.h>" "header bypasses Types")
check_source("void f(void) { SetEvent(0); }" "function/macro bypasses Types")
foreach(call IN ITEMS AllocConsole FreeConsole FillConsoleOutputCharacterW
        CallWindowProcW EnumWindows InitializeCriticalSection ReleaseSemaphore
        TerminateProcess)
    check_source("void f(void) { ${call}(0); }" "function/macro bypasses Types")
endforeach()
check_source("#include \"lib/types/win32/test.h\"\nlib_win32_input_record *record;\nconst lib_win32_raw_input_device *device;\nlib_win32_uint *count;\n/* PINPUT_RECORD */\nconst char *text = \"PUINT\";" "")
file(REMOVE "${probe}/probe.c")
execute_process(COMMAND "${CMAKE_COMMAND}" "-DTEST_ROOTS=${probe}"
    -P "${CMAKE_CURRENT_LIST_DIR}/verify_types_boundary.cmake"
    RESULT_VARIABLE result ERROR_VARIABLE error OUTPUT_QUIET)
if(result EQUAL 0 OR NOT error MATCHES "contains no C/H sources")
    message(FATAL_ERROR "An empty scan must fail: ${error}")
endif()
message(STATUS "test Types boundary positive and negative cases verified")
