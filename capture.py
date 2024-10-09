import pyzed.sl as sl
import cv2
import time
import numpy as np
import subprocess

# Initialize ZED camera
zed = sl.Camera()

# Create InitParameters object to define the initialization parameters
init_params = sl.InitParameters()
init_params.camera_resolution = sl.RESOLUTION.HD720  # Use HD720 video mode
init_params.camera_fps = 30  # Set FPS to 30

# Open the camera
err = zed.open(init_params)
if err != sl.ERROR_CODE.SUCCESS:
    print(f"Failed to open ZED camera: {err}")
    exit(1)

# Create Mat objects to store images
left_image = sl.Mat()
right_image = sl.Mat()

# Set up the video writer to store both camera frames side by side using XVID codec
fourcc = cv2.VideoWriter_fourcc(*'XVID')  # Codec for XVID
output_size = (1280 * 2, 720)  # Combine two 1280x720 frames side by side
video_out = cv2.VideoWriter('zed_combined.avi', fourcc, 30.0, output_size)

# Capture video for 20 seconds
start_time = time.time()
while time.time() - start_time < 40:
    if zed.grab() == sl.ERROR_CODE.SUCCESS:
        # Retrieve left and right images
        zed.retrieve_image(left_image, sl.VIEW.LEFT)
        zed.retrieve_image(right_image, sl.VIEW.RIGHT)
        
        # Convert ZED SDK format to OpenCV BGR format
        left_frame = left_image.get_data()[:, :, :3]  # Strip alpha channel
        right_frame = right_image.get_data()[:, :, :3]
        
        # Ensure the format is uint8 (for OpenCV compatibility)
        left_frame = left_frame.astype(np.uint8)
        right_frame = right_frame.astype(np.uint8)
        
        # Combine left and right frames side by side
        combined_frame = np.hstack((left_frame, right_frame))
        
        # Write the combined frame to the video file
        video_out.write(combined_frame)
        
        # Optional: Display the combined frame in a window
        cv2.imshow('ZED Combined', combined_frame)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

# Release everything after the recording is done
video_out.release()
zed.close()
cv2.destroyAllWindows()

print("Video capture complete and saved as zed_combined.avi")
