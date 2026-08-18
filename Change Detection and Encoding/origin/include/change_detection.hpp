#ifndef CHANGE_DETECTION_HPP
#define CHANGE_DETECTION_HPP

#include <opencv2/opencv.hpp>
#include <vector>

std::vector<cv::Rect> detectChanges(const cv::Mat& alignedCurrentImage, const cv::Mat& referenceImage, double threshold, int tileSize);

#endif // CHANGE_DETECTION_HPP