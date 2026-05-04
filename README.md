# Face Detection Pipeline — Cross-Platform Build Guide

A modular face detection system with two pipeline modes: an **In-Memory** pipeline (fastest, zero disk I/O) and a **Decoupled** pipeline (frame extraction → detection). Both are built with C++17, OpenCV, and CMake, and run on **Linux**, **macOS**, and **Windows**.

---

## Project Structure

```
img_classification/
├── in_memory_detector/       # Reads video directly, detects faces in memory
│   ├── src/
│   ├── tests/
│   └── CMakeLists.txt
├── decoupled_detector/       # Reads pre-extracted JPEG frames from disk
│   ├── src/
│   ├── tests/
│   └── CMakeLists.txt
├── extract_frames.sh         # GStreamer frame extraction (Linux/macOS)
├── benchmark_script.sh       # Bash benchmark (Linux/macOS)
├── benchmark_script.py       # Cross-platform Python benchmark
└── README.md
```

---

## Prerequisites

### Linux (Ubuntu / Debian / Fedora)

```bash
# Ubuntu / Debian
sudo apt install cmake g++ libopencv-dev libgtest-dev gstreamer1.0-tools

# Fedora
sudo dnf install cmake gcc-c++ opencv-devel gtest-devel gstreamer1-plugins-base-tools
```

### macOS (Homebrew)

```bash
# Install Xcode Command Line Tools (if not already installed)
xcode-select --install

# Install dependencies via Homebrew
brew install cmake opencv googletest ffmpeg

# Optional: GStreamer for the decoupled pipeline shell script
brew install gstreamer gst-plugins-base gst-plugins-good
```

> **Note:** On Apple Silicon (M1/M2/M3/M4), Homebrew installs to `/opt/homebrew/`. On Intel Macs, it installs to `/usr/local/`. The code auto-detects both paths for the Haar cascade XML.

### Windows

**Option A — vcpkg (Recommended)**
```powershell
# Install vcpkg if you haven't
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg\vcpkg install opencv4:x64-windows gtest:x64-windows

# When running CMake, pass the toolchain file:
# cmake .. -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
```

