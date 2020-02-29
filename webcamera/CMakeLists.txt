# 摄像头web基本操作
# 查找boost库
# 查找boost库
find_package(Boost REQUIRED)
find_library(BOOSTTEST_LIBRARY NAMES boost_unit_test_framework)
find_library(BOOSTPO_LIBRARY NAMES boost_program_options)
find_library(BOOSTSYSTEM_LIBRARY NAMES boost_system)


set(LIB_SRC
    video_listener.cpp
    video_source_to_webdata.cpp
    video_to_web.cpp
    web_request_handler.cpp
)
include_directories(${PROJECT_SOURCE_DIR}/base)
include_directories(${PROJECT_SOURCE_DIR}/network/net_base)
include_directories(${PROJECT_SOURCE_DIR}/network/net)
include_directories(${PROJECT_SOURCE_DIR}/network/http)
include_directories(${Boost_INCLUDE_DIRS}/webcamera)
include_directories(${Boost_INCLUDE_DIRS}/imgproc)


add_library(stream_webcamera SHARED ${LIB_SRC})

target_link_libraries(stream_webcamera 
    stream_base
    stream_camera
    stream_imgproc
    stream_network
    pthread
    ${Boost_LIBRARIES}
)

set_target_properties(stream_webcamera PROPERTIES OUTPUT_NAME "stream_network")

