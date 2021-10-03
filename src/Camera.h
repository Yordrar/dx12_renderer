#pragma once

#include <dxgi.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>

#include <resource/ConstantBuffer.h>

class Camera
{
public:
	Camera( DirectX::XMVECTOR position, DirectX::XMVECTOR lookat, float fov, float aspect_ratio);
	~Camera();

	ConstantBuffer* getCameraBuffer() const { return m_cameraBuffer; }

	void move(float delta_x, float delta_y, float delta_z);
	void rotate(float angles_x, float angles_y);

	void updateCameraBuffers();

	DirectX::XMMATRIX m_viewProjMatrix;
	DirectX::XMVECTOR m_position;
	ConstantBuffer* m_cameraBuffer;

	DirectX::XMVECTOR right;
	DirectX::XMVECTOR up;
	DirectX::XMVECTOR lookat;
	float aspect_ratio;
	float fov;
};

