# Conway's Game of Life, with Compute Shaders

Version: 1.0.0

An implementation of Conway's Game of Life using OpenGL compute shaders.

Requires C++17.

### Usage
1. Supply an image named `input.png` in the executable's directory to use as the starting conditions.
	- Use a white (#FFFFFF) pixel for living cells and black (#000000) for dead cells.
2. Run executable.

### Dependencies
- GLAD
- GLFW
- STB's stb_image.h
