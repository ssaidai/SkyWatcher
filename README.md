# SkyWatcher

## Overview

SkyWatcher is a drone fleet management system designed to control a formation of drones tasked with surveilling a specified area effectively. The system ensures comprehensive coverage by optimizing drone paths in real-time, guaranteeing that every point in the surveilled area is checked at least every 5 minutes.

The core of SkyWatcher utilizes advanced algorithms combining the Traveling Salesman Problem (TSP) and Coverage Path Planning (CPP) to generate efficient surveillance routes. It adapts dynamically to operational changes such as battery levels, drone malfunctions, or other unforeseen events.

## Features

- **Dynamic Path Planning**: Implements a TSP-CPP algorithm to optimize surveillance routes for maximum efficiency.
- **Real-time Adaptation**: Adjusts drone paths in response to battery levels, malfunctions, or operational changes.
- **Drone Simulation Model**: Simulates drone positions, battery levels, and operational statuses with simplified models.
- **Redis Integration**: Utilizes Redis for real-time data storage and messaging, ensuring high performance and quick response times.
- **Visual Monitoring**: Provides a graphical interface using SFML (Simple and Fast Multimedia Library) for real-time visualization of drone positions and statuses.
- **Scalable Architecture**: Supports a large number of drones and sectors, with efficient resource management.

## Prerequisites

- **C++ Compiler**: GCC (for Linux) or Visual Studio (for Windows)
- **CMake**: For building the project
- **Redis Server**: Installation guide available at [Redis Official Site](https://redis.io/download)
- **SFML Library**: For graphical visualization ([Download SFML](https://www.sfml-dev.org/download.php))
- **Redis++ Library**: C++ client for Redis ([Redis++ GitHub](https://github.com/sewenew/redis-plus-plus))
- **Boost Libraries**: Required for UUID generation and other functionalities
- **Modern C++ IDE/Text Editor**: Optional, but recommended for easier code navigation and development
- **WSL (Windows Subsystem for Linux)**: For Windows users, to run Linux commands and tools (WSL2 recommended)

## Installation

Follow the steps below to install the required dependencies and build the SkyWatcher application in a linux environment.

### 1. Install Redis Server

Follow the installation instructions for your operating system from the [Redis Official Site](https://redis.io/download).

### 2. Install SFML Library
  ```bash
  sudo apt-get install libsfml-dev
  ```

### 3. Install Redis++ Library

Follow the installation guide provided in the [Redis++ GitHub Repository](https://github.com/sewenew/redis-plus-plus#installation).

### 4. Install Boost Libraries

  ```bash
  sudo apt-get install libboost-all-dev
  ```
### 5. Clone the SkyWatcher Repository

```bash
git clone https://github.com/yourusername/SkyWatcher.git
cd SkyWatcher
```

### 6. Build the Project with CMake

```bash
cmake -B Build -S .
cmake --build Build
```

### 7. Run the Application

Ensure that the Redis server is running before starting the application.

- **Start Redis Server**:

  ```bash
  redis-server
  ```

- **Run SkyWatcher**:

  ```bash
  ./SkyWatcher
  ```

## Usage

Upon running the application, the control tower will initialize and start listening for drone connections. Drones can be simulated by running the drone client application, which will connect to the tower and start the surveillance operation.

The graphical interface will display:

- The surveillance grid
- Drone positions and their movements
- Starting points and sectors
- Real-time updates on drone statuses

## Project Structure

- `SkyWatcher/`: Source code files for the SkyWatcher application
- `Drone/`: Source code files for the drone client application
- `Utils/`: Header files for utility functions and classes
- `Build/`: Build directory created by CMake
- `CMakeLists.txt`: Build configuration
- `README.md`: Project documentation

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request for any improvements or bug fixes.

## Acknowledgments

- [Redis](https://redis.io/)
- [SFML](https://www.sfml-dev.org/)
- [Redis++](https://github.com/sewenew/redis-plus-plus)
- [Boost](https://www.boost.org/)
