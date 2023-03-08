#include "Descriptor.h"

Descriptor::Descriptor( D3D12_CPU_DESCRIPTOR_HANDLE const& cpuDescriptorHandleForHeapStart, UINT offsetInDescriptors, UINT descriptorIncrementSize )
    : m_descriptorIndex( offsetInDescriptors )
{
    m_descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE( cpuDescriptorHandleForHeapStart,
                                                  offsetInDescriptors,
                                                  descriptorIncrementSize );
}

Descriptor::~Descriptor()
{

}
