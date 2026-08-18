#include "illumination_alignment.hpp"
#include <iostream>
#include <vector> 
#include <limits> 


cv::Mat alignIllumination(const cv::Mat& currentImage, const cv::Mat& referenceImage, const cv::Mat& currentCloudMask) {
    if (currentImage.empty() || referenceImage.empty()) {
        std::cerr << "AlignIllumination: Input image empty." << std::endl;
        return currentImage.empty() ? cv::Mat() : currentImage.clone();
    }
    if (currentImage.channels() != referenceImage.channels()) {
        std::cerr << "AlignIllumination: Image channels mismatch. Current: "
                  << currentImage.channels() << ", Reference: " << referenceImage.channels() << std::endl;
        return currentImage.clone();
    }

    cv::Mat nonCloudMask;
    if (!currentCloudMask.empty() && currentCloudMask.size() == currentImage.size() && currentCloudMask.type() == CV_8UC1) {
        cv::bitwise_not(currentCloudMask, nonCloudMask); 
    } else {
        // std::cout << "AlignIllumination: No valid cloud mask provided, using whole current image for stats." << std::endl;
        nonCloudMask = cv::Mat(currentImage.size(), CV_8UC1, cv::Scalar(255)); 
    }

    cv::Mat alignedImage = cv::Mat::zeros(currentImage.size(), currentImage.type());
    std::vector<cv::Mat> currentChannelsVec, refChannelsVec, alignedChannelsVec;

    cv::Mat currentImageFloat, referenceImageFloat;
    
    currentImage.convertTo(currentImageFloat, CV_32F);
    referenceImage.convertTo(referenceImageFloat, CV_32F);

    cv::split(currentImageFloat, currentChannelsVec);
    cv::split(referenceImageFloat, refChannelsVec);

    for (size_t c = 0; c < currentChannelsVec.size(); ++c) {
        cv::Scalar currentMean, currentStdDev;
        cv::Scalar refMean, refStdDev;

        
        cv::meanStdDev(currentChannelsVec[c], currentMean, currentStdDev, nonCloudMask);
        
        cv::meanStdDev(refChannelsVec[c], refMean, refStdDev);

        if (currentStdDev[0] < 1e-6 || refStdDev[0] < 1e-6) { // 避免除以零或标准差过小
            // std::cout << "Warning: Channel " << c << " has near zero stddev. Skipping alignment for this channel." << std::endl;
            cv::Mat originalTypeChannel;
            
            if (currentImage.depth() != CV_32F) {
                 std::vector<cv::Mat> originalCurrentChannels;
                 cv::split(currentImage, originalCurrentChannels);
                 originalTypeChannel = originalCurrentChannels[c];
            } else {
                currentChannelsVec[c].convertTo(originalTypeChannel, currentImage.depth());
            }
            alignedChannelsVec.push_back(originalTypeChannel);
            continue;
        }

        cv::Mat alignedChannelFloat = (currentChannelsVec[c] - currentMean[0]) / currentStdDev[0];
        alignedChannelFloat = alignedChannelFloat * refStdDev[0] + refMean[0];

        
        bool needsClipping = true; 
        switch (currentImage.depth()) {
            case CV_32F:
            case CV_64F:
                needsClipping = false;
                break;
            default:
                
                break;
        }

        if (needsClipping) { 
                             
            cv::patchNaNs(alignedChannelFloat, 0); 
        }
        

        cv::Mat alignedChannelSingle;
        alignedChannelFloat.convertTo(alignedChannelSingle, currentImage.depth()); 

        if (needsClipping) {
            double minVal = 0, maxVal = 0;
            
            switch (currentImage.depth()) {
                case CV_8U:  minVal = 0; maxVal = 255; break;
                case CV_16U: minVal = 0; maxVal = 65535; break;
                case CV_16S: minVal = static_cast<double>(std::numeric_limits<short>::min()); maxVal = static_cast<double>(std::numeric_limits<short>::max()); break;
                case CV_32S: minVal = static_cast<double>(std::numeric_limits<int>::min()); maxVal = static_cast<double>(std::numeric_limits<int>::max()); break;
                
                default:
                    // std::cerr << "AlignIllumination: Clipping for unsupported original image depth: " << currentImage.depth() << std::endl;
                    
                    minVal = 0; maxVal = (currentImage.depth() == CV_8U) ? 255 : (currentImage.depth() == CV_16U ? 65535 : 0);
                    break;
            }
            
            alignedChannelSingle = cv::max(alignedChannelSingle, minVal);
            alignedChannelSingle = cv::min(alignedChannelSingle, maxVal);
        }
        alignedChannelsVec.push_back(alignedChannelSingle);
    }

    if (!alignedChannelsVec.empty() && alignedChannelsVec.size() == static_cast<size_t>(currentImage.channels())) {
        cv::merge(alignedChannelsVec, alignedImage);
    } else {
        std::cerr << "AlignIllumination: Failed to process channels or channel count mismatch. Returning original type image." << std::endl;
        currentImage.convertTo(alignedImage, currentImage.type()); 
    }
    
    return alignedImage;
}