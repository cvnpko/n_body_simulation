# n_body_simulation
```n_body_simulation``` is a project for the Basics of Mechanics course at the Faculty of Mathematics, University of Belgrade for the school year of 2024/2025.
# Setup
## Linux
To build the project, run:
```./build.sh```  
To start the program, run:
```./build.sh/NBodySimulation```
# Project Overview

The **n-body problem** refers to the challenge of predicting the individual motions of a system of celestial bodies interacting with one another under the influence of gravitational forces.

This simulation offers **four modes** to explore different scenarios:

1. **Trail Mode** – Displays a trail behind the moving bodies to track their trajectories.
2. **Walls Mode** – Treats the edges of the simulation screen as walls, causing bodies to collide with and bounce off them.
3. **Collisions Mode** – Enables body-to-body collisions with an adjustable coefficient of restitution to simulate realistic impacts.

---

## Simulation Modes

### 1. **Three Bodies**
This mode involves three bodies, with adjustable parameters for position, velocity, and mass.

**Available Features:**  
- Trail  
- Walls  
- Collisions  

### 2. **Fixed Two Bodies**
In this mode, two of the three bodies are fixed in place. The user can adjust the position, speed, and mass of the first body, while the second and third bodies allow modifications to mass and position only.

**Available Features:**  
- Trail  
- Walls  
- Collisions  

### 3. **Small n Bodies**
This mode supports up to 10 bodies, where the user can adjust the position, speed, and mass for each body individually.

**Available Features:**  
- Trail  
- Walls  
- Collisions  

### 4. **Large n Bodies**
In this mode, the simulation involves thousands of bodies. No adjustments can be made to the properties of individual bodies.

**Available Features:**  
- Walls  

### 5. **Three Bodies 3D**
This mode is similar to the "Three Bodies" simulation but with an additional spatial dimension for a more complex simulation environment.

**Available Features:**  
- Collisions  