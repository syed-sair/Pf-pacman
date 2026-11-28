# Pac-Man (C++ & SFML)

A 2D Pac-Man game built in C++ with SFML, featuring a multi-state menu system, persistent save/load functionality (`savegame.txt`), win/lose pop-up overlays, and distinct ghost behaviors (chasers and wanderers, plus a frightened mode when a power pellet is active).

## Team Members

| Name | Roll Number |
| :--- | :--- |
| Syed Muhammad Sair Hamdani | 26L-0001 |


## Screenshot

<img width="569" height="709" alt="image" src="https://github.com/user-attachments/assets/e149673b-3bce-4a37-9f91-929e7eb99960" />


## Controls

### Main Menu
* **Up / Down Arrow** or **W / S**: Navigate menu options (`CONTINUE` / `NO SAVED GAME`, `NEW GAME`, `EXIT`)
* **Enter**: Confirm selected option

### Gameplay
* **Up / Down / Left / Right Arrow** or **W / S / A / D**: Move Pac-Man
* **P**: Pause / Resume game
* **Escape**: Save progress to `savegame.txt` and return to the Main Menu
* **X**: Close the game window

### Win / Lose Screen
* **Enter**: Return to the Main Menu (clears `savegame.txt` on win or lose)

## How to Run the Game

### Required Files in Working Directory
Place the following files alongside your compiled executable:
* `main.cpp`
* `maze.txt` (21 rows × 19 columns grid layout)
* `arial.ttf` (font file used for menus, HUD, and pop-ups)
* `screenshot.png` (for README display)

### Building and Running (Visual Studio 2026 on Windows)
1. Open the project/solution in **Visual Studio 2026** configured with **SFML 3.0**.
2. Set the build configuration and platform (`x64` / `Debug` or `Release`) matching your SFML libraries.
3. Build the solution (`Ctrl + Shift + B`) and run (`F5` or `Ctrl + F5`).
4. Make sure `maze.txt` and `arial.ttf` are present in the working directory where the `.exe` runs so map loading and text rendering work properly.
