#pragma once

// Windows
#include <Windows.h>
#include <Windowsx.h>
#include <wrl.h>
using namespace Microsoft::WRL;

// DirectX 12
#include <d3d12.h>
#include <d3dx12/d3dx12.h>
#include <dxgi1_6.h>

// Utilities
#include <chrono>
#include <functional>
#include <memory>

// Numeric
#include <limits>

// Strings
#include <string>
#include <cctype>
#include <cwctype>

// Containers
#include <vector>
#include <array>
#include <queue>
#include <unordered_map>

// Algorithms
#include <algorithm>

// Regex
#include <regex>

// Input/Output
#include <filesystem>

// Atomic operations
#include <atomic>

// Threads
#include <mutex>

// External Libraries
#if defined(RENDERER)
#include <dxcapi.h>
#include <pix3.h>
#include <meshoptimizer.h>
#endif