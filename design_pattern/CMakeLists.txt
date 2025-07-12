aux_source_directory(. SRC_LIST)
# 生成可执行文件
add_executable(design_test ${SRC_LIST})

target_include_directories(design_test
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(design_test
    PRIVATE
    absl::strings
    libcurl
)

# 设置可执行文件输出目录
set_target_properties(design_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)