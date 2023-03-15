#pragma once

#include <resource/Resource.h>
#include <geometry/Mesh.h>

class Camera
{
public:
	Camera( wchar_t const* name, DirectX::XMVECTOR position, DirectX::XMVECTOR focusPosition, float fov = 90.0f, float aspect_ratio = 16.0f/9.0f, float nearZ = 0.1f, float farZ = 10000.0f );
	~Camera() = default;

	void move( float delta_x, float delta_y, float delta_z );
	void rotate( float delta_angles_x, float delta_angles_y );

	bool isAABBVisible( Mesh::AABB const& aabb ) const;

	Resource const* getGPUBufferResource() const { return m_cameraBuffer; }

	struct
	{
		DirectX::XMMATRIX m_viewProjMatrix;
		DirectX::XMMATRIX m_inverseViewProjMatrix;
		DirectX::XMVECTOR m_position;
	} m_cameraData;
	
	Resource* m_cameraBuffer;

	std::wstring m_name;

	DirectX::XMVECTOR right;
	DirectX::XMVECTOR up;
	DirectX::XMVECTOR forward;
	DirectX::XMVECTOR m_focusPosition;
	float aspect_ratio;
	float fov;
	float m_nearZ;
	float m_farZ;
};

