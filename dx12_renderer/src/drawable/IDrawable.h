#pragma once

#include <vector>

#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <Renderer.h>

#include <bindable/IBindable.h>
#include <resource/IResource.h>
#include <resource/ConstantBuffer.h>

class IDrawable
{
public:
	IDrawable();
	virtual ~IDrawable() = 0;

	virtual void addBindable( IBindable* bindable );
	virtual void deleteBindable( IBindable* bindable );

	virtual void addResource( IResource* resource );
	virtual void deleteResource( IResource* resource );

	virtual void draw( Renderer::RenderContext& context );

private:
	std::vector<IBindable*> m_bindables;
	std::vector<IResource*> m_resources;
	ComPtr<ID3D12PipelineState> m_pipelineStateObject;
	std::vector<UINT> m_bindlessIndices;
	ConstantBuffer* m_bindlessIndicesBuffer;
};