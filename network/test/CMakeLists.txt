# 查找boost库



include_directories(${PROJECT_SOURCE_DIR}/base)
include_directories(${PROJECT_SOURCE_DIR}/network/net_base)
include_directories(${PROJECT_SOURCE_DIR}/network/net)
include_directories(${PROJECT_SOURCE_DIR}/network/http)


add_executable(http_server_test http_server_test.cpp)

target_link_libraries(http_server_test 
    pthread 
    stream_network
)
