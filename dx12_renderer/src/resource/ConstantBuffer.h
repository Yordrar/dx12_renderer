#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <resource/IResource.h>

class ConstantBuffer : public IResource
{
public:
	ConstantBuffer( void* data, UINT sizeInBytes, LPCWSTR debugName = nullptr );
	~ConstantBuffer();

	UINT getSizeInBytes() const { return m_sizeInBytes; }
	UINT getAlignedSizeInBytes() const { return m_alignedSizeInBytes; }

private:
	void* m_data;
	UINT m_sizeInBytes;
	UINT m_alignedSizeInBytes;
};
