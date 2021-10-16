#pragma once

#include <d3d12.h>

#include <bindable/IBindable.h>

class InputLayout : public IBindable
{
public:
	InputLayout( D3D12_INPUT_ELEMENT_DESC inputElements[], UINT count );
	~InputLayout();

	virtual void bind( Renderer::RenderContext& context ) override;

private:
	D3D12_INPUT_LAYOUT_DESC m_inputLayoutDesc;
	D3D12_INPUT_ELEMENT_DESC* m_inputElements;
};