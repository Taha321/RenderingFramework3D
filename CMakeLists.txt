cmake_minimum_required(VERSION 3.15)
project(RenderingFramework)
set (CMAKE_CXX_STANDARD 14)


set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)

option(ENABLE_TESTS "Enable building tests" OFF)

# Set output directories for libraries and executables
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/output/lib>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/output/lib>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/output/bin>)

# Include Vulkan headers
include_directories(rfw3d ${VULKAN_DIR}/include)
include_directories(rfw3d ${VULKAN_DIR}/x86_64/include)

# Include project headers
include_directories(include)
include_directories(rfw3d src/core)
include_directories(rfw3d src/common)
include_directories(rfw3d src/backend)
include_directories(rfw3d src/api)
include_directories(rfw3d lib/glfw/include)

# link directories
link_directories(rfw3d lib/glfw/x86_64)
link_directories(rfw3d ${VULKAN_DIR}/Lib)
link_directories(rfw3d ${VULKAN_DIR}/x86_64/lib)

# Source files (excluding test_scene.cpp)
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
)

# Add static library
add_library(rfw3d STATIC ${SRC_FILES})

# Link Vulkan library
target_link_libraries(rfw3d glfw3)

# Ensure Vulkan library exists
if(WIN32)
    target_link_libraries(rfw3d vulkan-1)
elseif(UNIX) 
    target_link_libraries(rfw3d vulkan)
endif()

if(ENABLE_TESTS)
    #build base test scene
    add_executable(basetest test/basetest/test_scene.cpp)

    #link test scene with rendering framework library
    target_link_libraries(basetest rfw3d)

    #build test custompipeline test scene
    add_executable(cpltest test/custompipelinetest/test_scene.cpp)

    #link test scene with rendering framework library
    target_link_libraries(cpltest rfw3d)

    #build test custompipeline test scene
    add_executable(dynmeshtest test/dynmeshtest/test_scene.cpp)

    #link test scene with rendering framework library
    target_link_libraries(dynmeshtest rfw3d)
endif()