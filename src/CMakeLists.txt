if (   NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/tinyexr/tinyexr.h"
    OR NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/lodepng/lodepng.cpp"
    OR NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/tclap/include")
    message(SEND_ERROR "Submodules missing!")
    message(FATAL_ERROR "Try git submodule init && git submodule update")
endif()

add_executable(diff-exr 
    main.cpp 
    "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/lodepng/lodepng.cpp"
    )

target_include_directories(diff-exr PRIVATE 
    "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/tinyexr"
    "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/lodepng"
    "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/tclap/include"
    "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/data"
    )

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(diff-exr PUBLIC OpenMP::OpenMP_CXX)
endif()


if (MSVC)
    target_compile_options(diff-exr PUBLIC /W3)
else()
    target_compile_options(diff-exr PUBLIC -Wall -Wextra -Wpedantic)
endif()