cmake_minimum_required(VERSION 3.11)
project(REngine_app)

set(REngine_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/rengine/share/cmake/rengine")

find_package(REngine REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32 OR MINGW)
    set(SDL2_DIR "${CMAKE_CURRENT_LIST_DIR}/sdl2-mingw/cmake")
endif()

find_package(SDL2 REQUIRED)

# Create application
add_executable(rengine_app src/main.cpp src/SceneLoader.cpp)

if(WIN32 OR MINGW)
    target_include_directories(rengine_app PRIVATE "${SDL2_INCLUDE_DIR}")
    set_target_properties(rengine_app PROPERTIES LINK_FLAGS "-mconsole")
endif()

target_link_libraries(rengine_app
    PRIVATE
    REngine::rengine
)
