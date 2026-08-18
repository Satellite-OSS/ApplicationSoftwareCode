#include "change_detection.hpp"
#include <iostream> // For std::cout


std::vector<cv::Rect> detectChanges(const cv::Mat& alignedCurrentImage, const cv::Mat& referenceImage, double threshold, int tileSize) {
    std::vector<cv::Rect> changedTiles;
    if (alignedCurrentImage.empty() || referenceImage.empty()) {
        std::cerr << "DetectChanges: Input image empty." << std::endl;
        return changedTiles;
    }

    cv::Mat currentProc, refProc;
    
    if (alignedCurrentImage.size() != referenceImage.size()) {
        std::cout << "DetectChanges: Resizing current image to match reference image size for change detection." << std::endl;
        cv::resize(alignedCurrentImage, currentProc, referenceImage.size(), 0, 0, cv::INTER_LINEAR);
    } else {
        currentProc = alignedCurrentImage.clone();
    }
    refProc = referenceImage.clone();


    if (currentProc.type() != refProc.type()) {
        std::cout << "DetectChanges: Converting current image type to match reference." << std::endl;
        currentProc.convertTo(currentProc, refProc.type());
    }
    if (currentProc.channels() != refProc.channels()) {
        std::cerr << "DetectChanges: Image channels mismatch. Will attempt to convert to grayscale." << std::endl;
        if(currentProc.channels() > 1) cv::cvtColor(currentProc, currentProc, cv::COLOR_BGR2GRAY);
        if(refProc.channels() > 1) cv::cvtColor(refProc, refProc, cv::COLOR_BGR2GRAY);

        if (currentProc.channels() != refProc.channels()){ // 再次检查
             std::cerr << "DetectChanges: Failed to reconcile channel numbers. Aborting change detection." << std::endl;
            return changedTiles;
        }
    }


    for (int y = 0; y < currentProc.rows; y += tileSize) {
        for (int x = 0; x < currentProc.cols; x += tileSize) {
            int currentTileWidth = std::min(tileSize, currentProc.cols - x);
            int currentTileHeight = std::min(tileSize, currentProc.rows - y);

            if (currentTileWidth <= 0 || currentTileHeight <=0) continue;

            cv::Rect tileRect(x, y, currentTileWidth, currentTileHeight);
            cv::Mat currentTile = currentProc(tileRect);
            cv::Mat refTile = refProc(tileRect);

            cv::Mat diffTile;
            cv::absdiff(currentTile, refTile, diffTile);

            
            if (diffTile.channels() > 1) {
                cv::cvtColor(diffTile, diffTile, cv::COLOR_BGR2GRAY); 
            }
            
            if (diffTile.type() != CV_8U && diffTile.type() != CV_16U && diffTile.type() != CV_32F) {
                 diffTile.convertTo(diffTile, CV_32F); 
            }


            cv::Scalar meanDiffScalar = cv::mean(diffTile);
            double avgDiffVal = meanDiffScalar[0]; 

            if (avgDiffVal > threshold) {
                changedTiles.push_back(tileRect);
            }
        }
    }
    std::cout << "Detected " << changedTiles.size() << " changed tiles." << std::endl;
    return changedTiles;
}