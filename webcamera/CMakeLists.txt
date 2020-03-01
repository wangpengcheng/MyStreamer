# 摄像头web基本操作


set(LIB_SRC
    video_listener.cpp
    video_source_to_webdata.cpp
    video_source_to_web.cpp
    web_request_handler.cpp
    web_camera_server.cpp
    file_request_handler.cpp
)
include_directories(${PROJECT_SOURCE_DIR}/base)
include_directories(${PROJECT_SOURCE_DIR}/imgproc)
include_directories(${PROJECT_SOURCE_DIR}/network/net_base)
include_directories(${PROJECT_SOURCE_DIR}/network/net)
include_directories(${PROJECT_SOURCE_DIR}/network/http)
include_directories(${PROJECT_SOURCE_DIR}/webcamera)


add_library(stream_webcamera SHARED ${LIB_SRC})

target_link_libraries(stream_webcamera 
    stream_base
    stream_camera
    stream_imgproc
    stream_network
    pthread
)

set_target_properties(stream_webcamera PROPERTIES OUTPUT_NAME "stream_webcamera")

