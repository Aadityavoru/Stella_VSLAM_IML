#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <stella_vslam/system.h>
#include <stella_vslam/config.h>
#include <stella_vslam/viewer/viewer.h>
#include <thread>

int main(int argc, char* argv[]) {
    // Initialize RealSense pipeline
    rs2::pipeline pipe;
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_RGB8, 30);
    pipe.start(cfg);

    // Initialize stella_vslam
    auto slam_cfg = std::make_shared<stella_vslam::config>("/home/perception/lib/stella_vslam_examples/build/realsense.yaml");
    stella_vslam::system SLAM(slam_cfg, "/home/perception/lib/stella_vslam_examples/build/orb_vocab.fbow");

    // Create and run the viewer in a separate thread
    auto viewer = std::make_shared<stella_vslam::viewer::viewer>(
        slam_cfg, SLAM.get_frame_publisher(), SLAM.get_map_publisher());
    std::thread viewer_thread([&]() { viewer->run(); });

    SLAM.startup();

    while (true) {
        rs2::frameset frames = pipe.wait_for_frames();
        rs2::frame color_frame = frames.get_color_frame();

        // Convert RealSense frame to OpenCV Mat
        cv::Mat image(cv::Size(640, 480), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

        // Feed frame to SLAM system
        double timestamp = color_frame.get_timestamp() / 1000.0; // Convert to seconds
        SLAM.feed_monocular_frame(image, timestamp);

        // Exit condition
        int key = cv::waitKey(1);
        if (key == 27) break; // Press 'Esc' to exit
    }

    SLAM.shutdown();
    viewer->request_terminate();
    viewer_thread.join();

    return 0;
}
