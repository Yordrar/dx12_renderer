#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <Renderer.h>

class IBindable
{
public:
	virtual ~IBindable() = default;

	virtual void bind( Renderer::RenderContext& context ) = 0;
};