#pragma once

#include <core/Math.h>
#include <resource/Resource.h>
#include <geometry/Mesh.h>

class Camera
{
public:
	enum class ProjectionType
	{
		Perspective,
		Orthographic
	};

	Camera( wchar_t const* name, ProjectionType projectionType = ProjectionType::Perspective );
	~Camera() = default;

	void move( float delta_x, float delta_y, float delta_z );
	void move( Vector3 delta );
	void rotate( float delta_angles_x, float delta_angles_y );

	ResourceHandle getGPUBufferResource() const { return m_cameraBuffer; }
	Descriptor getCameraBufferDescriptor() const { return m_cameraBufferDescriptor; }

public:
	void recalculateCameraData();

	struct
	{
		Matrix4 m_viewProjMatrix;
		Matrix4 m_inverseViewProjMatrix;
		Vector4 m_position;
	} m_cameraData;
	
	ResourceHandle m_cameraBuffer;
	Descriptor m_cameraBufferDescriptor;

	std::wstring m_name;
	ProjectionType m_projectionType;
	Vector3 m_localRightVector;
	Vector3 m_localUpVector;
	Vector3 m_localForwardVector;

	// Perspective camera data
	float m_aspectRatio = 16.0f / 9.0f;
	float m_fov = 90.0f;
	float m_nearZ = 0.1f;
	float m_farZ = 10000.0f;

	// Orthographic camera data
	float width = 1.0f;
	float height = 1.0f;
};

