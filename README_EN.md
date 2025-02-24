# Game of Life

## Build Status

[![Build status](https://github.com/AsuRaHan/GameOfLife3D/actions/workflows/release_setup.yml/badge.svg)](https://github.com/AsuRaHan/GameOfLife3D/actions)
[![Build status](https://github.com/AsuRaHan/GameOfLife3D/actions/workflows/release.yml/badge.svg)](https://github.com/AsuRaHan/GameOfLife3D/actions)

## Screenshots

Example of user interface.

![Game of Life 3D](.github/nowar.png)

Example of hidden user interface.

![Game of Life 3D](.github/preview_image.png)

## Overview

**Game of Life** is a visualization of Conway's classic "Life" game using OpenGL. The project allows users to explore cellular automata dynamics in an interactive environment where you can rotate, move the camera, and interact with cells.

This project extends the classic "Life" cellular automaton. Cells exist on a grid where their life or death in each generation depends on the number of neighboring cells.

### Download

To download the latest version of the program, click the following link:

[Download Latest Release](https://github.com/AsuRaHan/GameOfLife3D/releases/latest)

**The project includes**:

- **Visualization**: Display of grid and cells in three-dimensional space.
- **User Interaction**: Simulation control, camera movement, and cell state modification.
- **OpenGL Rendering**: Using modern OpenGL methods for high performance.
- **ImGui User Interface**: Library allows easy customization and modification of interfaces.
- **Pattern Management**: Added ability to load and place patterns from .cells files through PatternManager class, simplifying the addition of new figures to the game.
- **Multi-type Cells**: 
  - Players can manually place three basic cell types: **green** (1), **red** (2), and **blue** (3).
  - When new cells are born, additional types can appear depending on neighbor predominance:
  - **Yellow** (4) — equal number of green and red neighbors.
  - **Orange** (5) — equal number of green and blue neighbors.
  - **Purple** (6) — equal number of red and blue neighbors.
  - **White** (7) — rare type, when all three basic neighbor types are equal (green = red = blue).
- **Random Generation**: Initial cell placement includes all 7 types with equal probability, making each game unique.

## Game Rules:

- **Initial State**: The game begins with an initial configuration where some cells are alive and others are dead.
- **Neighborhood**: Each cell has eight neighbors (cells immediately adjacent, including diagonals).
- **Update Rules**: At each step (or generation), each cell's state is updated according to the following rules:
- **Death by Loneliness**: A living cell dies if it has fewer than two living neighbors.
- **Death by Overcrowding**: A living cell dies if it has more than three living neighbors.
- **Survival**: A living cell stays alive if it has two or three living neighbors.
- **Birth**: A dead cell becomes alive if it has exactly three living neighbors.
- **Generations**: After applying rules to all cells, a new generation is created, and the process repeats.

## Key Features

- **3D Visualization**: The game is displayed in three-dimensional space, with cells represented on a plane.
- **GPU Implementation**: Uses GPU through OpenGL Compute Shaders for simulation acceleration.
- **Interactive Control**:
  - **Camera Navigation**: Use WSADQE keys for camera movement.
  - **Cell State Modification**: Left mouse button toggles cell states (alive/dead).
- **Colored Cell Grid**: Cells can have different colors depending on their state and type.
- **Simulation Control**:
  - **Start/Stop**: Space to control simulation.
  - **Step-by-Step**: Right arrow for next generation.
- **Grid Randomization**: 'R' key for random field filling.
- **Rule Management**: Implemented ability to dynamically change game rules:
  - **Birth**: Number of living neighbors needed for new cell birth.
  - **Survival**: Minimum and maximum number of living neighbors for cell survival.
  - **Overcrowding**: Number of living neighbors causing cell death by overcrowding.
- **User Interface**: Uses ImGui for creating user interface:
  - Simulation control (start, stop, next step).
  - Pattern selection and placement (Glider, Blinker, etc.).
  - Game rule configuration through comboboxes.
- **Grid and Visualization**: 
  - Ability to toggle between showing and hiding grid.
  - Support for toroidal world or bounded field.
- **Save and Load**: Mechanisms for saving and loading game state.
- **Camera**: Implemented navigation in 3D space with camera movement capability.

## Controls

- **Space**: Start/stop simulation.
- **ENTER**: Step forward.
- **WSADQE**: Move camera forward, backward, left, right, up, down.
- **R**: Random field filling.
- **T**: Reset camera to view entire game world.
- **Y**: Change world type (grid with bounded edges or toroidal edges).
- **I**: Copy selected area as pattern for pasting.
- **C**: Clear field, kill all cells.
- **G**: Hide or show grid.
- **Mouse Wheel**: Scaling (zoom). Hold left Control (Ctrl) to increase scaling speed.
- **Right Click and Drag**: Move camera.
- **LMB**: Change cell state under cursor.
- **LMB + SHIFT and Drag**: Select field, then use **INSERT** to insert live cells in selected area, **DELETE** to remove live cells, **ESCAPE** to clear selection.
- **MMB (middle mouse button click)**: Add figure from preset patterns or selected files.
- **Keys 1 to 6**: Select various preset patterns for placement:
  - **1**: Glider
  - **2**: Blinker
  - **3**: Toad
  - **4**: Beacon
  - **5**: Pentadecathlon
  - **6**: Gosper Glider Gun
- **Keys 7 to 9**: Select color for cell placement:		
  - **7**: Green (type 1).
  - **8**: Red (type 2).
  - **9**: Blue (type 3).

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

5. **Build via GitHub**
	- Make changes to project, prepare release.
	- Create `git tag`.
```bash
git tag -a v1.0.0 -m "Version 1.0.0"
git push origin v1.0.0
```

6. **Launch**:
   - Run the built application.
   - You can also pass command line parameters to specify game field size *-gridWidth 200* *-gridHeight 150* default is 300x400. You can also run in fullscreen mode by passing *-fullscreen* parameter set fullscreen resolution *-screenResolution 800x600* `life.exe -gridWidth 200 -gridHeight 150 -screenResolution 800x600 -fullscreen` see example in [run_life.bat](run_life.bat)
	
## Features

- **Simulation Control**:
  - Start/Stop simulation
  - Step-by-step generation progression

- **3D Camera Control**:
  - Panning, scaling

- **Grid Control**:
  - Initialize grid with random cells
  - Change individual cell states with mouse

## Technologies Used

- **C++** for core logic and game mechanics
- **OpenGL** for visualization
- **Windows API** for window management and event handling
- **ImGuiI** for user interface

## Project Structure

- **./game**: Core game logic:
  - `Grid.h`, `Grid.cpp` - Grid management
  - `GameController.h`, `GameController.cpp` - Game state management
  - `GPUAutomaton.h`, `GPUAutomaton.cpp` - Cellular automaton using GPU for calculation
  - `GameStateManager.h`, `GameStateManager.cpp` - World loading and saving management
  - `PatternManager.h`, `PatternManager.cpp` - Management of ready-made pattern loading

- **./rendering**: Rendering-related code:
  - `Camera.h`, `Camera.cpp` - Camera for 3D view.
  - `CameraController.h`, `CameraController.cpp` - Camera control.
  - `Renderer.h`, `Renderer.cpp` - OpenGL rendering logic.
  - `UIRenderer.h`, `UIRenderer.cpp` - GUI window and menu rendering and operation.
  - `SelectionRenderer.h`, `SelectionRenderer.cpp` - Mouse field selection line rendering.
  - `GridRenderer.h`, `GridRenderer.cpp` - Navigation coordinate grid rendering.
  - `TextureFieldRenderer.h`, `TextureFieldRenderer.cpp` - Game field rendering at far distances from the field, improves performance without affecting visual display.
  - `CellsRenderer.h`, `CellsRenderer.cpp` - Game field rendering at close distances from the game field, improves visual display affects performance.
  - `IRendererProvider.h` - Conductor ensuring data exchange between different classes. This is a simple interface, provider pattern.

- **./mathematics**: Mathematical utilities:
  - `Matrix4x4.h`, `Matrix4x4.cpp` - Matrix operations (currently not used)
  - `Vector3d.h`, `Vector3d.cpp` - Vector operations

- **./windowing**: Window and input management:
  - `MainWindow.h`, `MainWindow.cpp` - Window creation and basic event handling
  - `WindowController.h`, `WindowController.cpp` - Keyboard and mouse event handling
  - `GridPicker.h`, `GridPicker.cpp` - Grid (game field) click handling

- **./system**: System, OpenGL, and user input setup:
  - `OpenGLInitializer.h`, `OpenGLInitializer.cpp` - OpenGL context initialization
  - `GLFunctions.h`, `GLFunctions.cpp` - OpenGL function loading (trimmed GLAD)
  - `ShaderManager.h`, `ShaderManager.cpp` - Creating compiling checking shaders (GLSL) OpenGL
  - `SettingsManager.h`, `SettingsManager.cpp` - User settings saving and loading.
  - `InputHandler.h`, `InputHandler.cpp` - User input handler (keyboard mouse).
  - `InputEvent.h` - User input structure itself.

- **main.cpp**: Application entry point

## Known Issues

- Performance may degrade on very large grids due to 3D visualization complexity and live/dead cell computation.
- Setting a very large world requires quite a long time for grid rebuilding, causing the game to freeze temporarily (depends on world size)
- Very large worlds may take long to load or save

### Future Improvements

- [x] Implemented support for different rule sets for Game of Life.
- [x] Added functionality for dynamic world size changes.
- [x] Optimized rendering for improved performance on large worlds and complex patterns.
- [x] Implemented loading of preset patterns from .cells files located in patterns folder.
- [x] Improved world state loading and saving time.
- [x] Accelerated world rebuilding when changing its dimensions.
- [x] Optimized random filling and complete world clearing functions.

## From the Author

Feel free to report any issues or suggest improvements!
- If you want to contribute:
  - Create a fork of the repository
  - Make changes in your copy
  - Send a pull request with description of changes
- You can contact me on [telegram](https://t.me/AsuRaHan)
- Collecting donations [here](https://boosty.to/asurahan/single-payment/donation/677381/target?share=target_link)

## What I Read, Who Inspired Me, and Where I Found Information

This is my first project using WinAPI and OpenGL.
- Learned how to work with OpenGL [here](https://github.com/msqrt)
- Read about Game of Life and cellular automaton [here](https://conwaylife.com/)
- Special thanks to [my city's IT community](https://t.me/dc78422)

## License

This project is under the [MIT license](LICENSE).
