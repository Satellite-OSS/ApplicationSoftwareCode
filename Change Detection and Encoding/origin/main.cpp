#include <iostream>
#include <string>
#include <vector>

#include "image_io.hpp"
#include "cloud_detection.hpp"
#include "illumination_alignment.hpp"
#include "change_detection.hpp"
#include "selective_encoder.hpp"

#include <opencv2/highgui.hpp> // For imshow, waitKey (optional for debugging)


int main() {
    std::string refImagePath = "../images/reference.tif";
std::string currentImagePath = "../images/current_capture.tif";

    std::cout << "--- Earth+ Simulator ---" << std::endl;

    
    std::cout << "\nStep 0: Loading Images" << std::endl;
    cv::Mat refImage = loadImageGDAL(refImagePath);
    cv::Mat currentImage = loadImageGDAL(currentImagePath);

    if (refImage.empty() || currentImage.empty()) {
        std::cerr << "Failed to load one or both images. Exiting." << std::endl;
        return -1;
    }
    
    if (refImage.channels() != currentImage.channels() || refImage.depth() != currentImage.depth()) {
        std::cerr << "Warning: Reference and current image have different channel counts or depth." << std::endl;
        
        // For simplicity, we'll proceed but this should be handled robustly
    }
    


    
    std::cout << "\nStep 1: Cloud Removal (Simplified)" << std::endl;
    cv::Mat currentCloudMask = detectCloudsSimple(currentImage);
    if (currentCloudMask.empty() && !currentImage.empty()) { 
        std::cout << "Cloud detection failed, assuming no clouds for current image." << std::endl;
        currentCloudMask = cv::Mat::zeros(currentImage.size(), CV_8UC1);
    }


    
    if(!currentCloudMask.empty()){
        double cloudArea = cv::sum(currentCloudMask)[0] / 255.0; 
        double totalArea = static_cast<double>(currentCloudMask.rows * currentCloudMask.cols);
        double cloudPercentage = (totalArea > 0) ? (cloudArea / totalArea) : 0.0;

        std::cout << "Cloud percentage in current image: " << cloudPercentage * 100 << "%" << std::endl;
        if (cloudPercentage > 0.5) {
            std::cout << "More than 50% cloudy, discarding current image (simulation)." << std::endl;
            
        }
    }


    
    std::cout << "\nStep 2: Illumination Alignment" << std::endl;
    cv::Mat alignedCurrentImage = alignIllumination(currentImage, refImage, currentCloudMask);


    
    std::cout << "\nStep 3: Change Detection" << std::endl;
    int tileSize = 64;
    double changeThreshold = 30.0; 
                                   
    if (alignedCurrentImage.depth() == CV_16U) changeThreshold = 700; 
    else if (alignedCurrentImage.depth() == CV_32F) changeThreshold = 0.15;


    std::vector<cv::Rect> changedTiles = detectChanges(alignedCurrentImage, refImage, changeThreshold, tileSize);


    
    #ifdef WITH_OPENCV_HIGHGUI 
    if (!changedTiles.empty() && !alignedCurrentImage.empty()) {
        cv::Mat currentImageWithChanges = alignedCurrentImage.clone();
         
        if(currentImageWithChanges.channels() == 1) {
            cv::cvtColor(currentImageWithChanges, currentImageWithChanges, cv::COLOR_GRAY2BGR);
        }
        for (const auto& rect : changedTiles) {
            cv::rectangle(currentImageWithChanges, rect, cv::Scalar(0, 0, 255), 2); 
        }
        cv::imshow("Detected Changes on Aligned Image", currentImageWithChanges);
        cv::waitKey(0);
        cv::destroyWindow("Detected Changes on Aligned Image");
    }
    #endif

    
    std::cout << "\nStep 4: Selective Encoding" << std::endl;
    selectiveEncode(currentImage, changedTiles, "output_sim_earthplus.j2k");

    std::cout << "\n--- Simulation Finished ---" << std::endl;
    return 0;
}