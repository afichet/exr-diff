if (   NOT EXISTS "../3rdparty/tinyexr/tinyexr.h"
    OR NOT EXISTS "../3rdparty/lodepng/lodepng.cpp"
    OR NOT EXISTS "../3rdparty/tclap/include")
    message(SEND_ERROR "Submodules missing!")
    message(FATAL_ERROR "Try git submodule init && git submodule update")
endif()


add_executable(diff-exr 
    main.cpp 
    "../3rdparty/lodepng/lodepng.cpp"
    )

target_include_directories(diff-exr PRIVATE 
    "../3rdparty/tinyexr"
    "../3rdparty/lodepng"
    "../3rdparty/tclap/include"
    )