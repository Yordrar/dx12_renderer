#include "IDrawable.h"

#include <algorithm>
#include <cassert>

#include <Renderer.h>
#include <bindable/IndexBuffer.h>

IDrawable::IDrawable()
{
}

IDrawable::~IDrawable()
{
	for (IBindable* bindable : m_bindables)
	{
		delete bindable;
	}
}

void IDrawable::addBindable(IBindable* bindable)
{
	m_bindables.push_back(bindable);
}

void IDrawable::deleteBindable( IBindable* bindable )
{
	m_bindables.erase( std::remove_if( m_bindables.begin(), m_bindables.end(), [bindable] ( IBindable* b )
									   {
										   if ( b == bindable )
										   {
											   delete b;
											   return true;
										   }
										   return false;
									   } ), m_bindables.end() );
}

void IDrawable::draw( Renderer::RenderContext& context )
{
	UINT numIndicesToDraw = -1;
	for (IBindable* bindable : m_bindables)
	{
		bindable->bind( context );

		IndexBuffer* indexBuffer = dynamic_cast<IndexBuffer*>( bindable );
		if ( indexBuffer )
		{
			numIndicesToDraw = indexBuffer->getIndexCount();
		}
	}
	assert( numIndicesToDraw != -1 );

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof( Renderer::PipelineStateStream ), &context.m_pipelineState
	};
	Renderer::device()->CreatePipelineState( &pipelineStateStreamDesc, IID_PPV_ARGS( m_pipelineStateObject.GetAddressOf() ) );

	context.m_commandList->SetPipelineState( m_pipelineStateObject.Get() );

	context.m_commandList->DrawIndexedInstanced( numIndicesToDraw, 1, 0, 0, 0 );
}
