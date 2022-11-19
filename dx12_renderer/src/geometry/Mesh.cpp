#include "Mesh.h"

#include <geometry/ShaderManager.h>
#include <geometry/PSOManager.h>

#include <pix3.h>

Mesh::Mesh( std::string name, std::initializer_list<std::string> renderPassNames )
    : IGeometry( name, renderPassNames )
    , m_vertexBuffer( nullptr )
    , m_indexBuffer( nullptr )
{

}

void Mesh::addInputLayoutElement( std::string semanticName, UINT semanticIndex, DXGI_FORMAT format )
{
    m_semanticNames.push_back( semanticName );
    if ( m_inputLayout.size() == 0 )
    {
        m_inputLayout.push_back( { m_semanticNames.back().c_str(), semanticIndex, format, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );
    }
    else
    {
        m_inputLayout.push_back( { m_semanticNames.back().c_str(), semanticIndex, format, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } );
    }
}

void Mesh::setVertexBuffer( void* vertexData, UINT vertexSize, UINT vertexCount )
{
    m_vertexBuffer = std::make_unique<VertexBuffer>( vertexData, vertexSize, vertexCount );
}

void Mesh::setIndexBuffer( UINT* indexData, UINT indexCount )
{
    m_indexBuffer = std::make_unique<IndexBuffer>( indexData, indexCount );
}

void Mesh::setShaders( std::string vertexShaderFilename, std::string pixelShaderFilename )
{
    m_vertexShaderFilename = std::wstring( vertexShaderFilename.begin(), vertexShaderFilename.end() );
    m_pixelShaderFilename = std::wstring( pixelShaderFilename.begin(), pixelShaderFilename.end() );
}

void Mesh::record( std::string techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState )
{
    std::string eventString = m_name + "/" + techniqueName;
    PIXBeginEvent( commandList.Get(), PIX_COLOR_DEFAULT, eventString.c_str() );

    IGeometry::record( techniqueName, commandList, pipelineState );

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
    inputLayoutDesc.pInputElementDescs = m_inputLayout.data();
    inputLayoutDesc.NumElements = m_inputLayout.size();
    pipelineState.m_inputLayout = inputLayoutDesc;

    pipelineState.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    std::wstring techniqueNameWide = std::wstring( techniqueName.begin(), techniqueName.end() );
    ShaderManager::ShaderParams shaderParams;
    shaderParams.m_enableDebug = true;

    shaderParams.m_filename = m_vertexShaderFilename;
    shaderParams.m_entryPoint = techniqueNameWide + L"_vs";
    shaderParams.m_shaderType = ShaderManager::ShaderType::VertexShader;
    pipelineState.m_vertexShader = ShaderManager::it().getShader( shaderParams );

    shaderParams.m_filename = m_pixelShaderFilename;
    shaderParams.m_entryPoint = techniqueNameWide + L"_ps";
    shaderParams.m_shaderType = ShaderManager::ShaderType::PixelShader;
    pipelineState.m_pixelShader = ShaderManager::it().getShader( shaderParams );

    std::string PSOName = m_name + "/" + techniqueName;
    std::wstring PSONameWide = std::wstring( PSOName.begin(), PSOName.end() );
    commandList->SetPipelineState( PSOManager::it().getPSO( PSONameWide, pipelineState ).Get() );

    m_vertexBuffer->bind( commandList );
    m_indexBuffer->bind( commandList );

    commandList->DrawIndexedInstanced( m_indexBuffer->getIndexCount(), 1, 0, 0, 0 );

    PIXEndEvent( commandList.Get() );
}
