#include "FaceDetector.hpp"
#include <iostream>
#include <cstdlib>
#include <filesystem>

using namespace std;

int main(int argc, char** argv) {
    cout << "=========================================" << endl;
    cout << "Face Detection Tool (In-Memory Video Mode)" << endl;
    cout << "=========================================" << endl;
    
    if (argc < 3) {
        cerr << "Usage: " << argv[0] 
                  << " <input_video.mp4> <output_dir> [cascade_xml]\n\n"
                  << "Example:\n"
                  << "  " << argv[0] 
                  << " video1.mp4 detected_faces\n"
                  << "  " << argv[0] 
                  << " video1.mp4 output /path/to/cascade.xml\n";
        return 1;
    }
    
    string input = argv[1];
    string output = argv[2];
    
    string cascade;
    if (argc >= 4) {
        cascade = argv[3];
    } else {
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
                cascade = p;
                break;
            }
        }
    }
    
    if (cascade.empty()) {
        cerr << "Error: Could not find Haar cascade XML file.\n"
                  << "Please provide path as third argument.\n";
        return 1;
    }
    
    namespace fs = filesystem;

    string final_output = output;
    if (output.find("in_memory_detector") == string::npos && !fs::path(output).is_absolute()) {
        final_output = (fs::path("in_memory_detector") / output).string();
    }

    cout << "Input Video: " << input << endl;
    cout << "Output directory: " << final_output << endl;
    cout << "Cascade file: " << cascade << endl;
    cout << "=========================================" << endl;
    
    try {
        FaceDetector detector(cascade);
        // Process the video at 3 FPS target by default
        detector.processVideo(input, final_output, 3, true);
        return 0;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}
