#pragma once

#include <d3d12.h>

#include <vector>
#include <string>
#include <memory>
#include <list>
#include <unordered_map>

#include <geometry/IGeometry.h>
#include <geometry/PSOManager.h>

class Mesh : public IGeometry
{
public:
    Mesh( std::wstring name, std::initializer_list<std::wstring> renderPassNames );
    ~Mesh() = default;

    void addInputLayoutElement( std::string semanticName, UINT semanticIndex, DXGI_FORMAT format );
    void setVertexBuffer( void* vertexData, UINT vertexSize, UINT vertexCount );
    void setIndexBuffer( UINT* indexData, UINT indexCount );
    void setShaders( std::wstring vertexShaderFilename, std::wstring pixelShaderFilename );

    void record( std::wstring techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState ) override;

private:
    std::list<std::string> m_semanticNames;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<IndexBuffer> m_indexBuffer;
    std::wstring m_vertexShaderFilename;
    std::wstring m_pixelShaderFilename;
    std::unordered_map<std::wstring, ID3D12PipelineState*> m_psoCache;
};