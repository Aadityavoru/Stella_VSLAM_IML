#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <stella_vslam/system.h>
#include <stella_vslam/config.h>

int main(int argc, char* argv[]) {
    // Initialize RealSense pipeline for both color and depth
    rs2::pipeline pipe;
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_RGB8, 30);  // RGB stream
    cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);   // Depth stream
    pipe.start(cfg);

    // Initialize stella_vslam system (config file should define RGB-D mode)
    auto slam_cfg = std::make_shared<stella_vslam::config>("/home/perception/lib/stella_vslam_examples/build/realsense_rgbd.yaml");
    stella_vslam::system SLAM(slam_cfg, "/home/perception/lib/stella_vslam_examples/build/orb_vocab.fbow");

    SLAM.startup();

    while (true) {
        // Wait for the next set of frames from the RealSense pipeline
        rs2::frameset frames = pipe.wait_for_frames();

        // Get color and depth frames
        rs2::frame color_frame = frames.get_color_frame();
        rs2::depth_frame depth_frame = frames.get_depth_frame();

        // Convert RealSense color frame to OpenCV Mat
        cv::Mat image(cv::Size(640, 480), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

        // Convert RealSense depth frame to OpenCV Mat (Depth data is 16-bit unsigned int)
        cv::Mat depth_image(cv::Size(640, 480), CV_16U, (void*)depth_frame.get_data(), cv::Mat::AUTO_STEP);

        // Feed RGB-D frame to SLAM system
        double timestamp = color_frame.get_timestamp() / 1000.0;  // Convert to seconds
        SLAM.feed_RGBD_frame(image, depth_image, timestamp);

        // Normalize depth image for visualization (8-bit conversion)
        cv::Mat depth_image_vis;
        depth_image.convertTo(depth_image_vis, CV_8U, 255.0 / 10000.0); // Scale for display (adjust max depth)

        // Display the current color frame and depth image
        cv::imshow("Color Frame", image);
        cv::imshow("Depth Frame", depth_image_vis);

        // Exit condition: Press 'Esc' to exit
        int key = cv::waitKey(1);
        if (key == 27) break;
    }

    // Shutdown the SLAM system
    SLAM.shutdown();

    return 0;
}
