# ModelViewer

A lightweight **OpenGL Model Viewer** written in modern C++ with **CMake**.  
It’s developed as a **university project** to explore real-time 3D rendering, **GLTF** model loading, and camera controls.

## 🚀 Features
- Scene system with a **Cube Scene** and a **Model Scene**  
- **GLTF model loading** with full mesh and material support  
- Basic camera movement and mouse input  
- Depth testing and shader-based rendering  
- Easy to extend for new 3D scenes or features  

## 🕹️ Controls
- **M** – Switch between cube and GLTF model  
- **Mouse drag** – Rotate camera (Right-click & Scroll-click)  
- **Scroll wheel** – Zoom  

## 📁 Structure
- `assets/` # Models, shaders, textures

- `external/glad/` # OpenGL loader

- `src/` # Application and scene code

- `CMakeLists.txt` # Build configuration


## ⚙️ Build
### Windows
1. Install **CMake** and **Visual Studio 2022**  
2. Run `build_windows_debug.bat` or `build_windows_release.bat`

### Linux
```bash
git clone https://github.com/mstoyanov7/ModelViewer.git
cd ModelViewer
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j