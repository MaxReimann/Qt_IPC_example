cmake_minimum_required(VERSION 2.8.11)

project(testproject)

set(target processExample)

set(sources
	src/process_example.cpp)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)

add_definitions( -DPYSOURCE="${CMAKE_CURRENT_SOURCE_DIR}" )

# Tell CMake to create the helloworld executable
add_executable(${target} ${sources})

# Use the Widgets module from Qt 5.
target_link_libraries(${target} Qt5::Widgets)