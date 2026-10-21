add_executable(myraidenii-unit-product-config-smoke
    config_smoke.c
    "${CMAKE_CURRENT_LIST_DIR}/../../../../src/myraidenii/product/config.c")
target_include_directories(myraidenii-unit-product-config-smoke PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../../../..")
target_link_libraries(myraidenii-unit-product-config-smoke PRIVATE types)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(myraidenii-unit-product-config-smoke PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
elseif(MSVC)
    target_compile_options(myraidenii-unit-product-config-smoke PRIVATE /UNDEBUG /W4 /WX)
endif()
add_test(NAME myraidenii.unit.product.config
    COMMAND myraidenii-unit-product-config-smoke)
set_tests_properties(myraidenii.unit.product.config PROPERTIES
    LABELS "unit;myraidenii" TIMEOUT 30)

add_executable(myraidenii-unit-product-composition-smoke
    composition_smoke.c
    "${CMAKE_CURRENT_LIST_DIR}/../../../../src/myraidenii/product/composition.c")
target_include_directories(myraidenii-unit-product-composition-smoke PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../../../..")
target_link_libraries(myraidenii-unit-product-composition-smoke PRIVATE types)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(myraidenii-unit-product-composition-smoke PRIVATE
        -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
elseif(MSVC)
    target_compile_options(myraidenii-unit-product-composition-smoke PRIVATE /UNDEBUG /W4 /WX)
endif()
add_test(NAME myraidenii.unit.product.composition
    COMMAND myraidenii-unit-product-composition-smoke)
set_tests_properties(myraidenii.unit.product.composition PROPERTIES
    LABELS "unit;myraidenii" TIMEOUT 30)
