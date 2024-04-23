cmake_minimum_required(VERSION 3.28)
project(SkyWatcher)

set(CMAKE_CXX_STANDARD 26)

add_executable(SkyWatcher SkyWatcher.cpp
        Drone.cpp
        Drone.h
        WatchZone.cpp
        WatchZone.h)
