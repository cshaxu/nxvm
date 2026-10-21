add_library(myraidenii-product STATIC config.c composition.c)
target_include_directories(myraidenii-product PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../..")
target_link_libraries(myraidenii-product PUBLIC audio-pcm base emulator-product myraidenii-machine
    storage types)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(myraidenii-product PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(MSVC)
    target_compile_options(myraidenii-product PRIVATE /W4 /WX)
endif()
