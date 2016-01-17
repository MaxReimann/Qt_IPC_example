cmake_minimum_required(VERSION 2.8.11)

project(testproject)

set(target processExample)

set(sources
	src/main.cpp
	src/process_example.h
	src/process_example.cpp)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Find the QtWidgets library
find_package(Qt5Widgets)

add_definitions( -DPYSOURCE="${CMAKE_CURRENT_SOURCE_DIR}" )

# Tell CMake to create the helloworld executable
add_executable(${target} ${sources})

# Use the Widgets module from Qt 5.
target_link_libraries(${target} Qt5::Widgets)