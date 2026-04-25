cmake_minimum_required(VERSION 3.10)
project(in_memory_detector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV with videoio for video processing
find_package(OpenCV REQUIRED COMPONENTS core imgproc objdetect imgcodecs videoio)

# Print OpenCV info for debugging
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")

# Create executable
add_executable(detect_faces_video 
    src/main.cpp 
    src/FaceDetector.cpp
)

# Include directories
target_include_directories(detect_faces_video PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(detect_faces_video 
    ${OpenCV_LIBS}
)

enable_testing()
add_subdirectory(tests)