**Option B — Pre-built OpenCV**
1. Download OpenCV from [opencv.org/releases](https://opencv.org/releases/)
2. Extract to `C:\opencv`
3. Add `C:\opencv\build\x64\vc16\bin` to your system `PATH`
4. Set environment variable `OpenCV_DIR=C:\opencv\build`

**Compiler:** Install [Visual Studio 2019+](https://visualstudio.microsoft.com/) with the "Desktop development with C++" workload, or use [MinGW-w64](https://www.mingw-w64.org/).

**Additional tools (for benchmarking):**
- Python 3.x — [python.org](https://www.python.org/downloads/)
- ffmpeg — `winget install ffmpeg` or download from [ffmpeg.org](https://ffmpeg.org/)

---

## Building the Detectors

Since the project contains two separate modules, you must build them individually.

### Linux / macOS

```bash
# Build the In-Memory Detector
cd in_memory_detector
mkdir build && cd build
cmake ..
make
cd ../..

# Build the Decoupled Detector
cd decoupled_detector
mkdir build && cd build
cmake ..
make
cd ../..
```

### Windows (Visual Studio / MSVC)

```powershell
# Build the In-Memory Detector
cd in_memory_detector
mkdir build; cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
cd ..\..

# Build the Decoupled Detector
cd decoupled_detector
mkdir build; cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
cd ..\..
```

> **Tip:** If using pre-built OpenCV instead of vcpkg, omit the `-DCMAKE_TOOLCHAIN_FILE` flag and ensure `OpenCV_DIR` is set in your environment.

### Windows (MinGW)

```powershell
cd in_memory_detector
mkdir build; cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
cd ..\..

# Repeat for decoupled_detector
```

---

## Running the Tests

From within each module's `build` directory:

### Linux / macOS
```bash
ctest -V
```

### Windows
```powershell
ctest -V -C Release
```

All tests should pass. Tests that cannot locate a Haar cascade XML on the system will be skipped gracefully (not failed).

---

## Running the Pipelines

### In-Memory Pipeline (Fastest — All Platforms)

Streams the video directly into memory and extracts faces on the fly. No intermediate disk I/O.

**Linux / macOS:**
```bash
./in_memory_detector/build/detect_faces_video <input_video.mp4> <output_dir> [cascade_xml]

# Example (cascade auto-detected):
./in_memory_detector/build/detect_faces_video video1.mp4 in_memory_detected_faces
```

**Windows:**
```powershell
.\in_memory_detector\build\Release\detect_faces_video.exe <input_video.mp4> <output_dir> [cascade_xml]

# Example:
.\in_memory_detector\build\Release\detect_faces_video.exe video1.mp4 in_memory_detected_faces
```

> **Windows users:** If the cascade is not auto-detected, provide it explicitly as the third argument:
> ```
> .\detect_faces_video.exe video1.mp4 output C:\opencv\etc\haarcascades\haarcascade_frontalface_default.xml
> ```

### Decoupled Pipeline

Extracts full video frames to disk as JPEGs, then runs face detection over the directory.

**Step 1: Extract Frames**

*Linux / macOS (GStreamer):*
```bash
./extract_frames.sh <input.mp4> <output_jpeg_dir> [fps]

# Example:
./extract_frames.sh video1.mp4 jpeg_frames 3
```

*Windows / macOS without GStreamer (ffmpeg fallback):*
```bash
ffmpeg -i video1.mp4 -vf "fps=3,scale=640:640:force_original_aspect_ratio=decrease,pad=640:640:(ow-iw)/2:(oh-ih)/2" -q:v 2 decoupled_detector/jpeg_frames/frame_%06d.jpg
```

**Step 2: Detect Faces**

*Linux / macOS:*
```bash
./decoupled_detector/build/detect_faces <input_jpeg_dir> <output_dir> [cascade_xml_path]

# Example:
./decoupled_detector/build/detect_faces jpeg_frames detected_faces
```

*Windows:*
```powershell
.\decoupled_detector\build\Release\detect_faces.exe jpeg_frames detected_faces [cascade_xml_path]
```

---

## Automated Benchmarking

### Cross-Platform (Python — Recommended)

Works on Linux, macOS, and Windows. Requires Python 3 and either GStreamer or ffmpeg for frame extraction.

```bash
python3 benchmark_script.py [input_video.mp4]

# Example:
python3 benchmark_script.py video1.mp4
```

### Linux / macOS Only (Bash)

```bash
./benchmark_script.sh [input_video.mp4]

# Example:
./benchmark_script.sh video1.mp4
```

Both scripts generate a `benchmark_results.md` report comparing execution time, disk I/O, and face detection counts.

---

## Cleaning the Build

### Linux / macOS
```bash
rm -rf in_memory_detector/build
rm -rf decoupled_detector/build
```

### Windows
```powershell
Remove-Item -Recurse -Force in_memory_detector\build
Remove-Item -Recurse -Force decoupled_detector\build
```

---

## Platform Support Matrix

| Feature | Linux | macOS (Intel) | macOS (Apple Silicon) | Windows |
|---------|:-----:|:-------------:|:---------------------:|:-------:|
| In-Memory Detector | ✅ | ✅ | ✅ | ✅ |
| Decoupled Detector | ✅ | ✅ | ✅ | ✅ |
| Cascade Auto-Discovery | ✅ | ✅ | ✅ | ✅ |
| Unit Tests (GTest) | ✅ | ✅ | ✅ | ✅ |
| Shell Scripts (.sh) | ✅ | ✅ | ✅ | ❌ |
| Python Benchmark | ✅ | ✅ | ✅ | ✅ |
| GStreamer Extraction | ✅ | ✅ | ✅ | ⚠️ Manual |
| ffmpeg Extraction | ✅ | ✅ | ✅ | ✅ |
