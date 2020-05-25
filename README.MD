# CSC 476 Lab 1

Alex Gravenor
John Potz
Evan Zhang
David Heyer

Every few seconds a new virus will spawn up to a max of 15 viruses at a time.
The player can score points by running into a virus.

All movement is time based and collisions are based on AABB.

A rolling average of the FPS, the current number of viruses that are on the map currently, and the number of viruses that have been collected are printed to the console every frame.

We decided to use an entity component based design pattern and focused on developing base code that we can expand for our main project.

## Controls

W / A / S / D to move the camera around
Click + Drag in order to pan the camera