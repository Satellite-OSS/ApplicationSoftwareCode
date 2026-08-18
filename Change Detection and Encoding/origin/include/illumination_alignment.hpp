#ifndef ILLUMINATION_ALIGNMENT_HPP
#define ILLUMINATION_ALIGNMENT_HPP

#include <opencv2/opencv.hpp>

cv::Mat alignIllumination(const cv::Mat& currentImage, const cv::Mat& referenceImage, const cv::Mat& currentCloudMask);

#endif // ILLUMINATION_ALIGNMENT_HPP