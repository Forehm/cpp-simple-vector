#pragma once
#include <stdexcept>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <utility>

#include "array_ptr.h"


class ReserveProxyObj
{
public:
    explicit ReserveProxyObj(size_t capacity) : capacity_(capacity)
    {
    }

    size_t GetCapacity()
    {
        return capacity_;
    }

private:
    size_t capacity_ = 0;
};



ReserveProxyObj Reserve(size_t capacity_to_reserve) 
{
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector 
{
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    SimpleVector(ReserveProxyObj obj) 
    {
        SimpleVector<Type> vec;
        vec.Reserve(obj.GetCapacity());
        swap(vec);
    }

    explicit SimpleVector(size_t size)
        : SimpleVector(size, Type{})
    {
    }

    SimpleVector(size_t size, const Type& value): array_(size), size_(size), capacity_(size)
    {
        std::fill(array_.Get(), array_.Get() + size_, value);
    }

    SimpleVector(std::initializer_list<Type> other): array_(other.size()), size_(other.size()), capacity_(other.size())
    {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(const SimpleVector& other) : array_(other.GetSize()) 
    {
        std::copy(other.begin(), other.end(), begin());
        this->size_ = other.size_;
        this->capacity_ = other.capacity_;
    }

    SimpleVector(SimpleVector&& other)
    {
        array_ = std::move(other.array_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    SimpleVector& operator=(const SimpleVector& rhs) 
    {
        if (this != &rhs) 
        {
            if (rhs.IsEmpty()) 
            {
                Clear();
                return *this;
            }
            SimpleVector<Type> arr_ptr(rhs.size_);
            std::copy(rhs.begin(), rhs.end(), arr_ptr.begin());
            arr_ptr.capacity_ = rhs.capacity_;
            swap(arr_ptr);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) 
    {
        if (this != &rhs) 
        {
            if (rhs.IsEmpty()) 
            {
                Clear();
                return *this;
            }
            SimpleVector<Type> arr_ptr(rhs.size_);
            std::copy(rhs.begin(), rhs.end(), arr_ptr.begin());
            arr_ptr.capacity_ = rhs.capacity_;
            swap(arr_ptr);
        }
        return *this;
    }

    void Reserve(size_t capacity_to_reserve) 
    {
        if (capacity_to_reserve > capacity_) 
        {
            SimpleVector<Type> tmp_items(capacity_to_reserve);
            std::copy(cbegin(), cend(), tmp_items.begin());
            tmp_items.size_ = size_;
            swap(tmp_items);
        }
    }

    size_t GetSize() const noexcept 
    {
        return size_;
    }

    size_t GetCapacity() const noexcept 
    {
        return capacity_;
    }

    bool IsEmpty() const noexcept 
    {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept 
    {
        assert(index <= capacity_);
        return array_[index];
    }

    const Type& operator[](size_t index) const noexcept
    {
        assert(index <= capacity_);
        return array_[index];
    }

    Type& At(size_t index) 
    {
        if (index >= size_) 
        {
            throw std::out_of_range("invalid index");
        }
        return array_[index];
    }
    
    const Type& At(size_t index) const 
    {
        if (index >= size_) 
        {
            throw std::out_of_range("invalid index");
        }
        return array_[index];
    }

    void Clear() noexcept 
    {
        size_ = 0;
    }

    void Resize(size_t new_size) 
    {
        if (new_size == size_) { return; }

        if (new_size < size_) 
        {
            for (auto it = &array_[new_size]; it != &array_[size_]; ++it) 
            {
                *(it) = std::move(Type{});
            }
        }

        if (new_size > capacity_) 
        {
            auto new_capacity = std::max(new_size, 2 * capacity_);
            ArrayPtr<Type> arr_ptr(new_capacity);
            std::move(&array_[0], &array_[size_], &arr_ptr[0]);
            for (auto it = &arr_ptr[size_]; it != &arr_ptr[new_size]; ++it) 
            {
                *(it) = std::move(Type{});
            }
            array_.swap(arr_ptr);
            capacity_ = new_capacity;
        }

        size_ = new_size;
    }

    Iterator begin() noexcept 
    {
        return array_.Get();
    }

    Iterator end() noexcept 
    {
        return array_.Get() + size_;
    }

    ConstIterator begin() const noexcept 
    {
        return array_.Get();
    }

    ConstIterator end() const noexcept 
    {
        return array_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept 
    {
        return array_.Get();
    }

    ConstIterator cend() const noexcept 
    {
        return array_.Get() + size_;
    }

    void PushBack(const Type& item)
    {
        if (size_ == capacity_)
        {
            Reserve(capacity_ > 0 ? capacity_ * 2 : 1);
        }
        array_[size_] = item;
    }

    void PushBack(Type&& item) 
    {
        if (size_ < capacity_) 
        {
            array_[size_] = std::move(item);
        }
        else 
        {
            auto new_capacity = std::max(size_t(1), 2 * capacity_); 
            ArrayPtr<Type> arr_ptr(new_capacity);
            std::move(&array_[0], &array_[size_], &arr_ptr[0]);
            arr_ptr[size_] = std::move(item);
            array_.swap(arr_ptr);
            capacity_ = new_capacity;
        }
        ++size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value) 
    {
        assert(pos >= cbegin() && pos <= cend());

        auto pos_element = std::distance(cbegin(), pos);

        if (size_ < capacity_)
        {
            std::copy_backward(pos, cend(), &array_[(size_ + 1)]);
            array_[pos_element] = value;
        }
        else 
        {
            auto new_capacity = std::max(size_t(1), 2 * capacity_); 
            ArrayPtr<Type> arr_ptr(new_capacity);
            std::copy(&array_[0], &array_[pos_element], &arr_ptr[0]);
            std::copy_backward(pos, cend(), &arr_ptr[(size_ + 1)]);
            arr_ptr[pos_element] = value;
            array_.swap(arr_ptr);
            capacity_ = new_capacity;
        }

        ++size_;
        return Iterator{ &array_[pos_element] };
    }

    Iterator Insert(ConstIterator pos, Type&& value) 
    {
        assert(pos >= cbegin() && pos <= cend());

        auto no_const_pos = const_cast<Iterator>(pos);
        auto pos_element = std::distance(begin(), no_const_pos);

        if (size_ < capacity_) 
        {
            std::move_backward(no_const_pos, end(), &array_[(size_ + 1)]);
            array_[pos_element] = std::move(value);
        }
        else 
        {
            auto new_capacity = std::max(size_t(1), 2 * capacity_); 
            ArrayPtr<Type> arr_ptr(new_capacity);
            std::move(&array_[0], &array_[pos_element], &arr_ptr[0]);
            std::move_backward(no_const_pos, end(), &arr_ptr[(size_ + 1)]);
            arr_ptr[pos_element] = std::move(value);
            array_.swap(arr_ptr);
            capacity_ = new_capacity;
        }

        ++size_;
        return Iterator{ &array_[pos_element] };
    }

    void PopBack() noexcept 
    {
        assert(!IsEmpty());
        --size_;
    }

    Iterator Erase(ConstIterator pos)
    {
        assert(pos >= cbegin() && pos < cend());
        auto no_const_pos = const_cast<Iterator>(pos);
        auto pos_element = std::distance(begin(), no_const_pos);
        std::move(++no_const_pos, end(), &array_[pos_element]);
        --size_;
        return &array_[pos_element];
    }

    void swap(SimpleVector& other) noexcept 
    {
        array_.swap(other.array_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    ArrayPtr<Type> array_;

    size_t size_ = 0;
    size_t capacity_ = 0;

};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) 
{
    if (lhs.GetSize() != rhs.GetSize()) return false;
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) 
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) 
{
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) 
{
    return !(rhs > lhs);
}
