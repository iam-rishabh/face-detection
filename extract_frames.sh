#!/bin/bash
INPUT=${1:?"Usage: $0 <input.mp4>"}
OUTDIR=${2:-"jpeg_frames"}
if [[ "$OUTDIR" != /* ]] && [[ "$OUTDIR" != *"decoupled_detector"* ]]; then
    OUTDIR="decoupled_detector/$OUTDIR"
fi
FPS=${3:-3}
if [ ! -f "$INPUT" ]; then
    echo "Error: Input video '$INPUT' not found."
    exit 1
fi

if ! command -v gst-launch-1.0 &> /dev/null; then
    echo "Error: gst-launch-1.0 could not be found. Please install gstreamer1.0-tools."
    exit 1
fi

if ! command -v identify &> /dev/null; then
    echo "Error: identify could not be found. Please install imagemagick."
    exit 1
fi

mkdir -p "$OUTDIR"

echo "Extracting frames with aspect ratio preservation..."
echo "Input: $INPUT"
echo "Output: $OUTDIR"
echo "FPS: $FPS"
echo "Target size: 640x640"

gst-launch-1.0 -e \
  filesrc location="$INPUT" \
  ! decodebin \
  ! videoconvert \
  ! videoscale add-borders=true \
  ! video/x-raw,width=640,height=640,pixel-aspect-ratio=1/1 \
  ! videorate ! video/x-raw,framerate=$FPS/1 \
  ! jpegenc quality=85 \
  ! multifilesink location="$OUTDIR/frame_%06d.jpg"

# Verify first frame dimensions
FIRST_FRAME=$(ls "$OUTDIR" | head -1)
if [ -n "$FIRST_FRAME" ]; then
    DIMENSIONS=$(identify "$OUTDIR/$FIRST_FRAME" 2>/dev/null | cut -d' ' -f3)
    echo "Verification :: First frame dimensions: $DIMENSIONS"
    if [ "$DIMENSIONS" = "640x640" ]; then
        echo "SUCCESS :: Padding verified - frames are exactly 640x640"
    else
        echo "Warning :: Frame dimensions are $DIMENSIONS, expected 640x640"
    fi
fi