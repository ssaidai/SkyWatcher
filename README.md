# SkyWatcher

## Overview
This system is designed to control a drone formation tasked with surveilling a given area effectively. Each drone is managed by a central control center that ensures every point in the surveilled area is checked at least every 5 minutes. The system uses a dynamic path planning algorithm based on a combination of the Traveling Salesman Problem (TSP) and Coverage Path Planning (CPP) to optimize drone paths in real-time.

## Features
- **Dynamic Path Planning**: Utilizes a TSP-CPP algorithm to optimize surveillance routes.
- **Real-time Adaptation**: Adjusts drone paths in response to battery levels, malfunctions, or other operational changes.
- **Drone Simulation Model**: Simplified drone models that simulate positions, battery levels, and operational statuses.
- **Redis Integration**: Uses Redis for real-time data storage and messaging, ensuring high performance and quick response times.

## Prerequisites
- C++ Compiler (e.g., GCC for Linux, Visual Studio for Windows)
- Redis Server (Installation guide available at [Redis Official Site](https://redis.io/download))
- A modern IDE or text editor that supports C++ (optional, but recommended for easier code navigation)

## Installation

### Setting Up the C++ Environment
1. **Linux (Ubuntu)**
   ```bash
   sudo apt update
   sudo apt install g++
    ```
2. **Windows**
    - Download and install Visual Studio from the [official site](https://visualstudio.microsoft.com/).
    - During installation, make sure to select the "Desktop development with C++" workload.

### Installing Redis
Follow the instructions on the Redis website to install Redis on your operating system. For most users, the following will suffice:
```bash
wget http://download.redis.io/releases/redis-6.0.9.tar.gz
tar xzf redis-6.0.9.tar.gz
cd redis-6.0.9
make
```

### Usage
```
Work in progress
```
