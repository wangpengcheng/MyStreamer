set(LIB_SRC
    base_error.cpp
    base_json_parser.cpp
    base_manual_reset_event.cpp
    base_obj_configuration_serializer.cpp
    base_ring_buffer.cpp
    base_str_tools.cpp
    base_tool.cpp
)
include_directories(${PROJECT_SOURCE_DIR}/base)

add_library(stream_base SHARED ${LIB_SRC})

target_link_libraries(stream_base pthread jpeg)

set_target_properties(stream_base PROPERTIES OUTPUT_NAME "stream_base")

add_subdirectory(test)