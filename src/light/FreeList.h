#pragma once

#include <cstdint>
#include <limits>
#include <vector>

namespace light
{

constexpr auto NIL = std::numeric_limits<uint32_t>::max();

/// <summary>
/// Provides an indexed free list with constant-tim removals from anywhere in the list without
/// invalidating indices. T must be trivially constructible and destructible.
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T>
class FreeList
{
public:
    FreeList();
    FreeList(uint32_t capacity);

    uint32_t push_back(const T& value);

    void erase(uint32_t index);

    void clear();

    size_t size() const;

    void reserve(size_t capacity);

    size_t range() const;

    T& operator[](uint32_t index);

    const T& operator[](uint32_t index) const;

private:
    union FreeElement
    {
        uint32_t next;
        T data;
    };
    std::vector<FreeElement> m_data;
    size_t m_size;
    uint32_t m_firstFree;
};

template<typename T>
FreeList<T>::FreeList()
  : m_data{}
  , m_size{ 0 }
  , m_firstFree{ NIL }
{
}

template<typename T>
FreeList<T>::FreeList(uint32_t capacity)
  : m_data{}
  , m_size{ 0 }
  , m_firstFree{ NIL }
{
    m_data.reserve(capacity);
}

template<typename T>
uint32_t FreeList<T>::push_back(const T& value)
{
    ++m_size;

    if (m_firstFree == NIL)
    {
        FreeElement newElement;
        newElement.data = value;
        m_data.push_back(newElement);
        return static_cast<uint32_t>(m_data.size() - 1);
    }
    else
    {
        const auto newElementIndex = m_firstFree;
        m_firstFree = m_data[m_firstFree].next;
        m_data[newElementIndex].data = value;
        return newElementIndex;
    }
}

template<typename T>
void FreeList<T>::erase(uint32_t index)
{
    m_data[index].next = m_firstFree;
    m_firstFree = index;
    --m_size;
}

template<typename T>
void FreeList<T>::clear()
{
    m_data.clear();
    m_firstFree = NIL;
}

template<typename T>
size_t FreeList<T>::size() const
{
    return m_size;
}

template<typename T>
void FreeList<T>::reserve(size_t capacity)
{
    m_data.reserve(capacity);
}

template<typename T>
size_t FreeList<T>::range() const
{
    return m_data.size();
}

template<typename T>
T& FreeList<T>::operator[](uint32_t index)
{
    return m_data[index].data;
}

template<typename T>
const T& FreeList<T>::operator[](uint32_t index) const
{
    return m_data[index].data;
}

}
