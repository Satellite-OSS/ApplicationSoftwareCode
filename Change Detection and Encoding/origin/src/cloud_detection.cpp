#include "cloud_detection.hpp"
#include <iostream> // For std::cout


cv::Mat detectCloudsSimple(const cv::Mat& image) {
    if (image.empty()) {
        std::cerr << "detectCloudsSimple: Input image is empty." << std::endl;
        return cv::Mat();
    }

    cv::Mat grayImage;
    if (image.channels() == 3 || image.channels() == 4) { // BGR or BGRA
        
        std::vector<cv::Mat> channels;
        cv::split(image, channels);
        
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY); 
    } else if (image.channels() == 1) {
        grayImage = image.clone();
    } else {
        std::cerr << "detectCloudsSimple: Unsupported number of channels: " << image.channels() << std::endl;
        return cv::Mat();
    }

    cv::Mat cloudMask;
    
    double highBrightnessThreshold = 200;
    if (grayImage.depth() == CV_8U) highBrightnessThreshold = 200;
    else if (grayImage.depth() == CV_16U) highBrightnessThreshold = 50000; 
    else if (grayImage.depth() == CV_32F) highBrightnessThreshold = 0.8;   
   

    cv::threshold(grayImage, cloudMask, highBrightnessThreshold, 255, cv::THRESH_BINARY);
    std::cout << "Generated simple cloud mask." << std::endl;
    return cloudMask;
}

cv::Mat applyCloudMask(const cv::Mat& image, const cv::Mat& cloudMask) {
    if (image.empty() || cloudMask.empty() || image.size() != cloudMask.size() || cloudMask.type() != CV_8UC1) {
        std::cerr << "applyCloudMask: Invalid input." << std::endl;
        return image.clone();
    }
    cv::Mat result = image.clone();
    result.setTo(cv::Scalar::all(0), cloudMask); 
    return result;
}