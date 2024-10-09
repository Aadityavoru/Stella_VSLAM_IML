// int stereo_tracking(const std::shared_ptr<stella_vslam::system>& slam,
//                     const std::shared_ptr<stella_vslam::config>& cfg,
//                     const unsigned int cam_num,
//                     const std::string& mask_img_path,
//                     const float scale,
//                     const std::string& map_db_path,
//                     const std::string& viewer_string) {
//     const cv::Mat mask = mask_img_path.empty() ? cv::Mat{} : cv::imread(mask_img_path, cv::IMREAD_GRAYSCALE);

//     // create a viewer object
//     // and pass the frame_publisher and the map_publisher
// #ifdef HAVE_PANGOLIN_VIEWER
//     std::shared_ptr<pangolin_viewer::viewer> viewer;
//     if (viewer_string == "pangolin_viewer") {
//         viewer = std::make_shared<pangolin_viewer::viewer>(
//             stella_vslam::util::yaml_optional_ref(cfg->yaml_node_, "PangolinViewer"),
//             slam,
//             slam->get_frame_publisher(),
//             slam->get_map_publisher());
//     }
// #endif
// #ifdef HAVE_IRIDESCENCE_VIEWER
//     std::shared_ptr<iridescence_viewer::viewer> iridescence_viewer;
//     std::mutex mtx_pause;
//     bool is_paused = false;
//     std::mutex mtx_terminate;
//     bool terminate_is_requested = false;
//     std::mutex mtx_step;
//     unsigned int step_count = 0;
//     if (viewer_string == "iridescence_viewer") {
//         iridescence_viewer = std::make_shared<iridescence_viewer::viewer>(
//             stella_vslam::util::yaml_optional_ref(cfg->yaml_node_, "IridescenceViewer"),
//             slam->get_frame_publisher(),
//             slam->get_map_publisher());
//         iridescence_viewer->add_checkbox("Pause", [&is_paused, &mtx_pause](bool check) {
//             std::lock_guard<std::mutex> lock(mtx_pause);
//             is_paused = check;
//         });
//         iridescence_viewer->add_button("Step", [&step_count, &mtx_step] {
//             std::lock_guard<std::mutex> lock2(mtx_step);
//             step_count++;
//         });
//         iridescence_viewer->add_button("Reset", [&is_paused, &mtx_pause, &slam] {
//             slam->request_reset();
//         });
//         iridescence_viewer->add_button("Save and exit", [&is_paused, &mtx_pause, &terminate_is_requested, &mtx_terminate, &slam, &iridescence_viewer] {
//             std::lock_guard<std::mutex> lock1(mtx_pause);
//             is_paused = false;
//             std::lock_guard<std::mutex> lock2(mtx_terminate);
//             terminate_is_requested = true;
//             iridescence_viewer->request_terminate();
//         });
//         iridescence_viewer->add_close_callback([&is_paused, &mtx_pause, &terminate_is_requested, &mtx_terminate] {
//             std::lock_guard<std::mutex> lock1(mtx_pause);
//             is_paused = false;
//             std::lock_guard<std::mutex> lock2(mtx_terminate);
//             terminate_is_requested = true;
//         });
//     }
// #endif
// #ifdef HAVE_SOCKET_PUBLISHER
//     std::shared_ptr<socket_publisher::publisher> publisher;
//     if (viewer_string == "socket_publisher") {
//         publisher = std::make_shared<socket_publisher::publisher>(
//             stella_vslam::util::yaml_optional_ref(cfg->yaml_node_, "SocketPublisher"),
//             slam,
//             slam->get_frame_publisher(),
//             slam->get_map_publisher());
//     }
// #endif

//     cv::VideoCapture videos[2];
//     for (int i = 0; i < 2; i++) {

