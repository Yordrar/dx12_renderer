#pragma once

#include <string>

#include <resource/IResource.h>

class ConstantBuffer : public IResource
{
public:
	ConstantBuffer( std::wstring name, void* data, UINT sizeInBytes );
	~ConstantBuffer();

	UINT getSizeInBytes() const { return m_sizeInBytes; }
	UINT getAlignedSizeInBytes() const { return m_alignedSizeInBytes; }

private:
	void* m_data;
	UINT m_sizeInBytes;
	UINT m_alignedSizeInBytes;
};
