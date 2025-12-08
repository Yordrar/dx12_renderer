#include "Camera.h"

#include <resource/ResourceManager.h>

Camera::Camera( wchar_t const* name, ProjectionType projectionType)
	: m_name( name )
	, m_projectionType(projectionType)
	, m_localRightVector( Vector4( 1, 0, 0, 0 ) )
	, m_localUpVector( Vector4( 0, 1, 0, 0 ) )
	, m_localForwardVector( Vector4( 0, 0, 1, 0 ) )
{
	m_cameraBuffer = ResourceManager::it().createResource( std::wstring(m_name + L"_buffer").c_str(), CD3DX12_RESOURCE_DESC::Buffer( sizeof( m_cameraData ) ), D3D12_SUBRESOURCE_DATA{ &m_cameraData, sizeof( m_cameraData ), 0 } );
	D3D12_RESOURCE_DESC cameraBufferResourceDesc = ResourceManager::it().getResourceDesc(m_cameraBuffer);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc =
	{
		.Format = DXGI_FORMAT_R32_TYPELESS,
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
	srvDesc.Buffer.NumElements = sizeof(m_cameraData)/sizeof(UINT);
	srvDesc.Buffer.StructureByteStride = 0;
	m_cameraBufferDescriptor = ResourceManager::it().getShaderResourceView(m_cameraBuffer, srvDesc);

	recalculateCameraData();
}

void Camera::move( float delta_x, float delta_y, float delta_z )
{
	m_cameraData.m_position += Vector3(delta_x, delta_y, delta_z);

	recalculateCameraData();
}

void Camera::rotate( float delta_angles_x, float delta_angles_y )
{
	// Create rotation quaternion for x axis
	float angle_x_rad = math::Deg2Rad( delta_angles_x / 2.0f);
	Quaternion quaternion_x = Quaternion::rotation(m_localRightVector, angle_x_rad);

	// Create rotation quaternion for y axis
	float angle_y_rad = math::Deg2Rad( delta_angles_y / 2.0f);
	Quaternion quaternion_y = Quaternion::rotation(Vector3(0,1,0), angle_y_rad);

	// Combine quaternions
	Quaternion quaternion = (quaternion_x * quaternion_y);
	quaternion.normalize();

	//Apply result to basis vectors and position
	m_cameraData.m_position = quaternion.apply(m_cameraData.m_position);
	m_localForwardVector = quaternion.apply(m_localForwardVector);
	m_localRightVector = quaternion.apply(m_localRightVector);
	m_localUpVector = quaternion.apply(m_localUpVector);

	recalculateCameraData();
}

void Camera::recalculateCameraData()
{
	Matrix4 projectionMatrix;
	switch (m_projectionType)
	{
	case ProjectionType::Perspective:
		projectionMatrix = Matrix4::perspective(m_fov, m_aspectRatio, m_nearZ, m_farZ);
		break;
	case ProjectionType::Orthographic:
		projectionMatrix = Matrix4::orthographic(1280 * 4, 720 * 4, m_nearZ, m_farZ);
		break;
	default:
		assert(false);
		break;
	}

	m_cameraData.m_viewProjMatrix = projectionMatrix * Matrix4::lookAt(m_cameraData.m_position, m_localForwardVector, Vector3(0,1,0));
	m_cameraData.m_inverseViewProjMatrix = m_cameraData.m_viewProjMatrix.getInverse();

	ResourceManager::it().setResourceNeedsCopyToGPU(m_cameraBuffer);
}
