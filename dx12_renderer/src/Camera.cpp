#include "Camera.h"

#include <resource/ResourceManager.h>

Camera::Camera( wchar_t const* name, DirectX::XMVECTOR position, DirectX::XMVECTOR focusPosition, float fov, float aspect_ratio, float nearZ, float farZ )
	: m_name( name )
	, forward( DirectX::XMVector3Normalize( DirectX::XMVectorSubtract( focusPosition, position ) ) )
	, m_focusPosition( focusPosition )
	, fov(fov)
	, aspect_ratio(aspect_ratio)
	, m_nearZ( nearZ )
	, m_farZ( farZ )
{
	up = DirectX::XMVectorSet( 0, 1, 0, 0 );
	right = DirectX::XMVectorSet( 1, 0, 0, 0 );
	
	m_cameraData.m_position = position;
	m_cameraData.m_viewProjMatrix = DirectX::XMMatrixTranspose( DirectX::XMMatrixLookAtRH( m_cameraData.m_position, m_focusPosition, up ) * DirectX::XMMatrixPerspectiveFovRH( fov, aspect_ratio, m_nearZ, m_farZ ) );
	m_cameraData.m_inverseViewProjMatrix = DirectX::XMMatrixInverse( nullptr, m_cameraData.m_viewProjMatrix );

	m_cameraBuffer = ResourceManager::it().createResource( std::wstring(m_name + L"_buffer").c_str(), CD3DX12_RESOURCE_DESC::Buffer( sizeof( m_cameraData ) ), D3D12_SUBRESOURCE_DATA{ &m_cameraData, sizeof( m_cameraData ), 0 } );
}

void Camera::move( float delta_x, float delta_y, float delta_z )
{
	m_cameraData.m_position.m128_f32[0] += delta_x;
	m_cameraData.m_position.m128_f32[1] += delta_y;
	m_cameraData.m_position.m128_f32[2] += delta_z;

	m_cameraData.m_viewProjMatrix = DirectX::XMMatrixTranspose( DirectX::XMMatrixLookAtRH( m_cameraData.m_position, m_focusPosition, up ) * DirectX::XMMatrixPerspectiveFovRH( fov, aspect_ratio, m_nearZ, m_farZ ));
	m_cameraData.m_inverseViewProjMatrix = DirectX::XMMatrixInverse( nullptr, m_cameraData.m_viewProjMatrix );

	m_cameraBuffer->setNeedsCopyToGPU( true );
}

void Camera::rotate( float delta_angles_x, float delta_angles_y )
{
	// Create rotation quaternion for x axis
	float angle_x_rad = DirectX::XMConvertToRadians( delta_angles_x / 2.0f);
	DirectX::XMFLOAT3 quaternion_x_imaginary(sinf(angle_x_rad) * right.m128_f32[0], sinf(angle_x_rad) * right.m128_f32[1], sinf(angle_x_rad) * right.m128_f32[2]);
	float quaternion_x_real = cosf(angle_x_rad);
	DirectX::XMVECTOR quaternion_x = DirectX::XMVectorSet(quaternion_x_imaginary.x, quaternion_x_imaginary.y, quaternion_x_imaginary.z, quaternion_x_real);

	// Create rotation quaternion for y axis
	float angle_y_rad = DirectX::XMConvertToRadians( delta_angles_y / 2.0f);
	DirectX::XMFLOAT3 quaternion_y_imaginary(0, sinf(angle_y_rad), 0);
	float quaternion_y_real = cosf(angle_y_rad);
	DirectX::XMVECTOR quaternion_y = DirectX::XMVectorSet(quaternion_y_imaginary.x, quaternion_y_imaginary.y, quaternion_y_imaginary.z, quaternion_y_real);

	// Combine quaternions
	DirectX::XMVECTOR quaternion = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(quaternion_x, quaternion_y));

	//Apply result quaternion to camera position and right vector
	{
		DirectX::XMVECTOR intermediate_result = DirectX::XMQuaternionMultiply( quaternion, m_cameraData.m_position );
		intermediate_result = DirectX::XMQuaternionMultiply(intermediate_result, DirectX::XMQuaternionConjugate(quaternion));
		m_cameraData.m_position = intermediate_result;
	}
	{
		DirectX::XMVECTOR intermediate_result = DirectX::XMQuaternionMultiply( quaternion_y, right );
		intermediate_result = DirectX::XMQuaternionMultiply(intermediate_result, DirectX::XMQuaternionConjugate(quaternion_y));
		right = intermediate_result;
	}

	// Update camera forward and up vectors
	forward = DirectX::XMVector3Normalize( DirectX::XMVectorSubtract( m_focusPosition, m_cameraData.m_position ) );
	forward.m128_f32[3] = 0;
	up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(forward, DirectX::XMVectorNegate(right)));
	up.m128_f32[3] = 0;

	m_cameraData.m_viewProjMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH( m_cameraData.m_position, m_focusPosition, up ) * DirectX::XMMatrixPerspectiveFovRH( fov, aspect_ratio, m_nearZ, m_farZ ));
	m_cameraData.m_inverseViewProjMatrix = DirectX::XMMatrixInverse( nullptr, m_cameraData.m_viewProjMatrix );

	m_cameraBuffer->setNeedsCopyToGPU( true );
}

