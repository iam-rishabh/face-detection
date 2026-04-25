#include <gtest/gtest.h>
#include "FaceDetector.hpp"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>

using namespace std;

class FaceDetectorTest : public ::testing::Test {
protected:
    string valid_cascade;
    string invalid_cascade;
    string test_video;
    string test_out_dir;

    void SetUp() override {
        // Try to find a valid cascade path
        vector<string> paths = {
            "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
            "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml",
            "/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
            "haarcascade_frontalface_default.xml"
        };
        for (const auto& p : paths) {
            if (filesystem::exists(p)) {
                valid_cascade = p;
                break;
            }
        }
        
        invalid_cascade = "invalid_path_to_cascade.xml";
        test_video = "test_video.mp4";
        test_out_dir = "test_output";
        
        // Create a dummy video file using OpenCV
        if (!valid_cascade.empty()) {
            cv::VideoWriter writer(test_video, cv::VideoWriter::fourcc('m','p','4','v'), 10, cv::Size(640, 640));
            cv::Mat frame = cv::Mat::zeros(640, 640, CV_8UC3);
            // Draw a white rectangle to simulate some content
            cv::rectangle(frame, cv::Point(200, 200), cv::Point(400, 400), cv::Scalar(255, 255, 255), -1);
            for (int i = 0; i < 30; ++i) { // 3 seconds video at 10 fps
                writer.write(frame);
            }
            writer.release();
        }
    }

    void TearDown() override {
        if (filesystem::exists(test_video)) {
            filesystem::remove(test_video);
        }
        if (filesystem::exists(test_out_dir)) {
            filesystem::remove_all(test_out_dir);
        }
        // Cleanup if the class created it with the prepended directory
        if (filesystem::exists("in_memory_detector/test_output")) {
            filesystem::remove_all("in_memory_detector/test_output");
        }
    }
};

TEST_F(FaceDetectorTest, InvalidCascadeThrowsException) {
    EXPECT_THROW(FaceDetector detector(invalid_cascade), runtime_error);
}

TEST_F(FaceDetectorTest, ValidCascadeLoadsSuccessfully) {
    if (valid_cascade.empty()) {
        GTEST_SKIP() << "No valid Haar cascade found on system, skipping test.";
    }
    EXPECT_NO_THROW(FaceDetector detector(valid_cascade));
}

TEST_F(FaceDetectorTest, ProcessMissingVideo) {
    if (valid_cascade.empty()) {
        GTEST_SKIP() << "No valid Haar cascade found on system, skipping test.";
    }
    FaceDetector detector(valid_cascade);
    // Should not crash, just print error and return
    testing::internal::CaptureStderr();
    detector.processVideo("non_existent_video.mp4", test_out_dir, 3, false);
    string stderr_output = testing::internal::GetCapturedStderr();
    EXPECT_NE(stderr_output.find("Error: Could not open video"), string::npos);
}

TEST_F(FaceDetectorTest, ProcessValidVideo) {
    if (valid_cascade.empty()) {
        GTEST_SKIP() << "No valid Haar cascade found on system, skipping test.";
    }
    FaceDetector detector(valid_cascade);
    
    // We run it quietly
    EXPECT_NO_THROW(detector.processVideo(test_video, test_out_dir, 3, false));
    
    // The test video doesn't have actual faces, so it shouldn't have created any face crops,
    // but the output directory should exist if faces were found. Since it's a blank video, 
    // it won't crash, which is the main thing we're testing here.
}
