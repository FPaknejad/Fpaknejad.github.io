# Universal C++20 Project Template (LLVM + Ninja + Modules Optional)

This template works for:

- Normal C++20 projects
- Projects that later add C++20 modules
- Pure module-based projects
- Mixed builds with libraries + modules

It uses:
- LLVM clang++
- Ninja build system
- clangd (for real IntelliSense)
- CMake 3.28+

---

## 📦 Folder Overview
src/
│── main/        → main application code
│── libs/        → regular C++ implementations
└── modules/     → C++20 module files (.cppm)
include/         → headers (if needed)
.vscode/         → editor configuration
CMakeLists.txt   → universal build file

You can delete any folders you don’t use.

---

## 🚀 How to Build

### Build & Run 

**Press:**  Cmd + Shift + B
This will:

1. Configure CMake with Ninja  
2. Build your project  
3. Run `./build/app`  

_or_ run manually:

```bash
cmake -G Ninja -S . -B build
cmake --build build
./build/app

### Terminal Commands
rm -rf build
cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build
cmake --build build
./build/app 
```