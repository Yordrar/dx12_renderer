#include "Descriptor.h"

#include <resource/Resource.h>

Descriptor::Descriptor( Resource* resource,
                        Type type,
                        D3D12_CPU_DESCRIPTOR_HANDLE const& cpuDescriptorHandleForHeapStart,
                        UINT offsetInDescriptors,
                        UINT descriptorIncrementSize )
    : m_resource( resource )
    , m_type( type )
    , m_descriptorIndex( offsetInDescriptors )
{
    m_descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE( cpuDescriptorHandleForHeapStart,
                                                  offsetInDescriptors,
                                                  descriptorIncrementSize );
}

Descriptor::~Descriptor()
{

}
