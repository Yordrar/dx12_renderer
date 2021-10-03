#pragma once

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <Renderer.h>

class IResource
{
public:
	virtual ~IResource() = default;

	virtual void getSlot() = 0;
};