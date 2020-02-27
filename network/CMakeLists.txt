# 查找boost库
find_package(Boost REQUIRED)
find_library(BOOSTTEST_LIBRARY NAMES boost_unit_test_framework)
find_library(BOOSTPO_LIBRARY NAMES boost_program_options)
find_library(BOOSTSYSTEM_LIBRARY NAMES boost_system)



set(LIB_SRC
    ./net_base/async_logging.cpp
    ./net_base/base_condition.cpp
    ./net_base/base_date.cpp
    ./net_base/base_exception.cpp
    ./net_base/base_thread.cpp
    ./net_base/count_downlatch.cpp
    ./net_base/current_thread.cpp
    ./net_base/file_util.cpp
    ./net_base/logging.cpp
    ./net_base/log_file.cpp
    ./net_base/log_stream.cpp
    ./net_base/process_info.cpp
    ./net_base/thread_pool.cpp
    ./net_base/time_stamp.cpp
    ./net_base/time_zone.cpp
    ./net/net_buffer.cpp
    ./net/net_channel.cpp
    ./net/net_connector.cpp
    ./net/net_event_loop.cpp
    ./net/net_inet_address.cpp
    ./net/net_poller.cpp
    ./net/net_socket.cpp
    ./net/net_sockets_ops.cpp
    ./net/net_tcp_connection.cpp
    ./net/net_timer.cpp
    ./net/net_timer_queue.cpp
)
include_directories(${PROJECT_SOURCE_DIR}/base)
include_directories(${PROJECT_SOURCE_DIR}/network/net_base)
include_directories(${PROJECT_SOURCE_DIR}/network/net)
include_directories(${Boost_INCLUDE_DIRS})

add_library(stream_network SHARED ${LIB_SRC})

target_link_libraries(stream_network stream_base pthread ${Boost_LIBRARIES})

set_target_properties(stream_network PROPERTIES OUTPUT_NAME "stream_network")
