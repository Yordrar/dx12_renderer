#pragma once

#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <string>
#include <unordered_map>

#include <Manager.h>

class ShaderManager : public Manager<ShaderManager>
{
    friend class Manager<ShaderManager>;
public:
    enum class ShaderType
    {
        VertexShader,
        PixelShader,
        ComputeShader
    };

    struct ShaderParams
    {
        std::wstring m_filename;
        std::wstring m_entryPoint;
        ShaderType m_shaderType;
        bool m_enableDebug;
        std::vector<std::wstring> m_defines;
    };

    ~ShaderManager() = default;

    D3D12_SHADER_BYTECODE getShader( ShaderParams& params );

private:
    ShaderManager();

    std::string getShaderId( ShaderParams params );
    LPCWSTR shaderTypeToTargetString( ShaderType type );

    using ShaderMap = std::unordered_map< std::string, ComPtr<IDxcBlob> >;
    ShaderMap m_shaders;
    ComPtr<IDxcUtils> m_utils;
    ComPtr<IDxcCompiler3> m_compiler;
    ComPtr<IDxcIncludeHandler> m_includeHandler;
};