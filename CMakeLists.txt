cmake_minimum_required(VERSION 3.21)
project(index)

include(FetchContent)
  set(CMAKE_BUILD_TYPE Debug)
  #if(NOT CMAKE_BUILD_TYPE)
  #endif()

find_package(PkgConfig REQUIRED)
set(CMAKE_C_STANDARD 99)

#add_compile_definitions(RAYLIB_CUSTOM_CONFIG)
# Raylib requires OpenGL and other system libs
set(PLATFORM Desktop CACHE STRING "Target platform")
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)
#include_directories(include)         # Your config.h
add_executable(${PROJECT_NAME})
add_subdirectory(src)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(resources)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
# Let CMake inherit the include dirs from raylib
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${raylib_SOURCE_DIR}/src
    includes
)
add_compile_options(-pg -g -O0)
if("${PLATFORM}" MATCHES "Web")
    add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/../resources
    )
    #DEPENDS ${PROJECT_NAME}
else()
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    )
  endif()
# Web Configurations
#if ("${PLATFORM}" MATCHES "Web")
#  target_link_libraries(index PRIVATE raylib)
#  set_target_properties(index PROPERTIES
#    LINK_FLAGS "--preload-file resources -O0 -gsource-map -sASSERTIONS=2 -sALLOW_MEMORY_GROWTH=1 -sUSE_GLFW=3 -sASYNCIFY -sUSE_WEBGL2=1 -sFETCH -sFORCE_FILESYSTEM=1 -sINITIAL_MEMORY=256MB -sTOTAL_STACK=2MB -sWASM=1 -s SINGLE_FILE=0 -o index.html -sEXPORTED_RUNTIME_METHODS=['HEAP8','HEAPU8','HEAP16','HEAPU16','HEAP32','HEAPU32','HEAPF32','HEAPF64','ccall','cwrap']"  )
#  add_custom_command(
#    TARGET ${PROJECT_NAME} PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/../resources
#  )
  #DEPENDS ${PROJECT_NAME}

  #endif()
  target_link_options(index PRIVATE -pg)
if(APPLE)
  target_link_libraries(${PROJECT_NAME} m dl pthread GL X11 Xrandr Xinerama Xcursor Xi)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(OpenGL_LIBRARY OpenGL REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(CoreVideo_LIBRARY CoreVideo REQUIRED)

    target_link_libraries(game ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IOKIT_LIBRARY} ${CoreVideo_LIBRARY})
endif()

