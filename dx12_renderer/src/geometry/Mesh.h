#pragma once

#include <d3d12.h>

#include <vector>
#include <string>
#include <memory>

#include <geometry/IGeometry.h>
#include <geometry/PSOManager.h>
#include <resource/IResource.h>

class Mesh : public IGeometry
{
public:
    Mesh( std::string name, std::initializer_list<std::string> renderPassNames );
    ~Mesh() = default;

    void addInputLayoutElement( std::string semanticName, UINT semanticIndex, DXGI_FORMAT format );
    void setVertexBuffer( void* vertexData, UINT vertexSize, UINT vertexCount );
    void setIndexBuffer( UINT* indexData, UINT indexCount );
    void setShaders( std::string vertexShaderFilename, std::string pixelShaderFilename );
    void addResource( IResource* resource );

    void recordRenderPass( std::string techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState ) override;

private:
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<IndexBuffer> m_indexBuffer;
    std::wstring m_vertexShaderFilename;
    std::wstring m_pixelShaderFilename;
    std::vector< std::unique_ptr<IResource> > m_resources;
};