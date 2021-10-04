#include "Camera.h"

Camera::Camera(DirectX::XMVECTOR position, DirectX::XMVECTOR lookat, float fov, float aspect_ratio)
	: m_position(position)
	, lookat(lookat)
	, fov(fov)
	, aspect_ratio(aspect_ratio)
{
	up = DirectX::XMVectorSet( 0, 1, 0, 0 );
	right = DirectX::XMVectorSet( 1, 0, 0, 0 );

	m_viewProjMatrix = DirectX::XMMatrixTranspose( DirectX::XMMatrixLookAtRH( m_position, lookat, up ) * DirectX::XMMatrixPerspectiveFovRH( fov, aspect_ratio, 0.1f, 500.f ) );

	m_cameraBuffer = new ConstantBuffer( 256 );
	Renderer::device()->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
												 D3D12_HEAP_FLAG_NONE,
												 &CD3DX12_RESOURCE_DESC::Buffer( 256 ),
												 D3D12_RESOURCE_STATE_GENERIC_READ,
												 nullptr,
												 IID_PPV_ARGS( m_intermediateUploadBuffer.GetAddressOf() ) );
	m_subResourceDataViewProjMatrix = {};
	m_subResourceDataViewProjMatrix.pData = &m_viewProjMatrix;
	m_subResourceDataViewProjMatrix.RowPitch = sizeof( m_viewProjMatrix );
	m_subResourceDataViewProjMatrix.SlicePitch = m_subResourceDataViewProjMatrix.RowPitch;

	m_subResourceDataPosition = {};
	m_subResourceDataPosition.pData = &m_position;
	m_subResourceDataPosition.RowPitch = sizeof( m_position );
	m_subResourceDataPosition.SlicePitch = m_subResourceDataPosition.RowPitch;
}

Camera::~Camera()
{
	delete m_cameraBuffer;
}

void Camera::move(float delta_x, float delta_y, float delta_z)
{
	m_position.m128_f32[0] += delta_x;
	m_position.m128_f32[1] += delta_y;
	m_position.m128_f32[2] += delta_z;

	m_viewProjMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(m_position, lookat, up) * DirectX::XMMatrixPerspectiveFovRH(fov, aspect_ratio, 0.1f, 500.f));
}

void Camera::rotate(float angles_x, float angles_y)
{
	// Create rotation quaternion for x axis
	float angle_x_rad = DirectX::XMConvertToRadians(angles_x / 2.0f);
	DirectX::XMFLOAT3 quaternion_x_imaginary(sinf(angle_x_rad) * right.m128_f32[0], sinf(angle_x_rad) * right.m128_f32[1], sinf(angle_x_rad) * right.m128_f32[2]);
	float quaternion_x_real = cosf(angle_x_rad);
	DirectX::XMVECTOR quaternion_x = DirectX::XMVectorSet(quaternion_x_imaginary.x, quaternion_x_imaginary.y, quaternion_x_imaginary.z, quaternion_x_real);

	// Create rotation quaternion for y axis
	float angle_y_rad = DirectX::XMConvertToRadians(angles_y / 2.0f);
	DirectX::XMFLOAT3 quaternion_y_imaginary(0, sinf(angle_y_rad), 0);
	float quaternion_y_real = cosf(angle_y_rad);
	DirectX::XMVECTOR quaternion_y = DirectX::XMVectorSet(quaternion_y_imaginary.x, quaternion_y_imaginary.y, quaternion_y_imaginary.z, quaternion_y_real);

	// Combine quaternions
	DirectX::XMVECTOR quaternion = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(quaternion_x, quaternion_y));

	//Apply result quaternion to camera position and right vector
	{
		DirectX::XMVECTOR intermediate_result = DirectX::XMQuaternionMultiply(quaternion, DirectX::XMVectorSet(m_position.m128_f32[0], m_position.m128_f32[1], m_position.m128_f32[2], 0));
		intermediate_result = DirectX::XMQuaternionMultiply(intermediate_result, DirectX::XMQuaternionConjugate(quaternion));
		m_position.m128_f32[0] = intermediate_result.m128_f32[0];
		m_position.m128_f32[1] = intermediate_result.m128_f32[1];
		m_position.m128_f32[2] = intermediate_result.m128_f32[2];
	}
	{
		DirectX::XMVECTOR intermediate_result = DirectX::XMQuaternionMultiply(quaternion_y, DirectX::XMVectorSet(right.m128_f32[0], right.m128_f32[1], right.m128_f32[2], 0));
		intermediate_result = DirectX::XMQuaternionMultiply(intermediate_result, DirectX::XMQuaternionConjugate(quaternion_y));
		right.m128_f32[0] = intermediate_result.m128_f32[0];
		right.m128_f32[1] = intermediate_result.m128_f32[1];
		right.m128_f32[2] = intermediate_result.m128_f32[2];
	}

	// Update camera lookat and up vectors
	lookat = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMVectorSet(0, 0, 0, 2), m_position));
	lookat.m128_f32[3] = 1;
	up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(lookat, DirectX::XMVectorNegate(right)));
	up.m128_f32[3] = 1;

	m_viewProjMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(m_position, lookat, up) * DirectX::XMMatrixPerspectiveFovRH(fov, aspect_ratio, 0.1f, 500.f));
}

void Camera::updateCameraBuffers(Renderer::RenderContext& context)
{
	context.m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_cameraBuffer->getResource().Get(),
																					  D3D12_RESOURCE_STATE_COMMON,
																					  D3D12_RESOURCE_STATE_COPY_DEST ) );
	UpdateSubresources( context.m_commandList.Get(),
						m_cameraBuffer->getResource().Get(),
						m_intermediateUploadBuffer.Get(),
						0, 0, 1,
						&m_subResourceDataViewProjMatrix );
	UpdateSubresources( context.m_commandList.Get(),
						m_cameraBuffer->getResource().Get(),
						m_intermediateUploadBuffer.Get(),
						sizeof(m_viewProjMatrix), 0, 1,
						&m_subResourceDataPosition );
	context.m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_cameraBuffer->getResource().Get(),
																					  D3D12_RESOURCE_STATE_COPY_DEST,
																					  D3D12_RESOURCE_STATE_GENERIC_READ ) );
}
