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


# 03
add_executable(03_device
  src/03_device/main.cc
  src/03_device/vulkan_ext.cc
)

target_include_directories(03_device PRIVATE third_party/SDL/include)
target_link_libraries(03_device SDL2::SDL2-static)

target_include_directories(03_device PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(03_device ${Vulkan_LIBRARIES} "-framework AppKit" "-framework QuartzCore")


# 04
add_executable(04_surface
  src/04_surface/main.cc
  src/04_surface/vulkan_ext.cc
)

target_include_directories(04_surface PRIVATE third_party/SDL/include)
target_link_libraries(04_surface SDL2::SDL2-static)

target_include_directories(04_surface PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(04_surface ${Vulkan_LIBRARIES} "-framework AppKit" "-framework QuartzCore")


# 04_hpp
add_executable(04_surface_hpp
  src/04_surface_hpp/main.cc
)

target_include_directories(04_surface_hpp PRIVATE third_party/SDL/include)
target_link_libraries(04_surface_hpp SDL2::SDL2-static)

target_include_directories(04_surface_hpp PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(04_surface_hpp ${Vulkan_LIBRARIES} "-framework AppKit" "-framework QuartzCore")


# 05_hpp
add_executable(05_swapchain_hpp
  src/05_swapchain_hpp/main.cc
)

target_include_directories(05_swapchain_hpp PRIVATE third_party/SDL/include)
target_link_libraries(05_swapchain_hpp SDL2::SDL2-static)

target_include_directories(05_swapchain_hpp PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(05_swapchain_hpp ${Vulkan_LIBRARIES} "-framework AppKit" "-framework QuartzCore")