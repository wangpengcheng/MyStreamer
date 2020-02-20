set(LIB_SRC
   image_drawer.cpp
   image.cpp
)
include_directories(${PROJECT_SOURCE_DIR}/imgproc)
add_library(stream_imgproc SHARED ${LIB_SRC})
target_link_libraries(stream_imgproc pthread jpeg stream_base)

set_target_properties(stream_imgproc PROPERTIES OUTPUT_NAME "stream_imgproc")
