#include "IndexBuffer.h"

#include <d3dx12.h>

#include <Renderer.h>

IndexBuffer::IndexBuffer( UINT* indices, UINT count )
{
	m_indices = indices;
	m_indexCount = count;

	Renderer::device()->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
												 D3D12_HEAP_FLAG_NONE,
												 &CD3DX12_RESOURCE_DESC::Buffer( count * sizeof( UINT ) ),
												 D3D12_RESOURCE_STATE_COMMON,
												 nullptr,
												 IID_PPV_ARGS( m_bufferResource.GetAddressOf() ) );

	Renderer::device()->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
												 D3D12_HEAP_FLAG_NONE,
												 &CD3DX12_RESOURCE_DESC::Buffer( count * sizeof( UINT ) ),
												 D3D12_RESOURCE_STATE_GENERIC_READ,
												 nullptr,
												 IID_PPV_ARGS( m_intermediateUploadBuffer.GetAddressOf() ) );

	m_subResourceData = {};
	m_subResourceData.pData = indices;
	m_subResourceData.RowPitch = count * sizeof( UINT );
	m_subResourceData.SlicePitch = m_subResourceData.RowPitch;

	m_isDirty = true;
}

IndexBuffer::~IndexBuffer()
{
	delete[] m_indices;
}

void IndexBuffer::bind( Renderer::RenderContext& context )
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

	D3D12_INDEX_BUFFER_VIEW view;
	view.BufferLocation = m_bufferResource->GetGPUVirtualAddress();
	view.SizeInBytes = sizeof( UINT ) * m_indexCount;
	view.Format = DXGI_FORMAT_R32_UINT;
	context.m_commandList->IASetIndexBuffer(&view);
}
