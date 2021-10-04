#include "InputLayout.h"

#include <Renderer.h>

InputLayout::InputLayout( D3D12_INPUT_ELEMENT_DESC inputElements[], UINT count )
{
    m_inputLayoutDesc.pInputElementDescs = inputElements;
    m_inputLayoutDesc.NumElements = count;

    m_inputElements = inputElements;
}

InputLayout::~InputLayout()
{
    //delete m_inputElements;
}

void InputLayout::bind( Renderer::RenderContext& context )
{
    context.m_pipelineState.m_inputLayout = m_inputLayoutDesc;
}
