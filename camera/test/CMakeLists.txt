# Find OpenCV
find_package(OpenCV REQUIRED)
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
# Add OpenCV headers location to your include paths
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/camera/V4L2/test)

add_executable(V4L2CameraTest v4l2_camrea_test.cpp)

# 添加目标链接
# 链接OpenCV
target_link_libraries(V4L2CameraTest 
    stream_base 
    stream_imgproc 
    stream_camera
    ${OpenCV_LIBS} 
)

