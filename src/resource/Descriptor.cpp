#include "Descriptor.h"

#include <resource/Resource.h>
#include <resource/DescriptorHeap.h>
#include <bindless.h>

Descriptor::Descriptor()
    : m_type(Type::InvalidView)
    , m_descriptorIndex(INVALID_DESCRIPTOR_INDEX)
{
    m_descriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT{});
}

Descriptor::Descriptor( Type type,
                        ResourceHandle handle,
                        D3D12_CPU_DESCRIPTOR_HANDLE const& cpuDescriptorHandleForHeapStart,
                        UINT offsetInDescriptors,
                        UINT descriptorIncrementSize )
    : m_type( type )
    , m_resource( handle )
    , m_descriptorIndex( offsetInDescriptors )
{
    m_descriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE( cpuDescriptorHandleForHeapStart,
                                                  offsetInDescriptors,
                                                  descriptorIncrementSize );
}

Descriptor::~Descriptor()
{
}

void Descriptor::invalidate()
{
    if (!isValid())
        return;

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
    m_type = Type::InvalidView;
    m_descriptorIndex = INVALID_DESCRIPTOR_INDEX;
    m_descriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT{});
}
