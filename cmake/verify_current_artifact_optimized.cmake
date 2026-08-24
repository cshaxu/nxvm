if(NOT DEFINED PROJECT_BUILD_TYPE OR
   NOT PROJECT_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(FATAL_ERROR
        "Current NXVM artifacts may be published to build/output only from RelWithDebInfo.")
endif()
