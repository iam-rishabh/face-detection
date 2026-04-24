cmake_minimum_required(VERSION 3.10)
project(decoupled_detector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED COMPONENTS core imgproc objdetect imgcodecs)

# Print OpenCV info for debugging
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")

# Create executable
add_executable(detect_faces 
    src/main.cpp 
    src/FaceDetector.cpp
)

# Include directories
target_include_directories(detect_faces PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(detect_faces 
    ${OpenCV_LIBS}
)