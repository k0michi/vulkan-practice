cmake_minimum_required(VERSION 3.7)
project(VulkanPractice CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

add_subdirectory(third_party/SDL)

# 00
add_executable(00_skeleton src/00_skeleton/main.cc)

target_include_directories(00_skeleton PRIVATE third_party/SDL/include)
target_link_libraries(00_skeleton SDL2::SDL2-static)

target_include_directories(00_skeleton PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(00_skeleton ${Vulkan_LIBRARIES} "-framework AppKit" "-framework QuartzCore")


# 01
add_executable(01_instance src/01_instance/main.cc)

target_include_directories(01_instance PRIVATE third_party/SDL/include)
target_link_libraries(01_instance SDL2::SDL2-static)

target_include_directories(01_instance PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(01_instance ${Vulkan_LIBRARIES} "-framework AppKit" "-framework QuartzCore")


# 02
add_executable(02_validation
  src/02_validation/main.cc
  src/02_validation/vulkan_ext.cc
)

target_include_directories(02_validation PRIVATE third_party/SDL/include)
target_link_libraries(02_validation SDL2::SDL2-static)

target_include_directories(02_validation PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(02_validation ${Vulkan_LIBRARIES} "-framework AppKit" "-framework QuartzCore")