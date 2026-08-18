#ifndef SELECTIVE_ENCODER_HPP
#define SELECTIVE_ENCODER_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

void selectiveEncode(const cv::Mat& originalImage, const std::vector<cv::Rect>& changedTiles, const std::string& outputPath);

#endif // SELECTIVE_ENCODER_HPP