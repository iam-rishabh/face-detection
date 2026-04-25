#!/bin/bash

# Configuration
INPUT=${1:-"video1.mp4"}
DECOUPLED_JPEG_DIR="decoupled_detector/jpeg_frames"
DECOUPLED_FACES_DIR="decoupled_detector/detected_faces"
IN_MEMORY_FACES_DIR="in_memory_detector/in_memory_detected_faces"
RESULTS_FILE="benchmark_results.md"

if [ ! -f "$INPUT" ]; then
    echo "Error: Input video '$INPUT' not found."
    exit 1
fi

if ! command -v bc &> /dev/null; then
    echo "Error: bc could not be found. Please install bc."
    exit 1
fi

if ! command -v du &> /dev/null; then
    echo "Error: du could not be found. Please install coreutils."
    exit 1
fi

echo "==========================================================="
echo "   Face Detection Benchmark: Decoupled vs. In-Memory"
echo "==========================================================="
echo "Input Video: $INPUT"
echo ""

# Cleanup previous runs
echo "Cleaning up previous outputs..."
rm -rf "$DECOUPLED_JPEG_DIR"
rm -rf "$DECOUPLED_FACES_DIR"
rm -rf "$IN_MEMORY_FACES_DIR"

# ---------------------------------------------------------
# Run Decoupled Pipeline
# ---------------------------------------------------------
echo "-----------------------------------------------------------"
echo "Running Decoupled Pipeline..."
echo "-----------------------------------------------------------"
# Phase 1: Extract frames
start_time=$(date +%s.%N)
./extract_frames.sh "$INPUT" "jpeg_frames" 3 > /dev/null 2>&1
# Measure intermediate size
decoupled_io_bytes=$(du -sb "$DECOUPLED_JPEG_DIR" | cut -f1)
decoupled_io_mb=$(echo "scale=2; $decoupled_io_bytes / 1048576" | bc)

# Phase 2: Detect faces
./decoupled_detector/build/detect_faces "$DECOUPLED_JPEG_DIR" "detected_faces" > /dev/null 2>&1
end_time=$(date +%s.%N)

decoupled_time=$(echo "$end_time - $start_time" | bc)
decoupled_faces=$(find "$DECOUPLED_FACES_DIR" -type f -name "*.jpeg" 2>/dev/null | wc -l)

echo "Decoupled pipeline completed in ${decoupled_time} seconds."

# ---------------------------------------------------------
# Run In-Memory Pipeline
# ---------------------------------------------------------
echo "-----------------------------------------------------------"
echo "Running In-Memory Pipeline..."
echo "-----------------------------------------------------------"
start_time=$(date +%s.%N)
./in_memory_detector/build/detect_faces_video "$INPUT" "in_memory_detected_faces" > /dev/null 2>&1
end_time=$(date +%s.%N)

in_memory_time=$(echo "$end_time - $start_time" | bc)
in_memory_faces=$(find "$IN_MEMORY_FACES_DIR" -type f -name "*.jpeg" 2>/dev/null | wc -l)

echo "In-Memory pipeline completed in ${in_memory_time} seconds."

# ---------------------------------------------------------
# Generate Report
# ---------------------------------------------------------
echo ""
echo "==========================================================="
echo "   Benchmark Results"
echo "==========================================================="

# Create Markdown content
cat << EOF > "$RESULTS_FILE"
# Face Detection Performance Benchmark

**Input Video**: \`$INPUT\`

| Metric | Decoupled Pipeline | In-Memory Pipeline |
|--------|-------------------|--------------------|
| **Execution Time (s)** | $decoupled_time | $in_memory_time |
| **Intermediate Disk I/O (MB)** | $decoupled_io_mb | 0.00 |
| **Total Faces Detected** | $decoupled_faces | $in_memory_faces |

## Conclusion
- **Speed**: The In-Memory pipeline was $(echo "scale=2; $decoupled_time / $in_memory_time" | bc)x faster.
- **I/O Efficiency**: The In-Memory pipeline completely eliminated the $decoupled_io_mb MB of disk reads/writes required by the decoupled approach.
EOF

cat "$RESULTS_FILE"
echo ""
echo "Results saved to $RESULTS_FILE"
