# Compilation Guide

This repository uses CMake to build the decoupled and in-memory face detectors. Follow these steps to build the project from source.

## Prerequisites
Ensure you have the following installed on your system:
- `cmake` (>= 3.10)
- `g++` (or another C++17 compatible compiler)
- `libopencv-dev` (OpenCV 4 core, imgproc, objdetect, imgcodecs, videoio)
- `libgtest-dev` (Google Test for the testing suites)
- `gstreamer1.0-tools` (For the decoupled pipeline extraction bash script)

*(On Ubuntu, you can install the core C++ dependencies via `sudo apt install cmake g++ libopencv-dev libgtest-dev`)*

---

## Building the Detectors

Since the project contains two separate modules, you must build them individually.

### 1. Build the In-Memory Detector
Navigate to the `in_memory_detector` directory, generate the build files, and compile:

```bash
cd in_memory_detector
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```
This will compile `detect_faces_video` and the `test_detector` binary.

### 2. Build the Decoupled Detector
Navigate to the `decoupled_detector` directory, generate the build files, and compile:

```bash
cd decoupled_detector
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```
This will compile `detect_faces` and the `test_detector` binary.

---

## Running the Tests
To ensure everything compiled correctly and your OpenCV Haar cascades are functioning, run the test suites for both modules.

From within each respective `build` directory, run:
```bash
ctest -V
```
All tests should pass gracefully.

---

## Cleaning the Build
If you need to perform a clean build, simply remove the `build/` directories:
```bash
rm -rf in_memory_detector/build
rm -rf decoupled_detector/build
```
