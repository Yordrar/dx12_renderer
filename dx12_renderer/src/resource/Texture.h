#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <string>

#include <resource/IResource.h>

class Texture : public IResource
{
public:
	Texture(std::string filename);
	~Texture();

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

private:
	unsigned char* m_data;
	int m_width;
	int m_height;
};