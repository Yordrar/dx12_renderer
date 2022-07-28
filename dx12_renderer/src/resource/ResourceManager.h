#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <memory>

#include <resource/ResourceHandle.h>
#include <resource/DescriptorHeap.h>

class ResourceManager
{
public:
    static ResourceManager* it() 
    {
        if ( !s_instance )
        {
            s_instance = new ResourceManager();
        }

        return s_instance;
    }

    DescriptorHeap* getSrvCbvUavDescriptorHeap() const { return m_descriptorHeapSrvCbvUav.get(); }
    DescriptorHeap* getSamplerDescriptorHeap() const { return m_descriptorHeapSampler.get(); }
    DescriptorHeap* getRtvDescriptorHeap() const { return m_descriptorHeapRtv.get(); }
    DescriptorHeap* getDsvDescriptorHeap() const { return m_descriptorHeapDsv.get(); }

private:
    ResourceManager();
    static ResourceManager* s_instance;

    std::unique_ptr<DescriptorHeap> m_descriptorHeapSrvCbvUav;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapSampler;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapRtv;
    std::unique_ptr<DescriptorHeap> m_descriptorHeapDsv;
};

