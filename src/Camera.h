#pragma once

#include <dxgi.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>

#include <Renderer.h>
#include <resource/ConstantBuffer.h>

class Camera
{
public:
	Camera( DirectX::XMVECTOR position, DirectX::XMVECTOR lookat, float fov, float aspect_ratio);
	~Camera();

	ConstantBuffer* getCameraBuffer() const { return m_cameraBuffer; }

	void move(float delta_x, float delta_y, float delta_z);
	void rotate(float angles_x, float angles_y);

	void updateCameraBuffers( Renderer::RenderContext& context );

	DirectX::XMMATRIX m_viewProjMatrix;
	D3D12_SUBRESOURCE_DATA m_subResourceDataViewProjMatrix;
	DirectX::XMVECTOR m_position;
	D3D12_SUBRESOURCE_DATA m_subResourceDataPosition;

	ConstantBuffer* m_cameraBuffer;
	ComPtr<ID3D12Resource> m_intermediateUploadBuffer;

	DirectX::XMVECTOR right;
	DirectX::XMVECTOR up;
	DirectX::XMVECTOR lookat;
	float aspect_ratio;
	float fov;
};

