if(NOT DEFINED PROJECT_BUILD_TYPE OR
   NOT PROJECT_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR
        "Current NXVM artifacts may be published to assets/binary-nxvm only from Release.")
endif()
