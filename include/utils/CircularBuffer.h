#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <array>
#include <cstddef>
#include <optional>

template <typename T, std::size_t Size> class CircularBuffer
{
private:
    std::array<T, Size> mData;
    std::size_t mFront = 0;
    std::size_t mBack = 0;
    std::size_t mCount = 0;

public:
    /**
     * @brief Push a value to the back of the buffer
     */
    void push_back(const T &value)
    {
        mData[mBack] = value;
        mBack = (mBack + 1) % Size;

        if (mCount == Size)
        {
            mFront = (mFront + 1) % Size;
        }
        else
        {
            ++mCount;
        }
    }

    /**
     * @brief Clear the buffer
     */
    void clear()
    {
        mFront = 0;
        mBack = 0;
        mCount = 0;
    }

    /**
     * @brief Pop a value from the front of the buffer
     */
    std::optional<T> pop_front()
    {
        if (empty())
        {
            return std::nullopt;
        }

        T value = mData[mFront];
        mFront = (mFront + 1) % Size;
        --mCount;

        return value;
    }

    /**
     * @brief Pop a value from the back of the buffer
     */
    std::optional<T> pop_back()
    {
        if (empty())
        {
            return std::nullopt;
        }

        mBack = (mBack == 0) ? Size - 1 : mBack - 1;
        T value = mData[mBack];
        --mCount;

        return value;
    }

    /**
     * @brief Get the front value of the buffer
     */
    std::optional<T> front() const
    {
        if (!empty())
        {
            return mData[mFront];
        }
        else
        {
            return std::nullopt;
        }
    }

    /**
     * @brief Get the back value of the buffer
     */
    std::optional<T> back() const
    {
        if (!empty())
        {
            return mData[(mBack == 0) ? Size - 1 : mBack - 1];
        }
        else
        {
            return std::nullopt;
        }
    }

    /**
     * @brief Get the value at a specific index
     */
    std::optional<T> at(std::size_t index) const
    {
        if (index < mCount && index >= 0)
        {
            return mData[(mFront + index) % Size];
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<T> find_if(std::function<bool(const T &)> predicate) const
    {
        for (std::size_t i = 0; i < mCount; ++i)
        {
            if (predicate(mData[(mFront + i) % Size]))
            {
                return mData[(mFront + i) % Size];
            }
        }

        return std::nullopt;
    }

    /**
     * @brief Get the value at a specific index
     */
    std::optional<T> operator[](std::size_t index) const { return at(index); }

    /**
     * @brief Get whether the buffer is empty
     */
    bool empty() const { return mCount == 0; }

    /**
     * @brief Get whether the buffer is full
     */
    bool full() const { return mCount == Size; }

    /**
     * @brief Get the size of the buffer
     */
    std::size_t size() const { return mCount; }
};

#endif // CIRCULAR_BUFFER_H
