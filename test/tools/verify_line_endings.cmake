if(NOT DEFINED REPOSITORY_ROOT)
    message(FATAL_ERROR "REPOSITORY_ROOT is required")
endif()

execute_process(
    COMMAND git ls-files --eol --cached --others --exclude-standard
    WORKING_DIRECTORY "${REPOSITORY_ROOT}"
    RESULT_VARIABLE result OUTPUT_VARIABLE files ERROR_VARIABLE error)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Unable to inspect Git line endings: ${error}")
endif()

string(REPLACE "\n" ";" lines "${files}")
set(violations "")
foreach(line IN LISTS lines)
    if(line MATCHES "^i/(crlf|mixed)" OR line MATCHES " w/(crlf|mixed)")
        string(APPEND violations "\n${line}")
    endif()
endforeach()
if(violations)
    message(FATAL_ERROR "Repository text must use LF:${violations}")
endif()
message(STATUS "Repository text line endings are LF")
