#pragma once

#include <geometry/VertexBuffer.h>
#include <geometry/IndexBuffer.h>
#include <geometry/Material.h>

class Mesh
{
public:
    struct Submesh
    {
        std::string m_name;
        std::wstring m_materialName;
        std::unique_ptr<VertexBuffer> m_vertexBuffer;
        std::unique_ptr<IndexBuffer> m_indexBuffer;
        std::wstring m_shaderFilepath;
        D3D12_PRIMITIVE_TOPOLOGY m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        D3D12_PRIMITIVE_TOPOLOGY_TYPE m_primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        CD3DX12_RASTERIZER_DESC m_rasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT{});

        std::unique_ptr<Material> m_material = nullptr;

        void record(ComPtr<ID3D12GraphicsCommandList> commandList) const;
    };

    struct Vertex
    {
        DirectX::XMFLOAT3 m_position;
        DirectX::XMFLOAT3 m_normal;
        DirectX::XMFLOAT2 m_uvs;
        DirectX::XMFLOAT3 m_tangent;
        DirectX::XMFLOAT3 m_bitangent;
    };

    Mesh( wchar_t const* objFilepath, wchar_t const* shaderFilepath );
    ~Mesh() = default;

    void record(ComPtr<ID3D12GraphicsCommandList> commandList) const;

    std::vector<Submesh> const& getSubmeshes() const { return m_submeshes; }
    static std::vector<D3D12_INPUT_ELEMENT_DESC> const& getInputLayout() { return s_inputLayout; }

    void setFrontCounterClockwise(bool frontCounterClockwise);

private:
    friend class Renderer;
    static std::vector<D3D12_INPUT_ELEMENT_DESC> s_inputLayout;

    std::vector<Submesh> m_submeshes;
};