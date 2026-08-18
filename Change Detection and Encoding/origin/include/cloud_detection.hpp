#ifndef CLOUD_DETECTION_HPP
#define CLOUD_DETECTION_HPP

#include <opencv2/opencv.hpp>


cv::Mat detectCloudsSimple(const cv::Mat& image);
cv::Mat applyCloudMask(const cv::Mat& image, const cv::Mat& cloudMask); // (可选)

#endif // CLOUD_DETECTION_HPP