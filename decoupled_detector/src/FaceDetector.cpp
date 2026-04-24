#include "FaceDetector.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

class ProgressBar {
    int total;
    int current;
    int bar_width;
public:
    ProgressBar(int t) : total(t), current(0), bar_width(50) {}
    
    void update(int inc = 1) {
        current += inc;
        if (total <= 0) return;
        float ratio = (float)current / total;
        int pos = bar_width * ratio;
        std::cout << "\r[";
        for (int i = 0; i < bar_width; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(ratio * 100) << "% (" 
                  << current << "/" << total << " frames)" << std::flush;
    }
    
    void finish() { 
        std::cout << std::endl; 
    }
    
    int getCurrent() const { return current; }
};

FaceDetector::FaceDetector(const std::string& cascade_path) {
    if (!cascade_.load(cascade_path)) {
        throw std::runtime_error("Failed to load cascade: " + cascade_path);
    }
    std::cout << "Cascade classifier loaded successfully" << std::endl;
}

void FaceDetector::processDirectory(const std::string& input_dir,
                                    const std::string& output_base_dir,
                                    bool show_progress) {
    namespace fs = std::filesystem;
    
    std::vector<fs::path> jpegs;
    for (const auto& entry : fs::directory_iterator(input_dir)) {
        std::string ext = entry.path().extension().string();
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".JPG" || ext == ".JPEG") {
            jpegs.push_back(entry.path());
        }
    }
    
    std::sort(jpegs.begin(), jpegs.end());

    if (jpegs.empty()) {
        std::cerr << "Error: No JPEG files found in " << input_dir << std::endl;
        return;
    }

    std::cout << "Found " << jpegs.size() << " frames to process" << std::endl;
    
    cv::Mat first_frame = cv::imread(jpegs[0].string(), cv::IMREAD_COLOR);
    if (!first_frame.empty()) {
        std::cout << "Frame dimensions: " << first_frame.cols << "x" 
                  << first_frame.rows << std::endl;
    }

    std::cout << "Processing frames..." << std::endl;
    ProgressBar bar(jpegs.size());
    
    int faces_found_total = 0;
    int frames_with_faces = 0;

    for (const auto& jpeg_path : jpegs) {
        cv::Mat frame = cv::imread(jpeg_path.string(), cv::IMREAD_COLOR);
        if (frame.empty()) {
            if (show_progress) bar.update();
            continue;
        }

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);

        std::vector<cv::Rect> faces;
        cascade_.detectMultiScale(gray, faces, 1.1, 4, 
                                  0 | cv::CASCADE_SCALE_IMAGE,
                                  cv::Size(30, 30));

        if (!faces.empty()) {
            frames_with_faces++;
            faces_found_total += faces.size();
            
            std::string stem = jpeg_path.stem().string();
            int frame_num = 0;
            std::string digits;
            for (char c : stem) {
                if (std::isdigit(c)) digits += c;
            }
            if (!digits.empty()) {
                try {
                    frame_num = std::stoi(digits);
                } catch (...) {
                    frame_num = bar.getCurrent();
                }
            }
            
            saveFaces(frame, faces, frame_num, output_base_dir);
        }

        if (show_progress) bar.update();
    }
    
    if (show_progress) bar.finish();
    
    std::cout << "\n=========================================" << std::endl;
    std::cout << "Processing Complete!" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Total frames processed: " << jpegs.size() << std::endl;
    std::cout << "Frames with faces: " << frames_with_faces << std::endl;
    std::cout << "Total faces detected: " << faces_found_total << std::endl;
    std::cout << "Output directory: " << output_base_dir << std::endl;
}

void FaceDetector::saveFaces(const cv::Mat& frame, 
                             const std::vector<cv::Rect>& faces,
                             int frame_num, 
                             const std::string& out_base) {
    namespace fs = std::filesystem;
    
    std::ostringstream dir_name;
    dir_name << out_base << "/frame_" 
             << std::setw(6) << std::setfill('0') << frame_num;
    fs::create_directories(dir_name.str());

    int count = 1;
    for (const auto& r : faces) {
        cv::Rect safe = r & cv::Rect(0, 0, frame.cols, frame.rows);
        
        if (safe.width <= 0 || safe.height <= 0 || safe.area() == 0) {
            continue;
        }
        
        cv::Mat face_roi = frame(safe);
        cv::Scalar mean_color = cv::mean(face_roi);
        
        if (mean_color[0] < 10 && mean_color[1] < 10 && mean_color[2] < 10) {
            continue;
        }
        
        std::ostringstream fname;
        fname << dir_name.str() << "/face_" 
              << std::setw(4) << std::setfill('0') << count << ".jpeg";
        
        cv::imwrite(fname.str(), face_roi);
        count++;
    }
}