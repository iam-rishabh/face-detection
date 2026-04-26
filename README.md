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
mkdir build && cd build
cmake ..
make
```
This will compile `detect_faces_video` and the `test_detector` binary.

### 2. Build the Decoupled Detector
Navigate to the `decoupled_detector` directory, generate the build files, and compile:

```bash
cd decoupled_detector
mkdir build && cd build
cmake ..
make
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

## Running the Pipelines

### In-Memory Pipeline (Fastest)
This pipeline streams the video directly into memory and extracts faces on the fly, saving significant disk space and execution time.
```bash
./in_memory_detector/build/detect_faces_video <input_video.mp4> <output_dir> [cascade_xml]

# Example (cascade auto-detected):
./in_memory_detector/build/detect_faces_video video1.mp4 in_memory_detected_faces
```

### Decoupled Pipeline
This pipeline first extracts full video frames to the disk as JPEGs using GStreamer, and then runs the face detector over the directory.

**Step 1: Extract Frames**
```bash
./extract_frames.sh <input.mp4> <output_jpeg_dir> [fps]

# Example:
./extract_frames.sh video1.mp4 jpeg_frames 3
```

**Step 2: Detect Faces**
```bash
./decoupled_detector/build/detect_faces <input_jpeg_dir> <output_dir> [cascade_xml_path]

# Example:
./decoupled_detector/build/detect_faces jpeg_frames detected_faces
```

### Automated Benchmarking
To automatically run both pipelines back-to-back and generate a comparative markdown report on execution speed and I/O efficiency:
```bash
./benchmark_script.sh <input_video.mp4>

# Example:
./benchmark_script.sh video1.mp4
```

---

## Cleaning the Build
If you need to perform a clean build, simply remove the `build/` directories:
```bash
rm -rf in_memory_detector/build
rm -rf decoupled_detector/build
```