bool Camera::isAABBVisible( Mesh::AABB const& aabb ) const
{
	// Very naive visibility test, just check if any one corner of the AABB is in front of the camera
	DirectX::XMVECTOR aabbCorners[ 8 ] = {
		DirectX::XMVectorSet( aabb.m_minBounds.m128_f32[ 0 ], aabb.m_minBounds.m128_f32[ 1 ], aabb.m_minBounds.m128_f32[ 2 ], 1.0 ),
		DirectX::XMVectorSet( aabb.m_maxBounds.m128_f32[ 0 ], aabb.m_minBounds.m128_f32[ 1 ], aabb.m_minBounds.m128_f32[ 2 ], 1.0 ),
		DirectX::XMVectorSet( aabb.m_minBounds.m128_f32[ 0 ], aabb.m_maxBounds.m128_f32[ 1 ], aabb.m_minBounds.m128_f32[ 2 ], 1.0 ),
		DirectX::XMVectorSet( aabb.m_maxBounds.m128_f32[ 0 ], aabb.m_maxBounds.m128_f32[ 1 ], aabb.m_minBounds.m128_f32[ 2 ], 1.0 ),
		DirectX::XMVectorSet( aabb.m_minBounds.m128_f32[ 0 ], aabb.m_minBounds.m128_f32[ 1 ], aabb.m_maxBounds.m128_f32[ 2 ], 1.0 ),
		DirectX::XMVectorSet( aabb.m_maxBounds.m128_f32[ 0 ], aabb.m_minBounds.m128_f32[ 1 ], aabb.m_maxBounds.m128_f32[ 2 ], 1.0 ),
		DirectX::XMVectorSet( aabb.m_minBounds.m128_f32[ 0 ], aabb.m_maxBounds.m128_f32[ 1 ], aabb.m_maxBounds.m128_f32[ 2 ], 1.0 ),
		DirectX::XMVectorSet( aabb.m_maxBounds.m128_f32[ 0 ], aabb.m_maxBounds.m128_f32[ 1 ], aabb.m_maxBounds.m128_f32[ 2 ], 1.0 ),
	};

	bool visible = false;
	for ( size_t i = 0; i < 8; i++ )
	{
		visible = visible || DirectX::XMVector3Dot( DirectX::XMVector3Normalize( DirectX::XMVectorSubtract( aabbCorners[i], m_cameraData.m_position ) ), forward ).m128_f32[ 0 ] > 0.0f;
	}
	return visible;
}
