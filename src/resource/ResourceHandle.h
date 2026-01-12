#pragma once

#define RESOURCE_HANDLE_INVALID_INDEX 0xFFFFFFFF
#define RESOURCE_HANDLE_INVALID_GENERATION 0xFFFFFFFF

class Descriptor;

class ResourceHandle
{
    friend class ResourceManager;
public:
    ResourceHandle()
        : m_index(RESOURCE_HANDLE_INVALID_INDEX),
        m_generation(RESOURCE_HANDLE_INVALID_GENERATION)
    {};

    bool isValid() const;

    bool operator==(ResourceHandle const& other) const { return m_index == other.m_index && m_generation == other.m_generation; }

    Descriptor const& getDefaultCBV();
    Descriptor const& getDefaultSRV();
    Descriptor const& getDefaultUAV();
    Descriptor const& getDefaultRTV();
    Descriptor const& getDefaultDSV(D3D12_DSV_FLAGS flags = D3D12_DSV_FLAG_NONE);

    Descriptor const& getSRV(UINT mostDetailedMip, UINT numMips = 0);
    Descriptor const& getUAV(UINT mipSlice);

    Descriptor const& getCBV(D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc);
    Descriptor const& getSRV(D3D12_SHADER_RESOURCE_VIEW_DESC const& srvDesc);
    Descriptor const& getUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC const& uavDesc);
    Descriptor const& getRTV(D3D12_RENDER_TARGET_VIEW_DESC const& rtvDesc);
    Descriptor const& getDSV(D3D12_DEPTH_STENCIL_VIEW_DESC const& dsvDesc);

private:
    D3D12_CONSTANT_BUFFER_VIEW_DESC getDefaultCBVDesc();
    D3D12_SHADER_RESOURCE_VIEW_DESC getDefaultSRVDesc();
    D3D12_UNORDERED_ACCESS_VIEW_DESC getDefaultUAVDesc();
    D3D12_RENDER_TARGET_VIEW_DESC getDefaultRTVDesc();
    D3D12_DEPTH_STENCIL_VIEW_DESC getDefaultDSVDesc();

    ResourceHandle(unsigned int index, unsigned int generation)
        : m_index(index),
        m_generation(generation)
    {};
    unsigned int m_index;
    unsigned int m_generation;
};