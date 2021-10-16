#include "IDrawable.h"

#include <algorithm>
#include <vector>
#include <cassert>

#include <Renderer.h>
#include <bindable/IndexBuffer.h>
#include <resource/IResource.h>

IDrawable::IDrawable()
{
	m_bindlessIndices.resize( 16 );
	m_bindlessIndicesBuffer = new ConstantBuffer( m_bindlessIndices.data(), 16 * sizeof( decltype( m_bindlessIndices )::value_type ) );
}

IDrawable::~IDrawable()
{
	for ( IBindable* bindable : m_bindables )
	{
		delete bindable;
	}
	for ( IResource* resource : m_resources )
	{
		delete resource;
	}
	delete m_bindlessIndicesBuffer;
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

void IDrawable::addResource( IResource* resource )
{
	m_resources.push_back( resource );
}

void IDrawable::deleteResource( IResource* resource )
{
	m_resources.erase( std::remove_if( m_resources.begin(), m_resources.end(), [resource] ( IResource* r )
										{
											if ( r == resource )
											{
												delete r;
												return true;
											}
											return false;
										} ), m_resources.end() );
}

void IDrawable::draw( Renderer::RenderContext& context )
{
	// Update indices buffer for bindless
	if ( m_resources.size() > 0 )
	{
		for ( int i = 0; i < m_resources.size(); i++ )
		{
			m_resources[ i ]->updateResource( context );
			m_bindlessIndices[ i ] = m_resources[ i ]->getSlot();
		}
		m_bindlessIndicesBuffer->updateResource( context );
	}
	context.m_commandList->SetGraphicsRootConstantBufferView( 1, m_bindlessIndicesBuffer->getGPUVirtualAddress() );

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