//         std::cout << "I am connecting ..." << cam_num +(i*2) << endl;
//         videos[i] = cv::VideoCapture(cam_num + (i*2));
//         if (!videos[i].isOpened()) {
//             spdlog::critical("cannot open a camera {}", cam_num + i);
//             slam->shutdown();
//             return EXIT_FAILURE;
//         }
//     }

//     const stella_vslam::util::stereo_rectifier rectifier(cfg, slam->get_camera());

//     cv::Mat frames[2];
//     cv::Mat frames_rectified[2];
//     std::vector<double> track_times;
//     unsigned int num_frame = 0;

//     bool is_not_end = true;
//     // run the slam in another thread
//     std::thread thread([&]() {
//         while (is_not_end) {
// #ifdef HAVE_IRIDESCENCE_VIEWER
//             while (true) {
//                 {
//                     std::lock_guard<std::mutex> lock(mtx_pause);
//                     if (!is_paused) {
//                         break;
//                     }
//                 }
//                 {
//                     std::lock_guard<std::mutex> lock(mtx_step);
//                     if (step_count > 0) {
//                         step_count--;
//                         break;
//                     }
//                 }
//                 std::this_thread::sleep_for(std::chrono::microseconds(5000));
//             }
// #endif

// #ifdef HAVE_IRIDESCENCE_VIEWER
//             // check if the termination of slam system is requested or not
//             {
//                 std::lock_guard<std::mutex> lock(mtx_terminate);
//                 if (terminate_is_requested) {
//                     break;
//                 }
//             }
// #else
//             // check if the termination of slam system is requested or not
//             if (slam->terminate_is_requested()) {
//                 break;
//             }
// #endif

//             is_not_end = videos[0].read(frames[0]) && videos[1].read(frames[1]);
//             if (frames[0].empty() || frames[1].empty()) {
//                 continue;
//             }
//             for (int i = 0; i < 2; i++) {
//                 if (scale != 1.0) {
//                     cv::resize(frames[i], frames[i], cv::Size(), scale, scale, cv::INTER_LINEAR);
//                 }
//             }
//             rectifier.rectify(frames[0], frames[1], frames_rectified[0], frames_rectified[1]);

//             const auto tp_1 = std::chrono::steady_clock::now();

//             // input the current frame and estimate the camera pose
//             std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
//             double timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch()).count();
//             slam->feed_stereo_frame(frames_rectified[0], frames_rectified[1], timestamp, mask);

//             const auto tp_2 = std::chrono::steady_clock::now();

//             const auto track_time = std::chrono::duration_cast<std::chrono::duration<double>>(tp_2 - tp_1).count();
//             track_times.push_back(track_time);

//             ++num_frame;
//         }

//         // wait until the loop BA is finished
//         while (slam->loop_BA_is_running()) {
//             std::this_thread::sleep_for(std::chrono::microseconds(5000));
//         }
//     });

//     // run the viewer in the current thread
//     if (viewer_string == "pangolin_viewer") {
// #ifdef HAVE_PANGOLIN_VIEWER
//         viewer->run();
// #endif
//     }
//     if (viewer_string == "iridescence_viewer") {
// #ifdef HAVE_IRIDESCENCE_VIEWER
//         iridescence_viewer->run();
// #endif
//     }
//     if (viewer_string == "socket_publisher") {
// #ifdef HAVE_SOCKET_PUBLISHER
//         publisher->run();
// #endif
//     }

//     thread.join();

//     // shutdown the slam process
//     slam->shutdown();

//     std::sort(track_times.begin(), track_times.end());
//     const auto total_track_time = std::accumulate(track_times.begin(), track_times.end(), 0.0);
//     std::cout << "median tracking time: " << track_times.at(track_times.size() / 2) << "[s]" << std::endl;
//     std::cout << "mean tracking time: " << total_track_time / track_times.size() << "[s]" << std::endl;

//     if (!map_db_path.empty()) {
//         if (!slam->save_map_database(map_db_path)) {
//             return EXIT_FAILURE;
//         }
//     }

//     return EXIT_SUCCESS;
// }
