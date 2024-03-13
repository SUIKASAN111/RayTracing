cmake_minimum_required(VERSION 3.15)
project(RayTracing)
set(CMAKE_CXX_STANDARD 17)
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/vendor)
link_directories(${PROJECT_SOURCE_DIR}/vendor/assimp/lib/Debug ${PROJECT_SOURCE_DIR}/vendor/assimp/lib/Release)
add_executable(RayTracing main.cpp)
target_link_libraries(RayTracing
    debug ${PROJECT_SOURCE_DIR}/vendor/assimp/lib/Debug/assimp-vc142-mtd.lib
    optimized ${PROJECT_SOURCE_DIR}/vendor/assimp/lib/Release/assimp-vc142-mt.lib)