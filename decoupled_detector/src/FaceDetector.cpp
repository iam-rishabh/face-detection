#include "FaceDetector.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace std;

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
        cout << "\r[";
        for (int i = 0; i < bar_width; ++i) {
            if (i < pos) cout << "=";
            else if (i == pos) cout << ">";
            else cout << " ";
        }
        cout << "] " << int(ratio * 100) << "% (" 
                  << current << "/" << total << " frames)" << flush;
    }
    
    void finish() { 
        cout << endl; 
    }
    
    int getCurrent() const { return current; }
};

FaceDetector::FaceDetector(const string& cascade_path) {
    if (!cascade_.load(cascade_path)) {
        throw runtime_error("Failed to load cascade: " + cascade_path);
    }
    cout << "Cascade classifier loaded successfully" << endl;
}

void FaceDetector::processDirectory(const string& input_dir,
                                    const string& output_base_dir,
                                    bool show_progress) {
    namespace fs = filesystem;
    
    vector<fs::path> jpegs;
    try {
        for (const auto& entry : fs::directory_iterator(input_dir)) {
            string ext = entry.path().extension().string();
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".JPG" || ext == ".JPEG") {
                jpegs.push_back(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        cerr << "Error: Could not open directory " << input_dir << endl;
        return;
    }
    
    sort(jpegs.begin(), jpegs.end());

    if (jpegs.empty()) {
        cerr << "Error: No JPEG files found in " << input_dir << endl;
        return;
    }

    cout << "Found " << jpegs.size() << " frames to process" << endl;
    
    cv::Mat first_frame = cv::imread(jpegs[0].string(), cv::IMREAD_COLOR);
    if (!first_frame.empty()) {
        cout << "Frame dimensions: " << first_frame.cols << "x" 
                  << first_frame.rows << endl;
    }

    cout << "Processing frames..." << endl;
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

        vector<cv::Rect> faces;
        cascade_.detectMultiScale(gray, faces, 1.1, 4, 
                                  0 | cv::CASCADE_SCALE_IMAGE,
                                  cv::Size(30, 30));

        if (!faces.empty()) {
            frames_with_faces++;
            faces_found_total += faces.size();
            
            string stem = jpeg_path.stem().string();
            int frame_num = 0;
            string digits;
            for (char c : stem) {
                if (isdigit(c)) digits += c;
            }
            if (!digits.empty()) {
                try {
                    frame_num = stoi(digits);
                } catch (...) {
                    frame_num = bar.getCurrent();
                }
            }
            
            saveFaces(frame, faces, frame_num, output_base_dir);
        }

        if (show_progress) bar.update();
    }
    
    if (show_progress) bar.finish();
    
    cout << "\n=========================================" << endl;
    cout << "Processing Complete!" << endl;
    cout << "=========================================" << endl;
    cout << "Total frames processed: " << jpegs.size() << endl;
    cout << "Frames with faces: " << frames_with_faces << endl;
    cout << "Total faces detected: " << faces_found_total << endl;
    cout << "Output directory: " << output_base_dir << endl;
}

void FaceDetector::saveFaces(const cv::Mat& frame, 
                             const vector<cv::Rect>& faces,
                             int frame_num, 
                             const string& out_base) {
    namespace fs = filesystem;
    
    ostringstream frame_suffix;
    frame_suffix << "frame_" << setw(6) << setfill('0') << frame_num;
    fs::path dir_path = fs::path(out_base) / frame_suffix.str();
             
    try {
        fs::create_directories(dir_path);
    } catch (const fs::filesystem_error& e) {
        cerr << "Error: Could not create output directory: " << e.what() << endl;
        return;
    }

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
        
        ostringstream face_name;
        face_name << "face_" << setw(4) << setfill('0') << count << ".jpeg";
        fs::path face_path = dir_path / face_name.str();
        
        if (!cv::imwrite(face_path.string(), face_roi)) {
            cerr << "Error: Failed to save face image to " << face_path.string() << endl;
        }
        count++;
    }
}