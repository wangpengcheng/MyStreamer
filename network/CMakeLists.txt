set(LIB_SRC
  
)
include_directories(${PROJECT_SOURCE_DIR}/base)
include_directories(${PROJECT_SOURCE_DIR}/network)
add_library(stream_network SHARED ${LIB_SRC})

target_link_libraries(stream_network stream_base pthread)

set_target_properties(stream_network PROPERTIES OUTPUT_NAME "stream_network")
