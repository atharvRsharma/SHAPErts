# Cross-Realm Entity Bridge

This project is a 10-day technical challenge demonstrating an entity (a glowing orb) that exists and persists across two separate applications: a low-level OpenGL C++ simulation and a high-level Unreal Engine 4 game.

The "bridge" between them is a shared entity_state.json file. The OpenGL app acts as the "physics authority," saving its state, while the UE4 app loads that state to recreate the entity with identical mechanics.

How to Run

Place the OGL_Build (or ogl_port.exe and its assets) and the UE4_Build (e.g., WindowsNoEditor) folders in the same directory.

Run the OpenGL app first (ogl_port.exe). Interact with the orb and press 'S' to save (or just close it). This will create/update entity_state.json.

Run the UE4 app (cross_realm_ue4.exe). The orb will spawn in the state you saved.

You can use the 'L' key in UE4 to "hot-reload" the JSON file after saving new changes in the OGL app.

DEMO VIDEO(slightly confusing as there are no screen messages to denote what the result of a key input is):

https://drive.google.com/file/d/1-NRtRsV2JRON9CFN-2D19qN12OBG-sV9/view?usp=sharing


Controls

OpenGL (ogl_port.exe)

WASD: Move Camera

Space / L-Shift: Move Camera Up / Down

Mouse: Look Camera

E (Hold): Interact (Gain Energy)

G: Toggle Gravity

R: Reset Orb Position

S: Save Current State to JSON

Esc: Exit Application

Unreal Engine 4 (cross_realm_ue4.exe)

WASD: Move Camera

Mouse: Look Camera

F (Hold): Interact (Gain Energy)

G: Toggle Gravity

R: Reset Orb Position

L: Load/Reload State from JSON

Shift + F1: Show / Hide Mouse Cursor
