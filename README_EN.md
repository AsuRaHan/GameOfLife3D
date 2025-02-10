# Game of Life 3D

![Game of Life 3D](.github/preview_image.png)

![Game of Life 3D](.github/preview_image2.png)

## Overview

**Game of Life 3D** is a visualization of Conway's classic Game of Life in 3D using OpenGL. The project allows users to explore cellular automata dynamics in an interactive environment where you can rotate, move the camera, and interact with cells.

This project extends the classic Game of Life cellular automaton into 3D. Cells exist on a grid where their life or death in each generation depends on the number of neighboring cells.

### Download

To download the latest version of the program, click the following link:

[Download Latest Release](https://github.com/AsuRaHan/GameOfLife3D/releases/latest)

**The project includes**:

- **3D Visualization**: Display of grid and cells in three-dimensional space.
- **User Interaction**: Control of simulation, camera movement, and cell state modification.
- **OpenGL Rendering**: Use of modern OpenGL methods for high performance.
- **ImGui User Interface**: Library allows easy customization and modification of interfaces.

**Game rules are as follows**:

- **Initial State**: The game begins with an initial configuration of cells, where some cells are alive and others are dead.
- **Neighborhood**: Each cell has eight neighbors (cells that are directly adjacent, including diagonals).
- **Update Rules**: At each step (or generation), each cell's state is updated according to the following rules:
- **Death by loneliness**: A living cell dies if it has fewer than two living neighbors.
- **Death by overcrowding**: A living cell dies if it has more than three living neighbors.
- **Survival**: A living cell stays alive if it has two or three living neighbors.
- **Birth**: A dead cell becomes alive if it has exactly three living neighbors.
- **Generations**: After applying rules to all cells, a new generation is created, and the process repeats.

## Key Features

- **3D visualization**: The game is displayed in three-dimensional space, where cells are represented on a plane.
- **GPU Implementation**: GPU is used through OpenGL Compute Shaders for simulation acceleration.
- **Interactive Controls**:
  - **Camera Navigation**: Use WSAD keys for camera movement.
  - **Cell State Modification**: Left mouse button toggles cell states (alive/dead).
- **Colored Cell Grid**: Cells can have different colors depending on their state and type.
- **Simulation Control**:
  - **Start/Stop**: Space for simulation control.
  - **Step-by-Step**: Right arrow for next generation.
- **Grid Randomization**: 'R' key for random field population.
- **Rules Management**: Ability to dynamically change game rules:
  - **Birth**: Number of live neighbors needed for a new cell to be born.
  - **Survival**: Minimum and maximum number of live neighbors for cell survival.
  - **Overcrowding**: Number of live neighbors at which a cell dies from overcrowding.
- **User Interface**: ImGui is used for creating the user interface:
  - Simulation control (start, stop, next step).
  - Pattern selection and placement (Glider, Blinker, etc.).
  - Game rules configuration through comboboxes.
- **Grid and Visualization**: 
  - Ability to toggle grid display.
  - Support for toroidal world or bounded field.
- **Save and Load**: Mechanisms for saving and loading game state.
- **Camera**: 3D space navigation with camera movement capability.

## Controls

- **Esc**: Exit game, close application.
- **Space**: Start/stop simulation.
- **Numpad '+'**: Increase simulation speed.
- **Numpad '-'**: Decrease simulation speed.
- **Right Arrow**: Step forward.
- **Mouse Wheel**: Scaling (zoom). Hold left Control (Ctrl) to increase scaling speed.
- **Right Click and Drag**: Camera movement.
- **WSADQE**: Camera movement forward, backward, left, right, up, down.
- **T**: Reset camera to view entire game world.
- **R**: Random field population.
- **C**: Clear field, kill all cells.
- **I**: Fill field with random number of gliders.
- **G**: Hide or show grid.
- **Y**: Change world type (bounded grid or toroidal edges).
- **LMB**: Change cell state under cursor.
- **MMB (middle mouse button click)**: Add figure from preset patterns.
- **Keys 1 to 6**: Select various preset patterns for placement:
  - **1**: Glider
  - **2**: Blinker
  - **3**: Toad
  - **4**: Beacon
  - **5**: Pentadecathlon
  - **6**: Gosper Glider Gun

## Installation

To set up and run the project:

1. **Prerequisites**:
   - Windows operating system
   - Visual Studio with C++ development tools
   - OpenGL libraries and header files

2. **Installation**:

```bash
git clone --recursive https://github.com/AsuRaHan/GameOfLife3D.git
cd GameOfLife3D
git submodule update --init --recursive
```

3. **Repository and Submodules Update:**
	- Navigate to your project directory

```bash
git pull origin main
git submodule update --recursive --remote
```

4. **Build**:
	- Open project in Visual Studio 2022 or higher.
	- Configure for Windows development with OpenGL support.
	- Build solution.

5. **GitHub Build**
	- Make changes to project, prepare release.
	- Create `git tag`.
```bash
git tag -a v1.0.0 -m "Version 1.0.0"
git push origin v1.0.0
```

6. **Launch**:
   - Run the built application.
   - You can also pass command line parameters to specify game field size, default is 300x400. You can also run in fullscreen mode by passing -fullscreen parameter `life.exe -gridWidth 200 -gridHeight 150 -fullscreen` see example in [run_life.bat](run_life.bat)

## Technologies Used

- **C++** for core logic and game mechanics
- **OpenGL** for visualization
- **Windows API** for window management and event handling
- **ImGui** for user interface

## Project Structure

- **./game**: Core game logic:
  - `Cell.h`, `Cell.cpp` - Cell class implementation
  - `Grid.h`, `Grid.cpp` - Grid management
  - `GameOfLife.h`, `GameOfLife.cpp` - Game rules and logic
  - `GameController.h`, `GameController.cpp` - Game state management
  - `GPUAutomaton.h`, `GPUAutomaton.cpp` - GPU-based cellular automaton
  - `GameStateManager.h`, `GameStateManager.cpp` - World save/load management

- **./rendering**: Rendering-related code:
  - `Camera.h`, `Camera.cpp` - 3D view camera
  - `CameraController.h`, `CameraController.cpp` - Camera control
  - `Renderer.h`, `Renderer.cpp` - OpenGL rendering logic
  - `UIRenderer.h`, `UIRenderer.cpp` - GUI window and menu rendering
  - `CellInstance.h` - Structure for storing live/dead cell instances for optimal OpenGL rendering
  - `IRendererProvider.h` - Provider pattern interface for data exchange between classes

- **./mathematics**: Mathematical utilities:
  - `Matrix4x4.h`, `Matrix4x4.cpp` - Matrix operations (currently unused)
  - `Vector3d.h`, `Vector3d.cpp` - Vector operations

- **./windowing**: Window and input management:
  - `MainWindow.h`, `MainWindow.cpp` - Window creation and basic event handling
  - `WindowController.h`, `WindowController.cpp` - Event handling
  - `GridPicker.h`, `GridPicker.cpp` - Grid click handling

- **./system**: System, OpenGL, and user input setup:
  - `OpenGLInitializer.h`, `OpenGLInitializer.cpp` - OpenGL context initialization
  - `GLFunctions.h`, `GLFunctions.cpp` - OpenGL function loading
  - `ShaderManager.h`, `ShaderManager.cpp` - OpenGL shader (GLSL) management
  - `SettingsManager.h`, `SettingsManager.cpp` - User settings management
  - `InputHandler.h`, `InputHandler.cpp` - User input handler
  - `InputEvent.h` - Input event structure

- **main.cpp**: Application entry point

## Known Issues

Performance may degrade on very large grids due to 3D visualization complexity and cell state calculations.

### Future Improvements

- [x] Implementation of different rule sets for "Life"
- [x] Grid size management
- [x] Rendering optimization for better performance on large grids or complex patterns

## Changes and Optimizations

- **Performance Optimization**:
  The following improvements have been made:

- **Iteration Reduction**: 
  Previously, the main game loop had three nested loops. Now operations are reduced to two main cycles:
  - Loading data to GPU and computing next generation
  - Direct cell color updates after GPU computation

- **Update and Render Synchronization**:
  Added SetCellColor method in GameOfLife class for setting cell color immediately after computing its new state, improving synchronization between game logic and visualization.

## From the Author

Feel free to report any issues or suggest improvements!
- If you want to contribute:
  - Fork the repository
  - Make changes in your copy
  - Submit a pull request with changes description
- Contact me on [Telegram](https://t.me/AsuRaHan)
- Donations accepted [here](https://boosty.to/asurahan/single-payment/donation/677381/target?share=target_link)

## References and Inspiration

This is my first WinAPI and OpenGL project.
- Learned OpenGL from [here](https://github.com/msqrt)
- Read about Game of Life and cellular automata [here](https://conwaylife.com/)
- Special thanks to [my city's IT community](https://t.me/dc78422)

## License

This project is under the [MIT License](LICENSE).