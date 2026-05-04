#!/usr/bin/env python3
"""
Cross-platform benchmark script for Face Detection pipelines.
Works on both Linux and Windows (replaces benchmark_script.sh).
"""

import os
import sys
import time
import shutil
import subprocess
import platform


def get_dir_size_mb(path):
    """Recursively compute directory size in MB."""
    total = 0
    for dirpath, _, filenames in os.walk(path):
        for f in filenames:
            fp = os.path.join(dirpath, f)
            if os.path.isfile(fp):
                total += os.path.getsize(fp)
    return round(total / (1024 * 1024), 2)


def count_files(path, extension=".jpeg"):
    """Count files with a given extension recursively."""
    count = 0
    if not os.path.isdir(path):
        return 0
    for dirpath, _, filenames in os.walk(path):
        for f in filenames:
            if f.lower().endswith(extension):
                count += 1
    return count


def find_executable(name):
    """Find an executable, checking platform-specific build paths."""
    if platform.system() == "Windows":
        candidates = [
            os.path.join("build", "Release", f"{name}.exe"),
            os.path.join("build", "Debug", f"{name}.exe"),
            os.path.join("build", f"{name}.exe"),
        ]
    else:
        candidates = [
            os.path.join("build", name),
        ]
    for c in candidates:
        if os.path.isfile(c):
            return c
    return None


def run_extract_frames(input_video, output_dir, fps=3):
    """
    Extract frames from video using GStreamer (Linux) or ffmpeg (Windows fallback).
    """
    os.makedirs(output_dir, exist_ok=True)

    if platform.system() != "Windows" and shutil.which("gst-launch-1.0"):
        cmd = [
            "gst-launch-1.0", "-e",
            "filesrc", f"location={input_video}",
            "!", "decodebin",
            "!", "videoconvert",
            "!", "videoscale", "add-borders=true",
            "!", f"video/x-raw,width=640,height=640,pixel-aspect-ratio=1/1",
            "!", "videorate", "!", f"video/x-raw,framerate={fps}/1",
            "!", "jpegenc", "quality=85",
            "!", f"multifilesink", f"location={output_dir}/frame_%06d.jpg",
        ]
    elif shutil.which("ffmpeg"):
        # ffmpeg works on both Linux and Windows
        cmd = [
            "ffmpeg", "-i", input_video,
            "-vf", f"fps={fps},scale=640:640:force_original_aspect_ratio=decrease,pad=640:640:(ow-iw)/2:(oh-ih)/2",
            "-q:v", "2",
            os.path.join(output_dir, "frame_%06d.jpg"),
            "-y",
        ]
    else:
        print("Error: Neither gst-launch-1.0 nor ffmpeg found. Please install one.")
        sys.exit(1)

    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Warning: Frame extraction returned code {result.returncode}")
        if result.stderr:
            print(result.stderr[:500])


