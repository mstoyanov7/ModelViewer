# ModelViewer

A lightweight **OpenGL Model Viewer** written in modern C++ with **CMake**.  
It’s developed as a **university project** to explore real-time 3D rendering, **GLTF** model loading, and camera controls.

## 🚀 Features
- Scene system with default **Cube Scene** and a **Model Scene**  
- **glTF 2.0 model loading** (.gltf and .glb) with mesh, colors, and textures  
- Recursively scans `assets/` for models and lets you switch at runtime (←/→)  
- Orbit/pan/zoom camera with helpful on‑screen shortcuts  
- Depth testing and shader-based rendering  
- Easy to extend for new 3D scenes or features  

## 🕹️ Controls
- General
  - **Right mouse button drag** – Orbit camera  
  - **Middle mouse button drag** – Pan camera  
  - **Scroll wheel** – Zoom  
  - **R** – Reset camera  
  - **F** – Toggle wireframe  
  - **C** – Toggle face culling  
  - **L** – Toggle lighting  
  - **H** – Toggle help overlay  
- Scenes
  - **M** – Toggle Cube/Model scene  
  - In Model scene: **← / →** – Switch between discovered models  

## 📁 Structure
- `assets/` # Models, shaders, textures

- `external/glad/` # OpenGL loader

- `src/` # Application and scene code

- `CMakeLists.txt` # Build configuration


## ⚙️ Build
### Windows
1. Install **CMake** and **Visual Studio 2022**  
2. Run `build_windows_debug.bat` or `build_windows_release.bat`


3. Build manually (If using different version of Visual Studio):

```bash
git clone https://github.com/mstoyanov7/ModelViewer.git
cd ModelViewer
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

### Linux

`sudo apt install libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`

```bash
git clone https://github.com/mstoyanov7/ModelViewer.git
cd ModelViewer
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

## 📦 Loading Your Own Models (glTF)

The viewer automatically discovers models under `assets/` at startup:

- Supported formats: `.gltf` (JSON) and `.glb` (binary)
- Discovery is recursive – any subfolder under `assets/` is scanned
- At runtime, go to Model scene (**M**) and switch models with **← / →**

Add your own model by placing it in `assets/`:

- For `.gltf` with external resources: copy the entire folder (the `.gltf` plus its `.bin` and texture files) preserving relative paths. Example:
  - `assets/MyModel/model.gltf`
  - `assets/MyModel/model.bin`
  - `assets/MyModel/textures/albedo.png`
- For `.glb`: just copy the `.glb` file; it is typically self‑contained.
- Textures referenced by the glTF must be accessible via the relative paths encoded in the file.

Notes:

- CMake copies `assets/` next to the built executable automatically (see the `POST_BUILD` step in `CMakeLists.txt`). If you add or replace files under `assets/`, rebuild or re‑run to refresh the runtime copy.
- The window title shows the currently loaded model path when in Model scene.
- If a model fails to load, check the console for an error and verify all referenced files exist.
