#pragma once

#include <string>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <Renderer.h>
#include <bindable/VertexShader.h>
#include <bindable/PixelShader.h>
#include <resource/Texture.h>

class Material
{
public:
    Material( std::string vertexShaderFilename, std::string pixelShaderFilename );
    ~Material();

    void bind( Renderer::RenderContext& context );

private:
    VertexShader m_vertexShader;
    PixelShader m_pixelShader;
    std::vector<IBindable*> m_shaderResources;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
};

