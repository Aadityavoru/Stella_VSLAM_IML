cmake_minimum_required(VERSION 3.1)
project(realsense_vslam_project)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(Eigen3 REQUIRED)
find_package(stella_vslam REQUIRED)
find_package(realsense2 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Pangolin REQUIRED)

# Add the executable
add_executable(realsense_vslam realsense_vslam.cpp)

# Link libraries
target_link_libraries(realsense_vslam
    Eigen3::Eigen
    stella_vslam::stella_vslam
    ${realsense2_LIBRARY}
    ${OpenCV_LIBS}
    yaml-cpp::yaml-cpp
    ${Pangolin_LIBRARIES}
)

# Include directories
target_include_directories(realsense_vslam PRIVATE
    ${stella_vslam_INCLUDE_DIRS}
    ${realsense2_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${Pangolin_INCLUDE_DIRS}
)
