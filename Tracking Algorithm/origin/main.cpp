#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
// #include <opencv2/highgui.hpp> // Not strictly needed if not displaying windows
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

#include <iostream>
#include <vector>
#include <string>
// For creating directories if saving frames
// #include <sys/stat.h> // For mkdir on Linux/macOS
// #include <direct.h>   // For _mkdir on Windows

int main(int argc, char **argv) {
    std::string video_path;
    if (argc > 1) {
        video_path = argv[1];
    } else {
        video_path = "video.mp4"; // Default video file name
        std::cout << "Usage: ./ovx1_app_cn <video_file_path_or_frame_sequence_pattern>" << std::endl;
        std::cout << "No path provided, attempting to use default: " << video_path << std::endl;
    }

    cv::VideoCapture cap;
    cap.open(video_path);

    if (!cap.isOpened()) {
        // Attempt to open as an image sequence if video file open failed
        cap.open(video_path, cv::CAP_IMAGES);
    }

    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video or image sequence from path: " << video_path << std::endl;
        return -1;
    }

    cv::Mat frame_4k, frame_hd_rgb, prev_gray, current_gray;
    std::vector<cv::Point2f> p0, p1; // Feature points for previous and current frame

    // Parameters for ShiTomasi/Harris corner detection
    int maxCorners = 200;
    double qualityLevel = 0.01;
    double minDistance = 10;
    int blockSize_harris = 3;
    bool useHarrisDetector = false; // false for Shi-Tomasi, true for Harris
    double k_harris = 0.04;

    // Parameters for Lucas-Kanade optical flow
    cv::Size winSize(21, 21);
    int maxLevel = 3;
    cv::TermCriteria criteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 0.01);

    bool first_frame_processed = false;
    int frame_count = 0;
    const int MAX_FRAMES_TO_PROCESS = 180; // Process 180 frames

    std::cout << "Starting video processing..." << std::endl;

    // Optional: Create a directory for output frames if you uncomment imwrite
    // const std::string output_dir = "output_frames";
    // For Linux/macOS: mkdir(output_dir.c_str(), 0777);
    // For Windows: _mkdir(output_dir.c_str());


    while (frame_count < MAX_FRAMES_TO_PROCESS) {
        cap >> frame_4k;
        if (frame_4k.empty()) {
            std::cout << "Video or sequence ended at frame " << frame_count << "." << std::endl;
            break;
        }
        frame_count++;

        // --- 2. Image conversion: 4K -> 1280x720 RGB ---
        cv::resize(frame_4k, frame_hd_rgb, cv::Size(1280, 720));

        // Convert to grayscale for feature detection and tracking
        cv::cvtColor(frame_hd_rgb, current_gray, cv::COLOR_BGR2GRAY);

        if (!first_frame_processed) {
            // --- 3. Feature detection (first frame) ---
            cv::goodFeaturesToTrack(current_gray,
                                    p0,
                                    maxCorners,
                                    qualityLevel,
                                    minDistance,
                                    cv::Mat(),
                                    blockSize_harris,
                                    useHarrisDetector,
                                    k_harris);

            if (p0.empty()) {
                std::cerr << "Warning: No features detected in the first frame. Try adjusting parameters." << std::endl;
            } else {
                std::cout << "Frame " << frame_count << ": Detected " << p0.size() << " features." << std::endl;
            }
            prev_gray = current_gray.clone();
            first_frame_processed = true;
        } else if (!p0.empty()) {
            // --- 4. Feature tracking (subsequent frames) ---
            std::vector<uchar> status;
            std::vector<float> err;

            cv::calcOpticalFlowPyrLK(prev_gray, current_gray, p0, p1, status, err, winSize, maxLevel, criteria);

            std::vector<cv::Point2f> good_new_points;
            for (uint i = 0; i < p0.size(); i++) {
                if (status[i] == 1) {
                    good_new_points.push_back(p1[i]);
                    // Optional: If you want to draw on frame_hd_rgb for saving later
                    // cv::line(frame_hd_rgb, p0[i], p1[i], cv::Scalar(0, 255, 0), 2);
                    // cv::circle(frame_hd_rgb, p1[i], 3, cv::Scalar(0, 0, 255), -1);
                }
            }
            p0 = good_new_points;

            if (!p0.empty()) {
                std::cout << "Frame " << frame_count << ": Tracking " << p0.size() << " features." << std::endl;
            } else {
                std::cout << "Frame " << frame_count << ": All features lost." << std::endl;
            }

            prev_gray = current_gray.clone();

            if (p0.size() < static_cast<size_t>(maxCorners / 4)) {
                std::cout << "Too few features, attempting to redetect..." << std::endl;
                cv::goodFeaturesToTrack(current_gray, p0, maxCorners, qualityLevel, minDistance, cv::Mat(), blockSize_harris, useHarrisDetector, k_harris);
                if (!p0.empty()) {
                    std::cout << "Frame " << frame_count << ": Redetected " << p0.size() << " features." << std::endl;
                }
            }
        } else {
            std::cout << "Frame " << frame_count << ": No features to track." << std::endl;
            cv::goodFeaturesToTrack(current_gray, p0, maxCorners, qualityLevel, minDistance, cv::Mat(), blockSize_harris, useHarrisDetector, k_harris);
            if (!p0.empty()) {
                std::cout << "Frame " << frame_count << ": Redetected " << p0.size() << " features." << std::endl;
            }
            prev_gray = current_gray.clone();
        }

        // --- 5. Visualization (Removed for headless operation) ---
        // if (!frame_hd_rgb.empty()) {
        //     cv::imshow("OVX1 Tracking (OpenCV Implementation)", frame_hd_rgb);
        //     if (cv::waitKey(30) == 27) {
        //         std::cout << "User interrupted." << std::endl;
        //         break;
        //     }
        // }

        // Optional: Save processed frames to disk
        // if (!frame_hd_rgb.empty()) {
        //    std::string output_filename = output_dir + "/frame_" + std::to_string(frame_count) + ".png";
        //    if (!cv::imwrite(output_filename, frame_hd_rgb)) {
        //        std::cerr << "Error: Could not save frame " << output_filename << std::endl;
        //    }
        // }
    }

    std::cout << "Processing finished." << std::endl;
    cap.release();
    // cv::destroyAllWindows(); // Not needed if not displaying windows
    return 0;
}