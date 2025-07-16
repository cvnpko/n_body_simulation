# n_body_simulation
```n_body_simulatio``` is project for Mechanics course at the Faculty of Mathematics, University of Belgrade for the school year of 2024/2025.
# Setup
## Linux
### Debian based (Ubuntu, Debian...)
To setup the necessary libraries, run:
```./build.sh```
To start the program, run:
```./buid.sh/NBodySimulation```
# What is this project even about?
The n-body problem is the problem of predicting the individual motions of a group of celestial objects interacting with each other gravitationally.
There are 3 possible options for simulation.
1. Trail - adds trail behind moving body.
2. Walls - edges of the screen are turned into walls, boides collide and bounce off of them.
3. Collisions - enables body to body collision with adjustable value of coefficient of restitution.
## Three bodies
Three-body problem is the most studied n-body problem. 
We can ajust x, y coordinates of bodies, their speed and mass.
Available options: Trail, Walls, Collisions.
## Fixed two bodies
Two bodies out of three can't move.
We can ajust position, speed and mass of first body. For second and third we can only change mass and position.
Available options: Trail, Walls, Collisions.
## Small n bodies
Up to 10 bodes.
For each body we can change position, speed, mass.
Available options: Trail, Walls, Collisions.
## Big n bodies
Number of bodies is in thousands.
We can't change anything about bodies.
Available options: Walls.
## Three bodies 3D
Same as Three bodies just with extra dimension.
Available options: Collisions.
