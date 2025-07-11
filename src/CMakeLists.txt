
# 自动收集当前目录所有.cpp文件  
file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*)  
 

# so 动态库放在根目录lib文件夹下
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 生成动态库（可通过SHARED/STATIC切换库类型）  
add_library(mymuduo SHARED ${SRC_FILES})  
 
# 暴露头文件路径（供外部target引用）  
target_include_directories(mymuduo 
    PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(mymuduo
    PRIVATE
    pthread
    absl::strings
)