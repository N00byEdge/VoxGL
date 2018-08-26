cmake_minimum_required(VERSION 3.5)
project(VoxGL)

file(GLOB SOURCE_FILES
        "VoxGL/*.cpp"
        "VoxGL/*.hpp"
        )

if(APPLE)

    set(SFML_INCLUDE_PATH /SFML/include/)
    set(SFML_LIB_PATH /SFML/lib/)

elseif(UNIX)

    set(SFML_INCLUDE_PATH /usr/local/include/)
    set(SFML_LIB_PATH /usr/local/lib/)

else(APPLE)

    set(SFML_LIB_PATH $ENV{SFML64232}/lib)
    set(SFML_INCLUDE_PATH $ENV{SFML64232}/include)

endif(APPLE)

link_directories(${SFML_LIB_PATH})
include_directories(${SFML_INCLUDE_PATH})
add_definitions(-DSFML_STATIC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -g")

include_directories(VoxGL)

add_executable(VoxGL ${SOURCE_FILES})

if (APPLE)
    # Mac
    target_link_libraries(VoxGL sfml-graphics sfml-window sfml-network sfml-system)
elseif (UNIX)
    # Linux
    target_link_libraries(VoxGL sfml-graphics sfml-window sfml-network sfml-system)
else (APPLE)
    # Windows
    target_link_libraries(VoxGL sfml-graphics-s sfml-window-s sfml-network-s sfml-system-s opengl32 winmm gdi32 freetype jpeg openal32 flac vorbisenc vorbisfile vorbis ogg)
endif (APPLE)
