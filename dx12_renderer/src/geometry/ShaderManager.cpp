#include "ShaderManager.h"

#include <Windows.h>

#include <d3dx12.h>

#include <regex>
#include <cstdio>

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode( const std::wstring& wstr )
{
    if ( wstr.empty() ) return std::string();
    int size_needed = WideCharToMultiByte( CP_UTF8, 0, &wstr[ 0 ], (int)wstr.size(), NULL, 0, NULL, NULL );
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte( CP_UTF8, 0, &wstr[ 0 ], (int)wstr.size(), &strTo[ 0 ], size_needed, NULL, NULL );
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode( const std::string& str )
{
    if ( str.empty() ) return std::wstring();
    int size_needed = MultiByteToWideChar( CP_UTF8, 0, &str[ 0 ], (int)str.size(), NULL, 0 );
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar( CP_UTF8, 0, &str[ 0 ], (int)str.size(), &wstrTo[ 0 ], size_needed );
    return wstrTo;
}

ShaderManager::ShaderManager()
    : m_utils( nullptr )
    , m_compiler( nullptr )
    , m_includeHandler( nullptr )
{
    DxcCreateInstance( CLSID_DxcUtils, IID_PPV_ARGS( &m_utils ) );
    DxcCreateInstance( CLSID_DxcCompiler, IID_PPV_ARGS( &m_compiler ) );
    m_utils->CreateDefaultIncludeHandler( &m_includeHandler );
}

D3D12_SHADER_BYTECODE ShaderManager::getShader( ShaderDesc& params )
{
    ShaderMap::iterator it = m_shaders.find( getShaderId( params ) );

    if ( it != m_shaders.end() )
    {
        CD3DX12_SHADER_BYTECODE shaderBytecode( it->second->GetBufferPointer(), it->second->GetBufferSize() );
        return shaderBytecode;
    }

    std::string filename = utf8_encode(params.m_filename);

    std::string csoFilename = std::regex_replace(filename, std::regex("hlsl"), "cso");
    std::wstring csoFilenameWideStr = std::wstring(csoFilename.begin(), csoFilename.end());

    std::string pdbFilename = std::regex_replace(filename, std::regex("hlsl"), "pdb");
    std::wstring pdbFilenameWideStr = std::wstring(pdbFilename.begin(), pdbFilename.end());

    std::vector<LPCWSTR> compileArgs;
    compileArgs.push_back( params.m_filename.c_str() );
    compileArgs.push_back( L"-E" );
    compileArgs.push_back( params.m_entryPoint.c_str() );
    compileArgs.push_back( L"-T" );
    compileArgs.push_back( shaderTypeToTargetString( params.m_shaderType ) );
    if ( params.m_enableDebug )
    {
        compileArgs.push_back( L"-Zi" );
    }
    if ( params.m_defines.size() > 0 )
    {
        compileArgs.push_back( L"-D" );
        for ( std::wstring& define : params.m_defines )
        {
            compileArgs.push_back( define.c_str() );
        }
    }
    compileArgs.push_back( L"-Fo" );
    compileArgs.push_back( csoFilenameWideStr.c_str() );
    compileArgs.push_back( L"-Fd" );
    compileArgs.push_back( pdbFilenameWideStr.c_str() );

    // Open source file.
    ComPtr<IDxcBlobEncoding> sourceBlob = nullptr;
    m_utils->LoadFile( params.m_filename.c_str(), nullptr, &sourceBlob );
    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();
    sourceBuffer.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

    // Compile it with specified arguments.
    ComPtr<IDxcResult> compilationResult;
    m_compiler->Compile(
        &sourceBuffer,                     // Source buffer.
        compileArgs.data(),                // Array of pointers to arguments.
        static_cast<UINT32>( compileArgs.size() ),                // Number of arguments.
        m_includeHandler.Get(),            // User-provided interface to handle #include directives (optional).
        IID_PPV_ARGS( &compilationResult ) // Compiler output status, buffer, and errors.
    );
    ComPtr<IDxcBlob> compiledBytecodeBlob;
    ComPtr<IDxcBlobUtf16> shaderName = nullptr;
    compilationResult->GetOutput( DXC_OUT_OBJECT, IID_PPV_ARGS( &compiledBytecodeBlob ), &shaderName );
   
    // Print errors if present
    ComPtr<IDxcBlobUtf8> pErrors = nullptr;
    compilationResult->GetOutput( DXC_OUT_ERRORS, IID_PPV_ARGS( &pErrors ), nullptr );
    // Note that d3dcompiler would return null if no errors or warnings are present.
    // IDxcCompiler3::Compile will always return an error buffer, but its length
    // will be zero if there are no warnings or errors.
    if ( pErrors != nullptr && pErrors->GetStringLength() != 0 )
    {
        OutputDebugStringA( pErrors->GetStringPointer() );
    }

    // Save pdb.
    ComPtr<IDxcBlob> pdbBlob = nullptr;
    ComPtr<IDxcBlobUtf16> pdbName = nullptr;
    HRESULT hr = compilationResult->GetOutput( DXC_OUT_PDB, IID_PPV_ARGS( &pdbBlob ), &pdbName );
    if ( pdbBlob && pdbName )
    {
        FILE* fp = NULL;

        // Note that if you don't specify -Fd, a pdb name will be automatically generated. Use this file name to save the pdb so that PIX can find it quickly.
        _wfopen_s( &fp, pdbName->GetStringPointer(), L"wb" );
        if ( fp )
        {
            fwrite( pdbBlob->GetBufferPointer(), pdbBlob->GetBufferSize(), 1, fp );
            fclose( fp );
        }
    }

    m_shaders[ getShaderId( params ) ] = compiledBytecodeBlob;
    CD3DX12_SHADER_BYTECODE shaderBytecode( m_shaders[ getShaderId( params ) ]->GetBufferPointer(), m_shaders[ getShaderId( params ) ]->GetBufferSize() );
    return shaderBytecode;
}

std::string ShaderManager::getShaderId( ShaderDesc params )
{
    std::wstring s = params.m_filename + L"/" + params.m_entryPoint + L"/" + shaderTypeToTargetString(params.m_shaderType);

    return utf8_encode(s);
}

LPCWSTR ShaderManager::shaderTypeToTargetString( ShaderType type )
{
    switch ( type )
    {
        case ShaderType::VertexShader:
            return L"vs_6_0";
        case ShaderType::PixelShader:
            return L"ps_6_0";
        case ShaderType::ComputeShader:
            return L"cs_6_0";
        default:
            return L"";
    }
}