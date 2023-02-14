#pragma once

#include <d3d12.h>

#include <vector>
#include <string>
#include <memory>
#include <list>

#include <geometry/IGeometry.h>
#include <geometry/PSOManager.h>

class Mesh : public IGeometry
{
public:
    Mesh( wchar_t const* name, std::initializer_list<wchar_t const*> renderPassNames );
    ~Mesh() = default;

    void addInputLayoutElement( char const* semanticName, UINT semanticIndex, DXGI_FORMAT format );
    void setVertexBuffer( void* vertexData, UINT vertexSize, UINT vertexCount );
    void setIndexBuffer( UINT* indexData, UINT indexCount );
    void setShaders( wchar_t const* vertexShaderFilename, wchar_t const* pixelShaderFilename );

    void record( wchar_t const* techniqueName, ComPtr<ID3D12GraphicsCommandList> commandList, PSOManager::PipelineStateStream& pipelineState ) override;

private:
    std::list<std::string> m_semanticNames;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<IndexBuffer> m_indexBuffer;
    std::wstring m_vertexShaderFilename;
    std::wstring m_pixelShaderFilename;
};