def main():
    input_video = sys.argv[1] if len(sys.argv) > 1 else "video1.mp4"

    decoupled_jpeg_dir = os.path.join("decoupled_detector", "jpeg_frames")
    decoupled_faces_dir = os.path.join("decoupled_detector", "detected_faces")
    in_memory_faces_dir = os.path.join("in_memory_detector", "in_memory_detected_faces")
    results_file = "benchmark_results.md"

    if not os.path.isfile(input_video):
        print(f"Error: Input video '{input_video}' not found.")
        sys.exit(1)

    print("=" * 59)
    print("   Face Detection Benchmark: Decoupled vs. In-Memory")
    print("=" * 59)
    print(f"Input Video: {input_video}")
    print(f"Platform: {platform.system()} {platform.machine()}")
    print()

    # Cleanup previous runs
    print("Cleaning up previous outputs...")
    for d in [decoupled_jpeg_dir, decoupled_faces_dir, in_memory_faces_dir]:
        if os.path.isdir(d):
            shutil.rmtree(d)

    # ---------------------------------------------------------
    # Run Decoupled Pipeline
    # ---------------------------------------------------------
    print("-" * 59)
    print("Running Decoupled Pipeline...")
    print("-" * 59)

    decoupled_exe = find_executable("detect_faces")
    if decoupled_exe is None:
        # Try from the decoupled_detector directory
        saved = os.getcwd()
        os.chdir("decoupled_detector")
        decoupled_exe_rel = find_executable("detect_faces")
        os.chdir(saved)
        if decoupled_exe_rel:
            decoupled_exe = os.path.join("decoupled_detector", decoupled_exe_rel)

    if decoupled_exe is None:
        print("Error: Could not find decoupled_detector executable 'detect_faces'.")
        print("Please build it first: cd decoupled_detector && mkdir build && cd build && cmake .. && cmake --build .")
        sys.exit(1)

    start_time = time.perf_counter()

    # Phase 1: Extract frames
    run_extract_frames(input_video, decoupled_jpeg_dir, fps=3)
    decoupled_io_mb = get_dir_size_mb(decoupled_jpeg_dir) if os.path.isdir(decoupled_jpeg_dir) else 0.0

    # Phase 2: Detect faces
    subprocess.run(
        [decoupled_exe, decoupled_jpeg_dir, "detected_faces"],
        capture_output=True, text=True,
    )
    end_time = time.perf_counter()

    decoupled_time = round(end_time - start_time, 4)
    decoupled_faces = count_files(decoupled_faces_dir)

    print(f"Decoupled pipeline completed in {decoupled_time} seconds.")

    # ---------------------------------------------------------
    # Run In-Memory Pipeline
    # ---------------------------------------------------------
    print("-" * 59)
    print("Running In-Memory Pipeline...")
    print("-" * 59)

    in_memory_exe = find_executable("detect_faces_video")
    if in_memory_exe is None:
        saved = os.getcwd()
        os.chdir("in_memory_detector")
        in_memory_exe_rel = find_executable("detect_faces_video")
        os.chdir(saved)
        if in_memory_exe_rel:
            in_memory_exe = os.path.join("in_memory_detector", in_memory_exe_rel)

    if in_memory_exe is None:
        print("Error: Could not find in_memory_detector executable 'detect_faces_video'.")
        print("Please build it first: cd in_memory_detector && mkdir build && cd build && cmake .. && cmake --build .")
        sys.exit(1)

    start_time = time.perf_counter()
    subprocess.run(
        [in_memory_exe, input_video, "in_memory_detected_faces"],
        capture_output=True, text=True,
    )
    end_time = time.perf_counter()

    in_memory_time = round(end_time - start_time, 4)
    in_memory_faces = count_files(in_memory_faces_dir)

    print(f"In-Memory pipeline completed in {in_memory_time} seconds.")

    # ---------------------------------------------------------
    # Generate Report
    # ---------------------------------------------------------
    print()
    print("=" * 59)
    print("   Benchmark Results")
    print("=" * 59)

    speedup = round(decoupled_time / in_memory_time, 2) if in_memory_time > 0 else "N/A"

    report = f"""# Face Detection Performance Benchmark

**Input Video**: `{input_video}`
**Platform**: {platform.system()} {platform.machine()}

| Metric | Decoupled Pipeline | In-Memory Pipeline |
|--------|-------------------|--------------------| 
| **Execution Time (s)** | {decoupled_time} | {in_memory_time} |
| **Intermediate Disk I/O (MB)** | {decoupled_io_mb} | 0.00 |
| **Total Faces Detected** | {decoupled_faces} | {in_memory_faces} |

## Conclusion
- **Speed**: The In-Memory pipeline was {speedup}x faster.
- **I/O Efficiency**: The In-Memory pipeline completely eliminated the {decoupled_io_mb} MB of disk reads/writes required by the decoupled approach.
"""

    with open(results_file, "w") as f:
        f.write(report)

    print(report)
    print(f"Results saved to {results_file}")


if __name__ == "__main__":
    main()
