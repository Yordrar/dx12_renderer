#pragma once

#define RESOURCE_HANDLE_INVALID_INDEX 0xFFFFFFFF
#define RESOURCE_HANDLE_INVALID_GENERATION 0xFFFFFFFF

class ResourceHandle
{
    friend class ResourceManager;
public:
    ResourceHandle()
        : m_index(RESOURCE_HANDLE_INVALID_INDEX),
        m_generation(RESOURCE_HANDLE_INVALID_GENERATION)
    {};
    bool isValid() const { return m_index != RESOURCE_HANDLE_INVALID_INDEX && m_generation != RESOURCE_HANDLE_INVALID_GENERATION; }

    bool operator==(ResourceHandle const& other) const { return m_index == other.m_index && m_generation == other.m_generation; }
private:
    ResourceHandle(unsigned int index, unsigned int generation)
        : m_index(index),
        m_generation(generation)
    {};
    unsigned int m_index;
    unsigned int m_generation;
};