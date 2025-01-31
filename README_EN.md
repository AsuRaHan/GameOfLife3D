# 3D Game of Life

**3D Game of Life** is an implementation of Conway's Game of Life in three dimensions, utilizing OpenGL for visualization.

## Overview

This project extends the classic cellular automaton known as the Game of Life into 3D space. Cells live on a grid where their life or death in each generation depends on the number of neighboring cells. This version includes:

- **3D Visualization**: Rendering the grid and cells in a 3D environment.
- **User Interaction**: Control over simulation, camera movement, and cell state.
- **OpenGL Rendering**: Using modern OpenGL techniques for performance.

## Features

- **Simulation Control**:
  - Start/Stop simulation
  - Step through generations manually
  - Navigate back through previous generations

- **3D Camera Control**:
  - Pan, zoom, and rotate the view of the grid

- **Grid Management**:
  - Initialize grid with random cells
  - Change individual cell states with mouse clicks

- **Debug Overlay**: 
  - Visual cues for simulation state (running/stopped)

## Technologies Used

- **C++** for core logic and game mechanics
- **OpenGL** for rendering
- **Windows API** for window management and event handling

## Project Structure

- **./game**: Core game logic
  - `Cell.h`, `Cell.cpp` - Cell class implementation
  - `Grid.h`, `Grid.cpp` - Grid management
  - `GameOfLife.h`, `GameOfLife.cpp` - Game rules and logic
  - `GameController.h`, `GameController.cpp` - Game state management

- **./rendering**: All rendering-related code
  - `Camera.h`, `Camera.cpp` - Camera management for 3D view
  - `Renderer.h`, `Renderer.cpp` - OpenGL rendering logic
  - `DebugOverlay.h`, `DebugOverlay.cpp` - Simple on-screen controls

- **./mathematics**: Math utilities
  - `Matrix4x4.h`, `Matrix4x4.cpp` - Matrix operations
  - `Vector3d.h`, `Vector3d.cpp` - Vector operations

- **./windowing**: Window and input management
  - `MainWindow.h`, `MainWindow.cpp` - Window creation and basic events
  - `WindowController.h`, `WindowController.cpp` - Event handling

- **./system**: System and OpenGL setup
  - `OpenGLInitializer.h`, `OpenGLInitializer.cpp` - OpenGL context setup
  - `GLFunctions.h`, `GLFunctions.cpp` - OpenGL function loading

- **main.cpp**: Entry point of the application

## Setup

To set up and run the project:

1. **Prerequisites**:
   - Windows OS
   - Visual Studio with C++ development tools
   - OpenGL libraries and headers

2. **Building**:
   - Open the project in Visual Studio.
   - Configure for Windows desktop development with OpenGL support.
   - Build the solution.

3. **Running**:
   - Execute the built application.

## Usage

- Use **spacebar** to start/stop the simulation.
- **Right arrow** for stepping forward, **Left arrow** for stepping back.
- **Mouse wheel** for zooming in/out.
- **Right click and drag** to move the camera.

## Known Issues

- Performance might degrade with very large grids due to the complexity of 3D rendering.

## Future Enhancements

- Implement different rule sets for the Game of Life.
- Add more interactive controls for grid size, cell size, etc.
- Optimize rendering for better performance on larger grids or more complex patterns.

## License

This project is under [MIT License](LICENSE).

---

Feel free to report any issues or suggest improvements!