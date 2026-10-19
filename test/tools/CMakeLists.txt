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
