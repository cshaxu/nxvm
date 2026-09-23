# Derived from the owner-provided MyNES root CMake entry point.  The root
# repository has already configured C11 and the neutral Lib/Common corpora;
# this module owns only the MyNES product traversal.

if(DEFINED MYNES_EXPECTED_POINTER_SIZE AND
        NOT CMAKE_SIZEOF_VOID_P EQUAL MYNES_EXPECTED_POINTER_SIZE)
    message(FATAL_ERROR "Selected compiler does not match the requested MyNES architecture")
endif()

# Keep MyNES's standalone build diagnostics when it is configured as one
# product in this repository rather than as its former repository root.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(mynes_enable_strict_warnings target)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    elseif(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    endif()
endfunction()

add_subdirectory("${CMAKE_SOURCE_DIR}/src/app-mynes/core" app-mynes-core)
add_subdirectory("${CMAKE_SOURCE_DIR}/src/app-mynes/product" app-mynes-product)
add_subdirectory("${CMAKE_SOURCE_DIR}/test/app-mynes/unit/core" test-app-mynes-unit-core)
add_subdirectory("${CMAKE_SOURCE_DIR}/test/app-mynes/unit/product" test-app-mynes-unit-product)
add_subdirectory("${CMAKE_SOURCE_DIR}/test/app-mynes/integration" test-app-mynes-integration)

# Preserve the upstream exception for its intentionally exhaustive CMake
# layout self-test.  Ordinary MyNES tests retain their imported timeout.
set_property(TEST library.types-layout-selftest DIRECTORY
    "${CMAKE_SOURCE_DIR}/test/lib" PROPERTY TIMEOUT 180)

# Imported MyNES test CMake currently owns direct CTest registration.  Keep a
# product-local execution route without re-registering those cases through
# NXVM's single-product test helper.
add_custom_target(mynes-test-binaries)
foreach(mynes_test_directory IN ITEMS
    "${CMAKE_SOURCE_DIR}/test/app-mynes/unit/core"
    "${CMAKE_SOURCE_DIR}/test/app-mynes/unit/product"
    "${CMAKE_SOURCE_DIR}/test/app-mynes/integration")
    get_property(mynes_test_targets DIRECTORY "${mynes_test_directory}"
        PROPERTY BUILDSYSTEM_TARGETS)
    add_dependencies(mynes-test-binaries ${mynes_test_targets})
endforeach()

add_custom_target(run-mynes-unit-tests
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}"
        --output-on-failure --no-tests=error -R "^mynes\\.(core|app)\\."
    DEPENDS mynes-test-binaries
    COMMENT "Executing MyNES unit tests"
    VERBATIM)

add_custom_target(run-mynes-integration-tests
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}"
        --output-on-failure --no-tests=error -R "^mynes\\.integration\\."
    DEPENDS mynes-test-binaries
    COMMENT "Executing MyNES integration tests"
    VERBATIM)

add_custom_target(run-mynes-tests
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${CMAKE_BINARY_DIR}"
        --output-on-failure --no-tests=error -R "^mynes\\."
    DEPENDS mynes-test-binaries
    COMMENT "Executing MyNES product tests"
    VERBATIM)

# The imported capture-contract fixture holds one maximum-size Window frame on
# its test stack.  Preserve the MyNES CMake behavior without changing the
# shared Lib source.
if(WIN32 AND MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_options(lib-test-kvm_window_capture_contract PRIVATE
        "-Wl,--stack,4194304")
endif()
