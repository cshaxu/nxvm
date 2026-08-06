if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIR is required")
endif()

file(READ "${PROJECT_SOURCE_DIR}/src/vm/composition/session/lifecycle.c" source)

foreach(operation IN ITEMS request_stop join finalize)
    string(REGEX MATCHALL "vm_platform_run_handle_${operation}" calls "${source}")
    list(LENGTH calls call_count)
    if(NOT call_count EQUAL 1)
        message(FATAL_ERROR "Composition lifecycle must have one ${operation} backend call")
    endif()
endforeach()

foreach(helper IN ITEMS vm_session_platform_request_stop
        vm_session_platform_join_and_finalize)
    string(FIND "${source}" "${helper}" helper_position)
    if(helper_position EQUAL -1)
        message(FATAL_ERROR "Composition lifecycle helper is missing: ${helper}")
    endif()
endforeach()

message("M5:T252:S3:COMPOSITION-LIFECYCLE-BOUNDARY:OK")
