cmake_minimum_required(VERSION 3.28)
project(SkyWatcher)

set(CMAKE_CXX_STANDARD 26)

# Include directories
include_directories(${PROJECT_SOURCE_DIR})

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Source files
set(SOURCE_FILES
        SkyWatcher/Cerebrum.cpp
        Drone/Drone.cpp
        SkyWatcher/SkyWatcher.cpp
        SkyWatcher/WatchZone.cpp
        Redis/Redis.h
        # Add other source files here
)

# Executable
add_executable(drone_monitor ${SOURCE_FILES})

# Link SFML libraries
target_link_libraries(drone_monitor sfml-graphics sfml-window sfml-system)

# <------------ add redis-plus-plus dependency -------------->
# NOTE: this should be *sw* NOT *redis++*
find_path(REDIS_PLUS_PLUS_HEADER sw)
target_include_directories(drone_monitor PUBLIC ${REDIS_PLUS_PLUS_HEADER})

find_library(REDIS_PLUS_PLUS_LIB redis++)
target_link_libraries(drone_monitor ${REDIS_PLUS_PLUS_LIB})
