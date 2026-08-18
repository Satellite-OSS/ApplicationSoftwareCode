#include "selective_encoder.hpp"
#include <iostream>
#include <cstring> // For strncpy, memset


#if __has_include(<openjpeg-2.5/openjpeg.h>)
    #include <openjpeg-2.5/openjpeg.h>
#elif __has_include(<openjpeg-2.4/openjpeg.h>)
    #include <openjpeg-2.4/openjpeg.h>
#elif __has_include(<openjpeg-2.3/openjpeg.h>)
    #include <openjpeg-2.3/openjpeg.h>
#else
    #include <openjpeg.h> 
#endif



opj_image_t* matToOpjImage(const cv::Mat& cvImage, opj_cparameters_t* params) {
    if (cvImage.empty() || (cvImage.type() != CV_8UC1 && cvImage.type() != CV_8UC3)) {
        std::cerr << "matToOpjImage: Input image is empty or has unsupported type (only CV_8UC1 or CV_8UC3)." << std::endl;
        return NULL;
    }

    int num_components = cvImage.channels();
    OPJ_COLOR_SPACE color_space = OPJ_CLRSPC_UNSPECIFIED;

    if (num_components == 3) color_space = OPJ_CLRSPC_SRGB;
    else if (num_components == 1) color_space = OPJ_CLRSPC_GRAY;

    
    opj_image_cmptparm_t* cmpparams = new (std::nothrow) opj_image_cmptparm_t[num_components];
    if (!cmpparams) {
        std::cerr << "matToOpjImage: Failed to allocate memory for cmpparams." << std::endl;
        return NULL;
    }
    

    for (int i = 0; i < num_components; ++i) {
        
        memset(&cmpparams[i], 0, sizeof(opj_image_cmptparm_t));
        
        cmpparams[i].dx = params->subsampling_dx;
        cmpparams[i].dy = params->subsampling_dy;
        cmpparams[i].w = cvImage.cols;
        cmpparams[i].h = cvImage.rows;
        cmpparams[i].prec = 8;
        cmpparams[i].bpp = 8;
        cmpparams[i].sgnd = 0;
    }

    opj_image_t *image = opj_image_create(num_components, cmpparams, color_space);
    
    delete[] cmpparams;

    if (!image) {
        std::cerr << "matToOpjImage: Failed to create opj_image." << std::endl;
        return NULL;
    }

    image->x0 = params->image_offset_x0;
    image->y0 = params->image_offset_y0;
    image->x1 = params->image_offset_x0 + cvImage.cols;
    image->y1 = params->image_offset_y0 + cvImage.rows;

    std::vector<cv::Mat> img_channels;
    cv::split(cvImage, img_channels);

    for (int comp_idx = 0; comp_idx < num_components; ++comp_idx) {
        int i = 0; 
        for (int y = 0; y < cvImage.rows; ++y) {
            for (int x = 0; x < cvImage.cols; ++x) {
                int current_comp_cv_idx = comp_idx;
                if (num_components == 3) { // BGR to RGB-like for OpenJPEG
                    if (comp_idx == 0) current_comp_cv_idx = 2; // R
                    else if (comp_idx == 1) current_comp_cv_idx = 1; // G
                    else if (comp_idx == 2) current_comp_cv_idx = 0; // B
                }
                
                if (image->comps[comp_idx].data == NULL) {
                     std::cerr << "matToOpjImage: Component " << comp_idx << " data is NULL." << std::endl;
                     opj_image_destroy(image); 
                     return NULL;
                }
                image->comps[comp_idx].data[i] = img_channels[current_comp_cv_idx].at<unsigned char>(y,x);
                i++;
            }
        }
    }
    return image;
}


void selectiveEncode(const cv::Mat& originalImage, const std::vector<cv::Rect>& changedTiles, const std::string& outputPath) {
    if (originalImage.empty()) {
        std::cerr << "SelectiveEncode: Original image is empty." << std::endl;
        return;
    }
    if (originalImage.type() != CV_8UC3 && originalImage.type() != CV_8UC1) {
        std::cerr << "SelectiveEncode: This example only supports CV_8UC3 or CV_8UC1 for encoding." << std::endl;
        return;
    }

    cv::Mat imageToEncode = cv::Mat::zeros(originalImage.size(), originalImage.type());
    if (changedTiles.empty()) {
        std::cout << "SelectiveEncode: No changed tiles. Encoding a black image (or low quality base)." << std::endl;
        
    } else {
        for (const auto& rect : changedTiles) {
            if (rect.x >= 0 && rect.y >= 0 &&
                rect.x + rect.width <= originalImage.cols &&
                rect.y + rect.height <= originalImage.rows &&
                rect.width > 0 && rect.height > 0) { 
                originalImage(rect).copyTo(imageToEncode(rect));
            }
        }
    }

    opj_cparameters_t parameters;
    opj_set_default_encoder_parameters(&parameters);

   
    if (outputPath.length() < OPJ_PATH_LEN) { 
        strncpy(parameters.outfile, outputPath.c_str(), OPJ_PATH_LEN - 1);
        parameters.outfile[OPJ_PATH_LEN - 1] = '\0';
    } else {
        std::cerr << "Error: Output path is too long for parameters.outfile buffer (" << OPJ_PATH_LEN << " chars max)." << std::endl;
        strncpy(parameters.outfile, outputPath.c_str(), OPJ_PATH_LEN - 1);
        parameters.outfile[OPJ_PATH_LEN - 1] = '\0'; 
        std::cerr << "Warning: Output path was truncated to: " << parameters.outfile << std::endl;
    }
    

    parameters.irreversible = 1;
    parameters.subsampling_dx = 1;
    parameters.subsampling_dy = 1;
    parameters.image_offset_x0 = 0;
    parameters.image_offset_y0 = 0;
    

    opj_image_t * opjImage = matToOpjImage(imageToEncode, &parameters);
    if (!opjImage) {
        std::cerr << "SelectiveEncode: Failed to convert cv::Mat to opj_image_t." << std::endl;
        return;
    }

    opj_codec_t* l_codec = opj_create_compress(OPJ_CODEC_J2K);
    if (!l_codec) {
        std::cerr << "SelectiveEncode: Failed to create OpenJPEG codec." << std::endl;
        opj_image_destroy(opjImage);
        return;
    }
    if (!opj_setup_encoder(l_codec, &parameters, opjImage)) {
        std::cerr << "SelectiveEncode: Failed to setup encoder." << std::endl;
        opj_destroy_codec(l_codec);
        opj_image_destroy(opjImage);
        return;
    }

    opj_stream_t* l_stream = opj_stream_create_default_file_stream(parameters.outfile, OPJ_FALSE);
    if (!l_stream) {
        std::cerr << "SelectiveEncode: Failed to open output file stream: " << parameters.outfile << std::endl;
        opj_destroy_codec(l_codec);
        opj_image_destroy(opjImage);
        return;
    }

    bool success = false;
    if (!opj_start_compress(l_codec, opjImage, l_stream)) {
        std::cerr << "SelectiveEncode: opj_start_compress failed." << std::endl;
    } else if (!opj_encode(l_codec, l_stream)) {
        std::cerr << "SelectiveEncode: opj_encode failed." << std::endl;
    } else if (!opj_end_compress(l_codec, l_stream)) {
        std::cerr << "SelectiveEncode: opj_end_compress failed." << std::endl;
    } else {
        success = true;
        std::cout << "Successfully encoded image to " << parameters.outfile << std::endl;
    }

    opj_stream_destroy(l_stream);
    opj_destroy_codec(l_codec);
    opj_image_destroy(opjImage);
}