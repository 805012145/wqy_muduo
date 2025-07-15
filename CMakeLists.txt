cmake_minimum_required(VERSION 3.16)
project(mymuduo)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC -Wno-implicit-function-declaration -Wno-int-conversion -D_POSIX_C_SOURCE=200809L")

# c++14 语言标准
set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_C_STANDARD 14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 使用系统安装的 OpenSSL
find_package(OpenSSL REQUIRED)


include(FetchContent)
FetchContent_Declare(
    abseil
    GIT_REPOSITORY git@github.com:abseil/abseil-cpp.git
    GIT_TAG 20230125.3
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/abseil
)

FetchContent_Declare(
    curl
    GIT_REPOSITORY git@github.com:/curl/curl.git
    GIT_TAG        curl-8_8_0  # 使用最新稳定版本
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/curl
    # 禁用不需要的功能以简化构建
    CMAKE_ARGS
        -DBUILD_CURL_EXE=OFF
        -DBUILD_TESTING=OFF
        -DBUILD_DOCS=OFF         # 禁用文档生成
        -DHTTP_ONLY=ON           # 只启用 HTTP/HTTPS
        -DCMAKE_USE_OPENSSL=ON
        -DCURL_DISABLE_LDAP=ON
        -DCURL_DISABLE_RTSP=ON
        -DCURL_DISABLE_DICT=ON
        -DCURL_DISABLE_TELNET=ON
        -DCURL_DISABLE_TFTP=ON
        -DCURL_DISABLE_POP3=ON
        -DCURL_DISABLE_IMAP=ON
        -DCURL_DISABLE_SMB=ON
        -DCURL_DISABLE_SMTP=ON
        -DCURL_DISABLE_GOPHER=ON
)

FetchContent_Declare(
    libevent
    GIT_REPOSITORY git@github.com:libevent/libevent.git
    GIT_TAG        release-2.1.12-stable  # 指定稳定版本
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/libevent
)

set(EVENT__DISABLE_TESTS ON)
set(EVENT__DISABLE_SAMPLES ON)

FetchContent_MakeAvailable(abseil curl libevent)

add_subdirectory(muduo)
add_subdirectory(my_test)
add_subdirectory(design_pattern)