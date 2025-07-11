#pragma once

#include <bits/stdint-intn.h>
#include <string>

// 时间类
namespace mymuduo {
    namespace base {
        class Timestamp {
        public:
            Timestamp();
            explicit Timestamp(int64_t microSecSinceEpoch);
            static Timestamp now();
            std::string toString() const;
        private:
            int64_t microSecSinceEpoch_;
        };
    }
}