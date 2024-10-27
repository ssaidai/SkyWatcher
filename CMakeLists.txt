cmake_minimum_required(VERSION 3.14)
project(SkyWatcher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        /usr/local/include
        /opt/homebrew/include
)

# Add source files
add_executable(SkyWatcher
        SkyWatcher/Cerebrum.cpp
        Drone/Drone.cpp
        SkyWatcher/SkyWatcher.cpp
        SkyWatcher/WatchZone.cpp
        Utils/utils.cpp
        Utils/Logger.cpp
        # Add other source files if any
)

add_executable(Drone
        Drone/main.cpp
        Drone/Drone.cpp
        Utils/utils.cpp
        Utils/Logger.cpp
        # Add other source files if any
)

# Find packages
find_package(ortools REQUIRED)
find_package(Protobuf REQUIRED)
find_package(SFML COMPONENTS graphics window system REQUIRED)
find_library(REDIS_PLUS_PLUS_LIBRARY redis++ REQUIRED)
find_library(HIREDIS_LIBRARY hiredis REQUIRED)

# Link libraries
target_link_libraries(SkyWatcher PRIVATE
        ortools::ortools
        ${Protobuf_LIBRARIES}
        sfml-graphics
        sfml-window
        sfml-system
        ${REDIS_PLUS_PLUS_LIBRARY}
        ${HIREDIS_LIBRARY}
        # Add other libraries if necessary
)

target_link_libraries(Drone PRIVATE
        ${REDIS_PLUS_PLUS_LIBRARY}
        ${HIREDIS_LIBRARY}
)

# Set library search paths
link_directories(
        /usr/local/lib
        /opt/homebrew/lib
)