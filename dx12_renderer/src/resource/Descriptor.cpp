#include "Descriptor.h"

#include <resource/Resource.h>
#include <resource/DescriptorHeap.h>

Descriptor::Descriptor( Type type,
                        Resource* resource,
                        D3D12_CPU_DESCRIPTOR_HANDLE const& cpuDescriptorHandleForHeapStart,
                        UINT offsetInDescriptors,
                        UINT descriptorIncrementSize,
                        D3D12_DSV_FLAGS dsvFlags )
    : m_resource( resource )
    , m_type( type )
    , m_descriptorIndex( offsetInDescriptors )
    , m_dsvFlags( dsvFlags )
{
    m_descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE( cpuDescriptorHandleForHeapStart,
                                                  offsetInDescriptors,
                                                  descriptorIncrementSize );
}

Descriptor::~Descriptor()
{
    switch ( m_type )
    {
        case Type::ConstantBufferView:
        case Type::ShaderResourceView:
        case Type::UnorderedAccessView:
            DescriptorHeap::getDescriptorHeapCbvSrvUav().removeDescriptor( *this );
            break;
        case Type::RenderTargetView:
            DescriptorHeap::getDescriptorHeapRtv().removeDescriptor( *this );
            break;
        case Type::DepthStencilView:
            DescriptorHeap::getDescriptorHeapDsv().removeDescriptor( *this );
            break;
        default:
            break;
    }
}
