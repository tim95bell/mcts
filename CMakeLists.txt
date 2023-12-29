
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)

project(mcts VERSION 0.0.0 LANGUAGES CXX)

# region raylib 
set(raylib_debug_DIR "./libs/raylib/Debug")
add_library(raylib_debug STATIC IMPORTED GLOBAL)
set_target_properties(raylib_debug PROPERTIES
    IMPORTED_LOCATION "${raylib_debug_DIR}/lib/libraylib.a")
target_include_directories(raylib_debug INTERFACE "${raylib_debug_DIR}/include")
target_link_libraries(raylib_debug INTERFACE "-framework iokit" "-framework cocoa")

set(raylib_release_DIR "./libs/raylib/Release")
add_library(raylib_release STATIC IMPORTED GLOBAL)
set_target_properties(raylib_release PROPERTIES
    IMPORTED_LOCATION "${raylib_release_DIR}/lib/libraylib.a")
target_include_directories(raylib_release INTERFACE "${raylib_release_DIR}/include")
target_link_libraries(raylib_release INTERFACE "-framework iokit" "-framework cocoa")
# endregion raylib

set(source_files src/main.cpp)
set(header_files)
add_executable("${PROJECT_NAME}" ${source_files})
target_link_libraries("${PROJECT_NAME}" PRIVATE debug raylib_debug optimized raylib_release)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${source_files} ${header_files})

if(APPLE)
    set_target_properties("${PROJECT_NAME}" PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] YES)
endif()
