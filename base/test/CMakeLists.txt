set(Test_SRC
   ring_test.cpp
)
include_directories(${PROJECT_SOURCE_DIR}/base)

add_executable(ring_test ring_test.cpp)
target_link_libraries(ring_test pthread)
