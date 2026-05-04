#include <gtest/gtest.h>
#include "FaceDetector.hpp"
#include <filesystem>
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace std;

class FaceDetectorTest : public ::testing::Test {
protected:
    string valid_cascade;
    string invalid_cascade;
    string test_in_dir;
    string test_out_dir;

    void SetUp() override {
        // Try to find a valid cascade path
        vector<string> paths = {
#ifdef _WIN32
            "C:/opencv/etc/haarcascades/haarcascade_frontalface_default.xml",
            "C:/tools/opencv/etc/haarcascades/haarcascade_frontalface_default.xml",
#elif defined(__APPLE__)
            "/opt/homebrew/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
            "/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
            "/usr/local/Cellar/opencv/*/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
#else
            "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
            "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml",
            "/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
#endif
            "haarcascade_frontalface_default.xml"
        };
        for (const auto& p : paths) {
            if (filesystem::exists(p)) {
                valid_cascade = p;
                break;
            }
        }
        
        invalid_cascade = "invalid_path_to_cascade.xml";
        test_in_dir = "test_input_frames";
        test_out_dir = "test_output";
        
        if (!valid_cascade.empty()) {
            filesystem::create_directories(test_in_dir);
            cv::Mat frame = cv::Mat::zeros(640, 640, CV_8UC3);
            cv::rectangle(frame, cv::Point(200, 200), cv::Point(400, 400), cv::Scalar(255, 255, 255), -1);
            filesystem::path frame_path = filesystem::path(test_in_dir) / "frame_000001.jpg";
            cv::imwrite(frame_path.string(), frame);
        }
    }

    void TearDown() override {
        if (filesystem::exists(test_in_dir)) {
            filesystem::remove_all(test_in_dir);
        }
        if (filesystem::exists(test_out_dir)) {
            filesystem::remove_all(test_out_dir);
        }
        // Cleanup if the class created it with the prepended directory
        if (filesystem::exists("decoupled_detector/test_output")) {
            filesystem::remove_all("decoupled_detector/test_output");
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

TEST_F(FaceDetectorTest, ProcessMissingDirectory) {
    if (valid_cascade.empty()) {
        GTEST_SKIP() << "No valid Haar cascade found on system, skipping test.";
    }
    FaceDetector detector(valid_cascade);
    // Should not crash, just print error and return
    testing::internal::CaptureStderr();
    detector.processDirectory("non_existent_directory", test_out_dir, false);
    string stderr_output = testing::internal::GetCapturedStderr();
    EXPECT_NE(stderr_output.find("Error: Could not open directory"), string::npos);
}

TEST_F(FaceDetectorTest, ProcessValidDirectory) {
    if (valid_cascade.empty()) {
        GTEST_SKIP() << "No valid Haar cascade found on system, skipping test.";
    }
    FaceDetector detector(valid_cascade);
    
    // We run it quietly
    EXPECT_NO_THROW(detector.processDirectory(test_in_dir, test_out_dir, false));
}
