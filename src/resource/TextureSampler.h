#pragma once

#include <resource/IBindable.h>

class TextureSampler : public IBindable
{
public:
	TextureSampler();
	~TextureSampler();

	virtual void bind( Renderer::RenderContext& context ) override;

private:

};

