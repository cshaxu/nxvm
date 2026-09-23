file(GLOB_RECURSE production_sources
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.h")

foreach(source IN LISTS production_sources)
    file(READ "${source}" contents)
    if(contents MATCHES "STD_SPRINTF|vsprintf[ \t\r\n]*\\(|sprintf[ \t\r\n]*\\(")
        message(FATAL_ERROR "Unbounded formatting vocabulary remains in ${source}")
    endif()
endforeach()

message("M5:T279:S3:BOUNDED-FORMAT-SOURCE:OK")
