# HideMe2

**HideMe2** is a lightweight Windows-only C++ utility that automatically hides all files with a specific extension (e.g., `.hideme`) in the current directory and enforces the Explorer setting to hide hidden files — even if the user attempts to change it.

> ⚠️ This project is intended for **Windows only** and should be built with a CMake-supported toolchain like Visual Studio.

## 🔧 Features

- Scans the current directory for files with a given extension (e.g., `.hideme`)
- Hides those files using the `FILE_ATTRIBUTE_HIDDEN` attribute
- Monitors and enforces Windows Explorer settings to keep hidden files invisible
- Restores original visibility on program termination (Ctrl+C)


## 📦 Build Instructions

### 🛠 Prerequisites

- Windows 10 or newer
- [CMake](https://cmake.org/download/)
- C++17-compatible compiler (MSVC recommended)

### 🧱 Steps

```bash
git clone https://github.com/your-username/HideMe2.git
cd HideMe2

# Create and enter build directory
cmake -S . -B out
cmake --build out --config Release
```

This will generate the HideMe2.exe binary in the out/Release/ folder.

### 🚀 Usage
```
HideMe2.exe
```

- The program will hide all .hideme files in the current folder (non-recursively)
- Continuously monitor the Hidden registry key and reset it to ensure hidden files remain invisible
- Terminate cleanly on Ctrl+C and restore hidden file visibility

### 🛡 Registry Modifications
This tool modifies the following registry key:
```
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced\Hidden
```

- While running: sets value to 2 (don't show hidden files)
- On exit: restores it to 1 (show hidden files)

✅ Administrator privileges are not required — the key resides in the current user hive.

### 📁 Project Structure
```
HideMe2/
├── HideMe2/                # C++ source code
├── out/                    # CMake build output (ignored via .gitignore)
├── CMakeLists.txt
├── CMakePresets.json
└── README.md
```

### ⚠ Limitations
- Does not hide files recursively through subdirectories.
- Hardcoded to target files with the .hideme extension.
- Only works on Windows; enforced at CMake level.