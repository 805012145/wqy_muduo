# 生成可执行文件  
add_executable(my_test test.cpp)

target_include_directories(my_test
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(my_test
    PRIVATE
    absl::strings
    libcurl
)

# 设置可执行文件输出目录
set_target_properties(my_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)