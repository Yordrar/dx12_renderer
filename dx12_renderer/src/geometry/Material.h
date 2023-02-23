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
    struct MaterialDesc
    {
        std::wstring m_name;
        std::vector<std::wstring> m_techniqueNames;
        std::vector<Descriptor> m_resourceViews;
        std::wstring m_vertexShaderFilename;
        std::wstring m_pixelShaderFilename;
        CD3DX12_BLEND_DESC m_blendState = CD3DX12_BLEND_DESC( CD3DX12_DEFAULT{} );
        CD3DX12_RASTERIZER_DESC m_rasterizerState = CD3DX12_RASTERIZER_DESC( CD3DX12_DEFAULT{} );
        CD3DX12_DEPTH_STENCIL_DESC m_depthStencilState = CD3DX12_DEPTH_STENCIL_DESC( CD3DX12_DEFAULT{} );
        std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
        D3D12_PRIMITIVE_TOPOLOGY_TYPE m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        CD3DX12_RT_FORMAT_ARRAY m_rtFormats;
        DXGI_FORMAT m_dsFormat;
    };

    Material( MaterialDesc const& materialDesc );
    ~Material() = default;

    std::wstring const& getName() const { return m_desc.m_name; }
    ComPtr<ID3D12PipelineState> getPSOForTechnique( wchar_t const* techniqueName ) const
    {
        PSOCache::const_iterator it = m_psoCache.find( techniqueName );
        if ( it != m_psoCache.end() )
        {
            return it->second;
        }

        return nullptr;
    }

    void bindToPipeline( ComPtr<ID3D12GraphicsCommandList> commandList );

private:
    MaterialDesc m_desc;

    Resource* m_bindlessIndicesResource;
    std::vector<UINT> m_bindlessIndices;
    
    // PSO name is: <technique_name>
    using PSOCache = std::unordered_map< std::wstring, ComPtr<ID3D12PipelineState> >;
    PSOCache m_psoCache;
};