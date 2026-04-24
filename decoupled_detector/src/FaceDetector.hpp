#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <filesystem>

class FaceDetector {
public:
    FaceDetector(const std::string& cascade_path);
    
    void processDirectory(const std::string& input_dir,
                          const std::string& output_base_dir,
                          bool progress = true);
private:
    cv::CascadeClassifier cascade_;
    
    void saveFaces(const cv::Mat& frame, 
                   const std::vector<cv::Rect>& faces,
                   int frame_num, 
                   const std::string& out_base);
};