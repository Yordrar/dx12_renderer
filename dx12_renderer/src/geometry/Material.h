#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>
#include <string>
#include <unordered_map>

#include <resource/Descriptor.h>

class Resource;

class Material
{
public:
    struct Technique
    {
        std::wstring m_name;
        std::wstring m_shaderFilename;
        CD3DX12_BLEND_DESC m_blendState = CD3DX12_BLEND_DESC( CD3DX12_DEFAULT{} );
        CD3DX12_RASTERIZER_DESC m_rasterizerState = CD3DX12_RASTERIZER_DESC( CD3DX12_DEFAULT{} );
        CD3DX12_DEPTH_STENCIL_DESC m_depthStencilState = CD3DX12_DEPTH_STENCIL_DESC( CD3DX12_DEFAULT{} );
        D3D12_PRIMITIVE_TOPOLOGY_TYPE m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        CD3DX12_RT_FORMAT_ARRAY m_rtFormats;
        DXGI_FORMAT m_dsFormat;
    };

    struct MaterialDesc
    {
        std::wstring m_name;
        std::vector<Technique> m_techniques;
        std::vector<Descriptor> m_resourceViews;
        std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
    };

    struct MaterialBufferData
    {
        uint32_t bindlessIndicesBufferIndex;
    };

    Material( MaterialDesc const& materialDesc );
    ~Material() = default;

    std::wstring const& getName() const { return m_desc.m_name; }
    ComPtr<ID3D12PipelineState> getPSOForTechnique( wchar_t const* techniqueName ) const;
    Resource const* getMaterialBufferResource() const { return m_materialBuffer; }
    Resource const* getBindlessIndicesBufferResource() const { return m_bindlessIndicesBufferResource; }

private:
    MaterialDesc m_desc;

    Resource* m_materialBuffer;
    MaterialBufferData m_materialBufferData;

    Resource* m_bindlessIndicesBufferResource;
    std::vector<UINT> m_bindlessIndices;
    
    // PSO name is: <technique_name>
    using PSOCache = std::unordered_map< std::wstring, ComPtr<ID3D12PipelineState> >;
    PSOCache m_psoCache;
};