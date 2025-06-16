#pragma once

namespace mymuduo {
    namespace CurrentThread {
        extern thread_local int t_cachedTid;

        void cacheTid();

        inline int tid() {
            if (__builtin_expect(t_cachedTid == 0, 0)) {
                cacheTid();
            }
            return t_cachedTid;
        }
    }
}