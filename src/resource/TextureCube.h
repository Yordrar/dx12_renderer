#pragma once

#include <string>
#include <vector>

#include <resource/IBindable.h>

class TextureCube : public IBindable
{
public:
	TextureCube();
	~TextureCube();

	virtual void bind( Renderer::RenderContext& context ) override;

private:
	std::vector<unsigned char*> textures;
	UINT m_slot;
};

