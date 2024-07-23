cmake_minimum_required(VERSION 3.28)
project(SkyWatcher)

set(CMAKE_CXX_STANDARD 26)

# Include directories
include_directories(${PROJECT_SOURCE_DIR})

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Source files
set(SOURCE_FILES
        Cerebrum.cpp
        Drone.cpp
        SkyWatcher.cpp
        WatchZone.cpp
        # Add other source files here
)

# Executable
add_executable(drone_monitor ${SOURCE_FILES})

# Link SFML libraries
target_link_libraries(drone_monitor sfml-graphics sfml-window sfml-system)
