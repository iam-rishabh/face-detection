#include "FaceDetector.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv) {
    std::cout << "=========================================" << std::endl;
    std::cout << "Face Detection Tool (Decoupled CPU Mode)" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] 
                  << " <input_jpeg_dir> <output_dir> [cascade_xml]\n\n"
                  << "Example:\n"
                  << "  " << argv[0] 
                  << " jpeg_frames detected_faces\n"
                  << "  " << argv[0] 
                  << " jpeg_frames output /path/to/cascade.xml\n";
        return 1;
    }
    
    std::string input = argv[1];
    std::string output = argv[2];
    
    std::string cascade;
    if (argc >= 4) {
        cascade = argv[3];
    } else {
        std::vector<std::string> paths = {
            "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
            "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml",
            "/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
            "haarcascade_frontalface_default.xml"
        };
        for (const auto& p : paths) {
            if (std::filesystem::exists(p)) {
                cascade = p;
                break;
            }
        }
    }
    
    if (cascade.empty()) {
        std::cerr << "Error: Could not find Haar cascade XML file.\n"
                  << "Please provide path as third argument.\n";
        return 1;
    }
    
    std::cout << "Input directory: " << input << std::endl;
    std::cout << "Output directory: " << output << std::endl;
    std::cout << "Cascade file: " << cascade << std::endl;
    std::cout << "=========================================" << std::endl;
    
    try {
        FaceDetector detector(cascade);
        detector.processDirectory(input, output, true);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}