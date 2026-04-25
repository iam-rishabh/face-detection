find_package(GTest REQUIRED)

add_executable(test_detector test_detector.cpp ../src/FaceDetector.cpp)

target_include_directories(test_detector PRIVATE ../src)
target_link_libraries(test_detector GTest::GTest GTest::Main ${OpenCV_LIBS})

# Add test to CTest
add_test(NAME InMemoryFaceDetectorTests COMMAND test_detector)
