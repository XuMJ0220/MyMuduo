#ifndef MYMUDUO_BASE_TYPES_H
#define MYMUDUO_BASE_TYPES_H

#include <cstring>

namespace mymuduo{
    inline void memZero(void* p,size_t n){
        memset(p,0,n);
    }

}

#endif // !MYMUDUO_BASE_TYPES_H