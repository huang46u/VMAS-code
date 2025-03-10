# Dynamic Skeletonization via Variational Medial Axis Sampling
![VMAS Header](doc/img/teaser.png)
This repository provides the official implementation of the paper "Dynamic Skeletonization via Variational Medial Axis Sampling" (SIGGRAPH Asia 2024). Originally developed within the [CGOGN_3](https://github.com/cgogn/CGoGN_3) library, it has been streamlined to include only the essential files required for functionality.


[Project Page](https://huang46u.github.io/VMAS/) | [Code](https://github.com/huang46u/VMAS)

## Dependencies
- Eigen
- Glfw3

## Usage
1. Clone the repository:
   ```sh
   git clone git@github.com:huang46u/VMAS-code.git
   cd VMAS-code
   ```

2. Build the project with Cmake:
   ```sh
   mkdir build
   cd build
   cmake ..
   make
   ```

## User Guide

Our algorithm extracts discretized medial axis (containing both curve skeletons and surface skeletons) from a mesh or point cloud through an iterative sphere optimization process.
1. Suppose you are under the `build` directory, launch the program with:
   ```sh  
   ./stage/bin/Release/vmas.exe ../data/xxx.off
   ```
2. Set the desired number of spheres in the `Nb Spheres` field
3. Click `Start spheres update` to begin the optimization process which automatically refines the medial spheres
4. Click `Stop spheres update` when satisfied with the result or when the result has converged

### Saving Results

1. Click on the top menu item `Mesh Provider(IncidenceGraph)`
2. Select `Save mesh` from the dropdown menu
3. In the dialog box:
   - Choose the skeleton mesh from the dropdown list
   - Select `.ply` format (currently the only supported format)
   - Enter your desired filename
   - Select `position` as the attribute to save
4. Click `Save` and choose a destination folder

### Interactive Refinement

The interface allows direct manipulation of the skeleton:
- Use `S` key to split the sphere
- Use `D` key to delete spheres

### Experiments Section

The **Experiments** section contains various testing parameters we used during our research and algorithm development - curious users can play with these settings to observe how different parameters affect the algorithm's behavior. Have fun! 

## Demo
![example](doc/img/example.gif)

## Citation
```sh
@inproceedings{10.1145/3680528.3687678,
author = {Huang, Qijia and Kraemer, Pierre and Thery, Sylvain and Bechmann, Dominique},
title = {Dynamic Skeletonization via Variational Medial Axis Sampling},
year = {2024},
isbn = {9798400711312},
publisher = {Association for Computing Machinery},
address = {New York, NY, USA},
url = {https://doi.org/10.1145/3680528.3687678},
doi = {10.1145/3680528.3687678},
booktitle = {SIGGRAPH Asia 2024 Conference Papers},
articleno = {66},
numpages = {11},
keywords = {Medial Axis, Skeleton, Variational approach},
location = {Tokyo, Japan},
series = {SA '24}
}
``` 
