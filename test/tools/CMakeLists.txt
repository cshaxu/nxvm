add_test(NAME repository.line-endings
    COMMAND "${CMAKE_COMMAND}" -DREPOSITORY_ROOT=${CMAKE_SOURCE_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_line_endings.cmake")
set_tests_properties(repository.line-endings PROPERTIES LABELS "unit;repository" TIMEOUT 60)

if(POWERSHELL_EXECUTABLE)
    add_test(NAME repository.build-cleanup
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_CURRENT_SOURCE_DIR}/build_cleanup.ps1")
    set_tests_properties(repository.build-cleanup PROPERTIES LABELS "unit;repository" TIMEOUT 60)
    add_test(NAME repository.build-environment
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
            -File "${CMAKE_CURRENT_SOURCE_DIR}/build_environment.ps1"
            -Compiler "${CMAKE_C_COMPILER}" -CompilerId "${CMAKE_C_COMPILER_ID}"
            -Architecture "${PROJECT_ARTIFACT_ARCHITECTURE}")
    set_tests_properties(repository.build-environment PROPERTIES LABELS "unit;repository" TIMEOUT 180)
endif()

if(WIN32)
    add_test(NAME repository.product-publication
        COMMAND "${CMAKE_COMMAND}" -DREPOSITORY_ROOT=${CMAKE_SOURCE_DIR}
            -DTEST_WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -DINPUT=$<TARGET_FILE:lib-test-base-win32-keyboard_decode>
            -P "${CMAKE_CURRENT_SOURCE_DIR}/publish_product.cmake")
    set_tests_properties(repository.product-publication PROPERTIES
        LABELS "unit;repository" TIMEOUT 60)
endif()
