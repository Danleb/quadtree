#pragma once

#include <cstdint>
#include <vector>

namespace light
{
constexpr auto STACK_ELEMENTS_COUNT = 128;

/// <summary>
/// Provides an indexed vector with small stack allocation base.
///  T must be trivially constructible and destructible.
/// </summary>
template<typename T>
class FastArray
{
public:
    FastArray();

    T& operator[](uint32_t index);

    const T& operator[](uint32_t index) const;

    bool empty() const;

    size_t size() const;

    void push_back(const T& value);

    T pop();

    T peek() const;

private:
    uint32_t m_stackDataSize;
    T m_stackData[STACK_ELEMENTS_COUNT];
    std::vector<T> m_heapData;
};

template<typename T>
FastArray<T>::FastArray()
  : m_stackDataSize{ 0 }
{
}

template<typename T>
T& FastArray<T>::operator[](uint32_t index)
{
    if (index >= STACK_ELEMENTS_COUNT)
    {
        return m_heapData[index - STACK_ELEMENTS_COUNT];
    }
    else
    {
        return m_stackData[index];
    }
}

template<typename T>
const T& FastArray<T>::operator[](uint32_t index) const
{
    return const_cast<FastArray*>(this)->operator[](index);
}

template<typename T>
bool FastArray<T>::empty() const
{
    return size() == 0;
}

template<typename T>
size_t FastArray<T>::size() const
{
    return m_stackDataSize + m_heapData.size();
}

template<typename T>
void FastArray<T>::push_back(const T& value)
{
    if (m_stackDataSize == STACK_ELEMENTS_COUNT)
    {
        m_heapData.push_back(value);
    }
    else
    {
        m_stackData[m_stackDataSize] = value;
        ++m_stackDataSize;
    }
}

template<typename T>
T FastArray<T>::pop()
{
    const auto sz = size();
    const auto value = (*this)[sz - 1];
    if (sz > STACK_ELEMENTS_COUNT)
    {
        m_heapData.resize(m_heapData.size() - 1);
    }
    else
    {
        --m_stackDataSize;
    }
    return value;
}

template<typename T>
T FastArray<T>::peek() const
{
    return (*this)[size() - 1];
}

}
