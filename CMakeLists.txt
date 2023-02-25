cmake_minimum_required(VERSION 3.7)
project(VulkanPractice CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

add_executable(00_skeleton src/00_skeleton/main.cc)

add_subdirectory(third_party/SDL)
target_include_directories(00_skeleton PRIVATE third_party/SDL/include)
target_link_libraries(00_skeleton SDL2::SDL2-static)

target_include_directories(00_skeleton PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(00_skeleton ${Vulkan_LIBRARIES} "-framework AppKit" "-framework QuartzCore")