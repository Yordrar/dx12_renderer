#include "Mesh.h"

#include <geometry/ShaderManager.h>
#include <geometry/PSOManager.h>

#include <pix3.h>

Mesh::Mesh( std::wstring name, std::initializer_list<std::wstring> renderPassNames )
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

void Mesh::setShaders( std::wstring vertexShaderFilename, std::wstring pixelShaderFilename )
{
    m_vertexShaderFilename = std::wstring( vertexShaderFilename.begin(), vertexShaderFilename.end() );
    m_pixelShaderFilename = std::wstring( pixelShaderFilename.begin(), pixelShaderFilename.end() );
}

void Mesh::record( std::wstring techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState )
{
    std::wstring eventString = m_name + L"/" + techniqueName;
    PIXBeginEvent( commandList.Get(), PIX_COLOR_DEFAULT, eventString.c_str() );

    IGeometry::record( techniqueName, commandList, pipelineState );
    pipelineState.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
    inputLayoutDesc.pInputElementDescs = m_inputLayout.data();
    inputLayoutDesc.NumElements = static_cast<UINT>( m_inputLayout.size() );
    pipelineState.m_inputLayout = inputLayoutDesc;

    ShaderManager::ShaderDesc shaderDesc;
    shaderDesc.m_enableDebug = true;

    shaderDesc.m_filename = m_vertexShaderFilename;
    shaderDesc.m_entryPoint = techniqueName + L"_vs";
    shaderDesc.m_shaderType = ShaderManager::ShaderType::VertexShader;
    pipelineState.m_vertexShader = ShaderManager::it().getShader(shaderDesc);

    shaderDesc.m_filename = m_pixelShaderFilename;
    shaderDesc.m_entryPoint = techniqueName + L"_ps";
    shaderDesc.m_shaderType = ShaderManager::ShaderType::PixelShader;
    pipelineState.m_pixelShader = ShaderManager::it().getShader(shaderDesc);

    commandList->SetPipelineState( PSOManager::it().getPSO( pipelineState ).Get() );

    m_vertexBuffer->bind( commandList );

    if( m_indexBuffer )
    {
        m_indexBuffer->bind(commandList);
        commandList->DrawIndexedInstanced(m_indexBuffer->getIndexCount(), 1, 0, 0, 0);
    }
    else
    {
        commandList->DrawInstanced(m_vertexBuffer->getVertexCount(), 1, 0, 0);
    }

    PIXEndEvent( commandList.Get() );
}
