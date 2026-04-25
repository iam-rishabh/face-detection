#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <filesystem>

using namespace std;

class FaceDetector {
public:
    FaceDetector(const string& cascade_path);
    
    // Process a video file directly from disk without creating intermediate files
    void processVideo(const string& input_video,
                      const string& output_base_dir,
                      int target_fps = 3,
                      bool progress = true);
private:
    cv::CascadeClassifier cascade_;
    
    void saveFaces(const cv::Mat& frame, 
                   const vector<cv::Rect>& faces,
                   int frame_num, 
                   const string& out_base);
};
