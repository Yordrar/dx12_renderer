#pragma once

#include <string>

#include <bindable/IBindable.h>

class Texture : public IBindable
{
public:
	Texture();
	~Texture();

	virtual void bind( Renderer::RenderContext& context ) override;

private:

};