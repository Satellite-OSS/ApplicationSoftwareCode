#include "image_io.hpp"
#include <iostream>


static GDALInitializer gdal_initializer_object;

GDALInitializer::GDALInitializer() {
    GDALAllRegister();
    std::cout << "GDAL drivers registered (from image_io.cpp)." << std::endl;
}

cv::Mat loadImageGDAL(const std::string& filePath) {
    GDALDataset *poDataset;
    poDataset = (GDALDataset *) GDALOpen(filePath.c_str(), GA_ReadOnly);
    if (poDataset == NULL) {
        std::cerr << "Error: GDAL Could not open dataset: " << filePath << std::endl;
        
        cv::Mat img = cv::imread(filePath, cv::IMREAD_LOAD_GDAL | cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);
        if (img.empty()) {
             std::cerr << "Error: OpenCV also failed to load image: " << filePath << std::endl;
        } else {
            std::cout << "Image loaded with OpenCV fallback: " << filePath << std::endl;
        }
        return img;
    }

    int width = poDataset->GetRasterXSize();
    int height = poDataset->GetRasterYSize();
    int numBands = poDataset->GetRasterCount();
    std::cout << "Image: " << filePath << " | Size: " << width << "x" << height << " | Bands: " << numBands << std::endl;

    GDALDataType eDataType = poDataset->GetRasterBand(1)->GetRasterDataType();
    int cvMatType = -1;
    if (eDataType == GDT_Byte) cvMatType = CV_8U;
    else if (eDataType == GDT_UInt16) cvMatType = CV_16U;
    else if (eDataType == GDT_Int16) cvMatType = CV_16S;
    else if (eDataType == GDT_Float32) cvMatType = CV_32F;
    

    if (cvMatType == -1) {
        std::cerr << "Error: Unsupported GDAL data type for OpenCV conversion." << std::endl;
        GDALClose(poDataset);
        return cv::Mat();
    }

    cv::Mat image;
    std::vector<cv::Mat> bandsVec;

    
    for (int i = 1; i <= numBands; ++i) {
        GDALRasterBand *poBand = poDataset->GetRasterBand(i);
        
        cv::Mat bandMat(height, width, cvMatType); 
        CPLErr err = poBand->RasterIO(GF_Read, 0, 0, width, height,
                                      bandMat.data, width, height, eDataType,
                                      0, 0);
        if (err != CE_None) {
            std::cerr << "Error reading band " << i << " from " << filePath << std::endl;
            GDALClose(poDataset);
            return cv::Mat();
        }
        bandsVec.push_back(bandMat);
    }

    if (bandsVec.empty()) {
        GDALClose(poDataset);
        return cv::Mat();
    }

    if (bandsVec.size() > 1) {
        cv::merge(bandsVec, image); 
    } else {
        image = bandsVec[0]; 
    }

    GDALClose(poDataset);
    return image;
}