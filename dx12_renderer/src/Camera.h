#pragma once

#include <directxmath.h>

#include <resource/Resource.h>

#include <string>

class Camera
{
public:
	Camera( wchar_t const* name, DirectX::XMVECTOR position, DirectX::XMVECTOR lookat, float fov, float aspect_ratio );
	~Camera() = default;

	void move( float delta_x, float delta_y, float delta_z );
	void rotate( float delta_angles_x, float delta_angles_y );

	void setCameraBufferView( ComPtr<ID3D12GraphicsCommandList> commandList );

	Resource const* getCameraBuffer() const { return m_cameraBuffer; }

	struct
	{
		DirectX::XMMATRIX m_viewProjMatrix;
		DirectX::XMVECTOR m_position;
	} m_cameraData;
	
	Resource* m_cameraBuffer;

	std::wstring m_name;

	DirectX::XMVECTOR right;
	DirectX::XMVECTOR up;
	DirectX::XMVECTOR lookat;
	float aspect_ratio;
	float fov;
};

