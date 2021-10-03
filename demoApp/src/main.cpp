// Windows headers
#include <Windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;

// DirectX 12 headers
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
// D3D12 extension library
#include <d3dx12.h>

#include <assimp/Importer.hpp>
#include <Renderer.h>
#include <Scene.h>

LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
        case WM_PAINT:
            break; 
        break;
        // The default window procedure will play a system notification sound 
        // when pressing the Alt+Enter keyboard combination if this message is 
        // not handled.
        case WM_SYSCHAR:
            break;
        case WM_SIZE:
        break;
        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;
        default:
            return DefWindowProcW( hwnd, message, wParam, lParam );
    }

    return 0;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow )
{
    static auto windowClassName = L"DemoWndClass";

    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof( WNDCLASSEX );
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = ::LoadIcon( hInstance, MAKEINTRESOURCE( 32512 ) );
    windowClass.hCursor = ::LoadCursor( NULL, IDC_ARROW );
    windowClass.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = windowClassName;
    windowClass.hIconSm = ::LoadIcon( hInstance, MAKEINTRESOURCE( 32512 ) );

    static ATOM atom = ::RegisterClassExW( &windowClass );
    assert( atom > 0 );

    int screenWidth = ::GetSystemMetrics( SM_CXSCREEN );
    int screenHeight = ::GetSystemMetrics( SM_CYSCREEN );

    RECT windowRect = { 0, 0, static_cast<LONG>( 1280 ), static_cast<LONG>( 720 ) };
    AdjustWindowRect( &windowRect, WS_OVERLAPPEDWINDOW, FALSE );

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    // Center the window within the screen. Clamp to 0, 0 for the top-left corner.
    int windowX = std::max<int>( 0, ( screenWidth - windowWidth ) / 2 );
    int windowY = std::max<int>( 0, ( screenHeight - windowHeight ) / 2 );

    HWND hWnd = CreateWindowExW(
        NULL,
        windowClassName,
        L"DX12 Renderer Demo",
        WS_OVERLAPPEDWINDOW,
        windowX,
        windowY,
        windowWidth,
        windowHeight,
        NULL,
        NULL,
        hInstance,
        nullptr
    );
    assert( hWnd );
    ShowWindow( hWnd, SW_SHOW );

    Renderer renderer( hWnd, windowRect );
    Scene scene;

    MSG msg = {};
    while ( msg.message != WM_QUIT )
    {
        if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        renderer.renderScene(scene);
    }
}