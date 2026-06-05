# acp_mini_proj

A console-based 2D Graphics Editor developed in C using a character-based canvas. It is a fully interactive, keyboard-driven vector graphics editor that runs entirely inside the Windows console, rendering shapes onto an 80x20 character canvas with ANSI colors and support for outline/filled shapes.

## Features

- **Interactive Canvas**: Render graphics in real-time onto an 80x20 text grid.
- **Draw Shapes**: Supports lines, rectangles, circles, and triangles.
- **Outline & Filled Rendering Styles**:
  - Filled rectangles.
  - Filled circles (horizontal scanlines).
  - Filled triangles (scanline rasterization algorithm).
- **ANSI Color Palette**: Set individual shapes to Red, Green, Yellow, Blue, Magenta, Cyan, or White. Menu selection and previews highlight in real-time.
- **Arrow-Key Driven Menus**: Standard console input typing is replaced with responsive keyboard controls (using `<conio.h>`).
- **Visual Canvas Cursor**: Move a cursor (`+`) around the canvas with your arrow keys to place coordinates visually.
- **Dynamic Shape Previews**: Construction lines and shape previews update in real-time as you move the cursor to place subsequent points.
- **Dynamic Rendering**: Utilizes Bresenham's Line Algorithm, Midpoint Circle Algorithm, and custom scanline rasterization for fill modes.

## Navigation Controls

- **Menus**: Use `Up / Down Arrow Keys` to navigate, `Enter` to select.
- **Canvas Selection Mode**:
  - `Arrow Keys`: Move cursor `+`.
  - `Enter`: Confirm point.
  - `Escape`: Cancel operation.

## How to Compile and Run (Windows)

Ensure you have a C compiler such as GCC installed.

1. **Compile**:
   ```bash
   gcc -Wall -o graphics_editor.exe graphics_editor.c -lm
   ```
2. **Run**:
   ```bash
   .\graphics_editor.exe
   ```
