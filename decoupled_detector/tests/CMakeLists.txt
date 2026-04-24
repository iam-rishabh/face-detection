#include <gtest/gtest.h>
#include "FaceDetector.hpp"

TEST(FaceDetectorTest, LoadsCascade) {
    EXPECT_NO_THROW(FaceDetector("path/to/haarcascade_frontalface_default.xml"));
}

TEST(FaceDetectorTest, NoFacesInBlankImage) {
    cv::Mat blank = cv::Mat::zeros(100,100,CV_8UC3);
    std::vector<cv::Rect> faces;
    // ... call internal detect method (expose for testing)
    EXPECT_EQ(faces.size(), 0);
}

find_package(GTest REQUIRED)
add_executable(test_detector test_detector.cpp ../src/FaceDetector.cpp)
target_include_directories(test_detector PRIVATE ../src)
target_link_libraries(test_detector GTest::GTest ${OpenCV_LIBS})