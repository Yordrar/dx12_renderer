#include "VertexBuffer.h"

#include <Renderer.h>

VertexBuffer::VertexBuffer( void* vertices, size_t vertexSize, size_t vertexCount )
	: m_vertices( new char[ vertexCount * vertexSize ] )
	, m_vertexSize( vertexSize )
	, m_vertexCount( vertexCount )
{
	memcpy( m_vertices, vertices, vertexCount * vertexSize );

	CD3DX12_HEAP_PROPERTIES heapProps( D3D12_HEAP_TYPE_DEFAULT );
	CD3DX12_HEAP_PROPERTIES uploadHeapProps( D3D12_HEAP_TYPE_UPLOAD );
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( vertexCount * vertexSize );

	Renderer::device()->CreateCommittedResource( &heapProps,
												 D3D12_HEAP_FLAG_NONE,
												 &resourceDesc,
												 D3D12_RESOURCE_STATE_COMMON,
												 nullptr,
												 IID_PPV_ARGS( m_bufferResource.GetAddressOf() ) );

	Renderer::device()->CreateCommittedResource( &uploadHeapProps,
												 D3D12_HEAP_FLAG_NONE,
												 &resourceDesc,
												 D3D12_RESOURCE_STATE_GENERIC_READ,
												 nullptr,
												 IID_PPV_ARGS( m_intermediateUploadBuffer.GetAddressOf() ) );

	m_subResourceData = {};
	m_subResourceData.pData = m_vertices;
	m_subResourceData.RowPitch = static_cast<UINT64>( vertexCount ) * static_cast<UINT64>( vertexSize );
	m_subResourceData.SlicePitch = 0;

	m_isDirty = true;
}

VertexBuffer::~VertexBuffer()
{
	delete[] m_vertices;
}

void VertexBuffer::bind( ComPtr<ID3D12GraphicsCommandList> commandList )
{
	if ( m_isDirty )
	{
		CD3DX12_RESOURCE_BARRIER copyDestBarrier = CD3DX12_RESOURCE_BARRIER::Transition( m_bufferResource.Get(),
																						 D3D12_RESOURCE_STATE_COMMON,
																						 D3D12_RESOURCE_STATE_COPY_DEST );
		CD3DX12_RESOURCE_BARRIER readBarrier = CD3DX12_RESOURCE_BARRIER::Transition( m_bufferResource.Get(),
																					 D3D12_RESOURCE_STATE_COPY_DEST,
																					 D3D12_RESOURCE_STATE_GENERIC_READ );
		commandList->ResourceBarrier( 1, &copyDestBarrier );
		UpdateSubresources<1>( commandList.Get(), m_bufferResource.Get(), m_intermediateUploadBuffer.Get(), 0, 0, 1, &m_subResourceData );
		commandList->ResourceBarrier( 1, &readBarrier );
		m_isDirty = false;
	}

	D3D12_VERTEX_BUFFER_VIEW view;
	view.BufferLocation = m_bufferResource->GetGPUVirtualAddress();
	view.SizeInBytes = static_cast<UINT>( m_vertexSize * m_vertexCount );
	view.StrideInBytes = static_cast<UINT>( m_vertexSize );
	commandList->IASetVertexBuffers( 0, 1, &view );
}