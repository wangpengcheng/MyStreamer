set(LIB_SRC
   video_frame_decorator.cpp
   ./V4L2/v4l2_camera.cpp
   ./V4L2/v4l2_camera_data.cpp
   ./V4L2/v4l2_camera_config.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/camera)
include_directories(${PROJECT_SOURCE_DIR}/camera/V4L2)

add_library(stream_camera SHARED ${LIB_SRC})

target_link_libraries(stream_camera stream_base stream_imgproc pthread jpeg)
set_target_properties(stream_camera PROPERTIES OUTPUT_NAME "stream_camera")

add_subdirectory(test)
