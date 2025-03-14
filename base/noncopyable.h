#ifndef MYMUDUO_BASE_NONCOPYABLE_H
#define MYMUDUO_BASE_NONCOPYABLE_H

namespace mymuduo
{

class noncopyable{
    private:
    public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
    protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

}

#endif