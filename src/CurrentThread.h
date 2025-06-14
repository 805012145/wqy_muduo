#pragma once

namespace mymuduo {
    namespace CurrentThread {
        thread_local int t_cachedTid = 0;

        void cacheTid();

        inline int tid() {
            if (__builtin_expect(t_cachedTid == 0, 0)) {
                cacheTid();
            }
            return t_cachedTid;
        }
    }
}