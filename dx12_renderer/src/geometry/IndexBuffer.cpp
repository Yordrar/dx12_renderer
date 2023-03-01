#include "IndexBuffer.h"

#include <d3dx12.h>

#include <Renderer.h>

IndexBuffer::IndexBuffer( UINT* indices, UINT count )
{
	m_indices = indices;
	m_indexCount = count;

	CD3DX12_HEAP_PROPERTIES heapProps( D3D12_HEAP_TYPE_DEFAULT );
	CD3DX12_HEAP_PROPERTIES uploadHeapProps( D3D12_HEAP_TYPE_UPLOAD );
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( count * sizeof( UINT ) );

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
	m_subResourceData.pData = indices;
	m_subResourceData.RowPitch = count * sizeof( UINT );
	m_subResourceData.SlicePitch = m_subResourceData.RowPitch;

	m_isDirty = true;
}

IndexBuffer::~IndexBuffer()
{
	delete[] m_indices;
}

void IndexBuffer::bind( ComPtr<ID3D12GraphicsCommandList> commandList )
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

	D3D12_INDEX_BUFFER_VIEW view;
	view.BufferLocation = m_bufferResource->GetGPUVirtualAddress();
	view.SizeInBytes = sizeof( UINT ) * m_indexCount;
	view.Format = DXGI_FORMAT_R32_UINT;
	commandList->IASetIndexBuffer(&view);
}
