# FeatherweightGPS Serial Reader
- A small C++ project to read the Featherwieght ground station serial data and plot it in real-time using the IMgui graphics library.
- **To build on Windows:**
  - Install MSYS2
  - Open MSYS2
    - Run: pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-gcc mingw-w64-x86_64-glfw mingw-w64-x86_64-glew curl libcurl
  - In your compiler, set the toolchain to use mingw-w64 system install.
  - Compile.
- **To build on Linux:**
  - Install system libraries for GLEW, GLFW, and curl using your package manager.