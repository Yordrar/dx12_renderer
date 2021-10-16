#pragma once

#include <string>
#include <vector>

#include <resource/IResource.h>
#include <resource/Texture.h>

class TextureCube : public IResource
{
public:
	TextureCube( std::string path );
	~TextureCube();

private:
	std::vector<unsigned char*> m_textures;
};

