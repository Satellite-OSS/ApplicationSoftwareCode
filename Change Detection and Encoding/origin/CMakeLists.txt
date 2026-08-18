cmake_minimum_required(VERSION 3.10)
project(EarthPlusSim)

set(CMAKE_CXX_STANDARD 17)


include_directories(include)


find_package(OpenCV REQUIRED)
# include_directories(${OpenCV_INCLUDE_DIRS}) 


find_package(GDAL REQUIRED)
# include_directories(${GDAL_INCLUDE_DIRS}) 


# find_package(PkgConfig REQUIRED)
# pkg_check_modules(OPENJPEG REQUIRED libopenjp2)
# include_directories(${OPENJPEG_INCLUDE_DIRS})
# link_directories(${OPENJPEG_LIBRARY_DIRS})


add_executable(EarthPlusSim
    main.cpp
    src/image_io.cpp
    src/cloud_detection.cpp
    src/illumination_alignment.cpp
    src/change_detection.cpp
    src/selective_encoder.cpp
)

target_link_libraries(EarthPlusSim
    ${OpenCV_LIBS}
    ${GDAL_LIBRARIES} 
    openjp2           
)


# target_link_libraries(EarthPlusSim ${OpenCV_LIBS} ${GDAL_LIBRARIES} ${OPENJPEG_LIBRARIES})