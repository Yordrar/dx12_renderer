#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <Manager.h>

class Profiler : public Manager<Profiler>
{
	friend class Manager<Profiler>;
public:
	~Profiler() = default;

private:
	Profiler();

	ComPtr<ID3D12QueryHeap> m_queryHeap;
	ComPtr<ID3D12Resource> m_readBackResource;
};
