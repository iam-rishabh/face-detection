#include "FaceDetector.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

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

void FaceDetector::processVideo(const string& input_video,
                                const string& output_base_dir,
                                int target_fps,
                                bool show_progress) {
    cv::VideoCapture cap(input_video);
    if (!cap.isOpened()) {
        cerr << "Error: Could not open video " << input_video << endl;
        return;
    }

    double fps = cap.get(cv::CAP_PROP_FPS);
    int frame_count = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    
    cout << "Video FPS: " << fps << endl;
    cout << "Video Total Frames: " << frame_count << endl;

    int skip_frames = 1;
    if (target_fps > 0 && target_fps < fps) {
        skip_frames = static_cast<int>(fps / target_fps);
    }
    cout << "Target FPS: " << target_fps << ", extracting every " << skip_frames << " frame(s)." << endl;

    int estimated_frames_to_process = frame_count / skip_frames;
    cout << "Processing roughly " << estimated_frames_to_process << " frames..." << endl;

    ProgressBar bar(estimated_frames_to_process);
    
    int faces_found_total = 0;
    int frames_with_faces = 0;
    int current_frame_idx = 0;
    int processed_frames_count = 0;

    cv::Mat frame;
    while (cap.read(frame)) {
        if (current_frame_idx % skip_frames != 0) {
            current_frame_idx++;
            continue;
        }

        // Resize to 640x640 with aspect ratio preservation to mimic bash script behavior
        // (For simplicity we just resize to a fixed square or pad, here we just do standard resize for the face detection input)
        cv::Mat resized_frame;
        // In the bash script, videoscale add-borders=true is used. 
        // We'll pad it to square to replicate the 640x640 aspect ratio preservation:
        int max_dim = max(frame.cols, frame.rows);
        cv::Mat square_frame = cv::Mat::zeros(max_dim, max_dim, frame.type());
        int dx = (max_dim - frame.cols) / 2;
        int dy = (max_dim - frame.rows) / 2;
        frame.copyTo(square_frame(cv::Rect(dx, dy, frame.cols, frame.rows)));
        
        cv::resize(square_frame, resized_frame, cv::Size(640, 640));

        cv::Mat gray;
        cv::cvtColor(resized_frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);

        vector<cv::Rect> faces;
        cascade_.detectMultiScale(gray, faces, 1.1, 4, 
                                  0 | cv::CASCADE_SCALE_IMAGE,
                                  cv::Size(30, 30));

        if (!faces.empty()) {
            frames_with_faces++;
            faces_found_total += faces.size();
            // Use current_frame_idx as the frame number for output file naming
            saveFaces(resized_frame, faces, current_frame_idx, output_base_dir);
        }

        if (show_progress) bar.update();
        processed_frames_count++;
        current_frame_idx++;
    }
    
    if (show_progress) bar.finish();
    
    cout << "\n=========================================" << endl;
    cout << "Processing Complete!" << endl;
    cout << "=========================================" << endl;
    cout << "Total frames processed: " << processed_frames_count << endl;
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
