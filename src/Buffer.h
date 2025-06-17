#pragma once

#include <bits/types/struct_iovec.h>
#include <cstddef>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include <string>
namespace mymuduo {
    namespace base {
        class Buffer {
        public:
            static const size_t kCheapPrepend = 8;   // 用于记录空间大小
            static const size_t kInitialSize = 1024; // 初始缓冲区大小
            explicit Buffer(size_t initialSize = kInitialSize)
             : buffer_(kCheapPrepend + kInitialSize)
             , readerIndex_(kCheapPrepend)
             , writerIndex_(kCheapPrepend) {

             }
            ~Buffer() {}
            size_t readableBytes() const {
                return writerIndex_ - readerIndex_; // 返回可读字节数
            }

            size_t writableBytes() const {
                return buffer_.size() - writerIndex_; // 返回可写字节数
            }

            size_t prependableBytes() const {
                return readerIndex_; // 返回预留字节数
            }

            const char* peek() const {
                return begin() + readerIndex_; // 返回可读数据的起始地址
            }

            void retrieve(size_t len) {
                if (len <= readableBytes()) {
                    readerIndex_ += len;
                } else {
                    retrieveAll();
                }
            }

            void retrieveAll() {
                readerIndex_ = writerIndex_ = kCheapPrepend; // 重置读写指针
            }

            // 把OnMessage函数上报的buffer数据转成string类型数据返回
            std::string retrieveAllAsString() {
                return retrieveAsString(readableBytes());
            }

            std::string retrieveAsString(size_t len) {
                std::string result(peek(), len);
                retrieve(len); // 上面已经把缓冲区的可读数据读取出来，这里需要对缓冲区进行复位操作
                return result; // 返回读取的数据
            }

            void ensureWritableBytes(size_t len) {
                if (writableBytes() < len) {
                    makeSpace(len); // 确保有足够的可写空间
                }
            }

            void append(const char* data, size_t len) {
                ensureWritableBytes(len); // 确保有足够的可写空间
                std::copy(data, data + len, beginWrite()); // 将数据复制到缓冲区
                writerIndex_ += len; // 更新写指针
            }

            char *beginWrite() {
                return begin() + writerIndex_; // 返回下一个可写位置
            }

            const char *beginWrite() const {
                return begin() + writerIndex_; // 返回下一个可写位置
            }

            // 从fd上读取数据
            ssize_t readFd(int fd, int* saveErrno) {
                char extraBuffer[65536]; // 栈上的内存空间
                struct iovec vec[2];
                const size_t writable = writableBytes();
                vec[0].iov_base = beginWrite(); // 第一个缓冲区指向
                vec[0].iov_len = writable; // 第一个缓冲区的长度
                vec[1].iov_base = extraBuffer; // 第二个缓冲区指向
                vec[1].iov_len = sizeof(extraBuffer); // 第二个缓冲区的

                const int iovcnt = (writable < sizeof(extraBuffer)) ? 2 : 1; // 确定使用的缓冲区数量
                const ssize_t n = ::recv(fd, &vec, iovcnt, 0); // 从fd读取数据
                if (n < 0) {
                    *saveErrno = errno; // 保存错误码
                } else if (n <= writable) {
                    writerIndex_ += n; // 更新写指针
                } else {
                    writerIndex_ = buffer_.size(); // 更新写指针到缓冲区末尾
                    append(extraBuffer, n - writable); // 将额外数据添加到缓冲区
                }
                return n; // 返回读取的字节数
            }

        private:
            char *begin() {
                return buffer_.data(); // 返回缓冲区的起始地址
            }

            const char *begin() const {
                return buffer_.data(); // 返回缓冲区的起始地址
            }

            void makeSpace(size_t len) {
                if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
                    buffer_.resize(writerIndex_ + len); // 扩展缓冲区大小
                } else {
                    size_t readable = readableBytes();
                    std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
                    readerIndex_ = kCheapPrepend;
                    writerIndex_ = readerIndex_ + readable; // 更新读写指针
                }
            }
            std::vector<char> buffer_; // 存储数据的缓冲区
            size_t readerIndex_; // 读指针，指向下一个要读取的位置
            size_t writerIndex_; // 写指针，指向下一个要写入的位置
        };
    }
}