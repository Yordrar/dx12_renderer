#include "VertexBuffer.h"

#include <Renderer.h>

template<typename vertex_t>
VertexBuffer<vertex_t>::VertexBuffer( vertex_t* vertices, UINT count )
{
	m_vertices = vertices;
	m_vertexCount = count;

	Renderer::device()->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
												 D3D12_HEAP_FLAG_NONE,
												 &CD3DX12_RESOURCE_DESC::Buffer( count * sizeof( vertex_t ) ),
												 D3D12_RESOURCE_STATE_COMMON,
												 nullptr,
												 IID_PPV_ARGS( m_bufferResource.GetAddressOf() ) );

	Renderer::device()->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
												 D3D12_HEAP_FLAG_NONE,
												 &CD3DX12_RESOURCE_DESC::Buffer( count * sizeof( vertex_t ) ),
												 D3D12_RESOURCE_STATE_GENERIC_READ,
												 nullptr,
												 IID_PPV_ARGS( m_intermediateUploadBuffer.GetAddressOf() ) );

	m_subResourceData = {};
	m_subResourceData.pData = vertices;
	m_subResourceData.RowPitch = count * sizeof( vertex_t );
	m_subResourceData.SlicePitch = m_subResourceData.RowPitch;

	m_isDirty = true;
}

template<typename vertex_t>
VertexBuffer<vertex_t>::~VertexBuffer()
{
	delete[] m_vertices;
}

template<typename vertex_t>
void VertexBuffer<vertex_t>::bind( Renderer::RenderContext& context )
{
    if ( m_isDirty )
    {
		context.m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_bufferResource.Get(),
                                                                                D3D12_RESOURCE_STATE_COMMON,
                                                                                D3D12_RESOURCE_STATE_COPY_DEST ) );
        UpdateSubresources<1>( context.m_commandList.Get(), m_bufferResource.Get(), m_intermediateUploadBuffer.Get(), 0, 0, 1, &m_subResourceData );
		context.m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_bufferResource.Get(),
                                                                                D3D12_RESOURCE_STATE_COPY_DEST,
                                                                                D3D12_RESOURCE_STATE_GENERIC_READ ) );
        m_isDirty = false;
    }

	D3D12_VERTEX_BUFFER_VIEW view;
	view.BufferLocation = m_bufferResource->GetGPUVirtualAddress();
	view.SizeInBytes = sizeof( vertex_t ) * m_vertexCount;
	view.StrideInBytes = sizeof( vertex_t );
	context.m_commandList->IASetVertexBuffers( 0, 1, &view );

	context.m_pipelineState.m_topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}
