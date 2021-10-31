#pragma once

#include <memory>
#include <vector>

namespace CGCP
{
    template <typename T>
    class ObjectPool
    {
        using PtrT = std::shared_ptr<T>;

    private:
        std::vector<PtrT> buf_;
        std::size_t curr_;

        void clear(std::size_t begin = 0)
        {
            for (auto it = buf_.begin() + begin; it < buf_.end(); it++)
            {
                *it = std::make_shared<T>();
            }
        };

        void grow()
        {
            std::size_t old_size = buf_.size();
            buf_.reserve(old_size * 2);
            buf_.resize(buf_.capacity());

            clear(old_size);
        };

    public:
        ObjectPool() : buf_(), curr_(0)
        {
            buf_.resize(4);
            clear();
        };

        explicit ObjectPool(std::size_t size) : buf_(size), curr_(0)
        {
            buf_.resize(buf_.capacity());
            clear();
        };

        PtrT store(const T &obj)
        {
            if (curr_ >= buf_.size())
            {
                grow();
            }

            *(buf_[curr_]) = obj;

            return buf_[curr_++];
        };

        ~ObjectPool() = default;
    };
} // namespace CGCP
