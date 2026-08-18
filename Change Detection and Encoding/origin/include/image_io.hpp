#ifndef IMAGE_IO_HPP
#define IMAGE_IO_HPP

#include <string>
#include <opencv2/opencv.hpp>
#include "gdal/gdal_priv.h"


class GDALInitializer {
public:
    GDALInitializer();
};

cv::Mat loadImageGDAL(const std::string& filePath);

#endif // IMAGE_IO_HPP