#include "IGeometry.h"

#include <algorithm>

#include <resource/Descriptor.h>
#include <resource/ResourceManager.h>

IGeometry::IGeometry( std::wstring name, std::initializer_list<std::wstring> techniqueNames )
    : m_name( name )
    , m_techniqueNames( techniqueNames )
    , m_bindlessIndices( nullptr )
{
}

IGeometry::~IGeometry()
{
}

void IGeometry::record( std::wstring techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState )
{
    if ( !m_bindlessIndices )
    {
        m_bindlessIndices = ResourceManager::it().createResource( m_name + L"_bindlessBuffer",
                                                                  CD3DX12_RESOURCE_DESC::Buffer( std::max( m_resourceIndices.size() * sizeof( float ), 1Ui64 ) ),
                                                                  D3D12_SUBRESOURCE_DATA{ m_resourceIndices.data(), static_cast<LONG_PTR>( m_resourceIndices.size() * sizeof( float ) ), 0 } );
    }
    commandList->SetGraphicsRootConstantBufferView( 1, m_bindlessIndices->getGPUVirtualAddress() );
}

void IGeometry::addResourceView( Descriptor const& resourceView )
{
    m_resourceIndices.push_back( resourceView.getDescriptorIndex() );
}