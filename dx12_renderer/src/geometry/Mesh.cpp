#include "Mesh.h"

#include <BarrierRecorder.h>

std::vector<D3D12_INPUT_ELEMENT_DESC> Mesh::s_inputLayout;

Mesh::Mesh()
{

}

void Mesh::record(ComPtr<ID3D12GraphicsCommandList> commandList) const
{
    for (Submesh const& submesh : m_submeshes)
    {
        submesh.record(commandList);
    }
}

void Mesh::setFrontCounterClockwise(bool frontCounterClockwise)
{
    for (Submesh& submesh : m_submeshes)
    {
        submesh.m_rasterizerState.FrontCounterClockwise = frontCounterClockwise;
    }
}

void Mesh::Submesh::record( ComPtr<ID3D12GraphicsCommandList> commandList ) const
{
    BarrierRecorder br;
    br.recordBarrierTransition(m_vertexBuffer->getResource(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    if (m_indexBuffer)
    {
        br.recordBarrierTransition(m_indexBuffer->getResource(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
    }
    br.submitBarriers(commandList);

    commandList->IASetPrimitiveTopology( m_primitiveTopology );
    m_vertexBuffer->bind( commandList );

    if( m_indexBuffer )
    {
        m_indexBuffer->bind(commandList);
        commandList->DrawIndexedInstanced(m_indexBuffer->getIndexCount(), 1, 0, 0, 0);
    }
    else
    {
        commandList->DrawInstanced(static_cast<UINT>( m_vertexBuffer->getVertexCount() ), 1, 0, 0);
    }
}
