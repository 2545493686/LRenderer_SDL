#pragma once
#define _USE_MATH_DEFINES

#include "Component.h"
#include "Transform.h"
#include "Eigen/Dense"
#include "cmath"

class Camera : public Component
{
public:
	float aspect = 1;
	float zNear = 0.1f;
	float zFar = 50;

	Transform* transform;
	explicit Camera(Transform* transform) 
	{
        this->transform = transform;
	};

	Camera(Transform* transform, float aspect) : Camera(transform)
	{
		this->aspect = aspect;
	}

	enum class Type { Perspective, Orthographic };

	virtual Type GetType() const = 0;
	virtual Eigen::Matrix4f GetViewMatrix() const = 0;
	virtual Eigen::Matrix4f GetFrustumMatrix() const = 0;
	virtual Eigen::Matrix4f GetClipToWorldMatrix() const = 0;
};

class PerspectiveCamera : public Camera
{
public:
	float fov = 45;

	explicit PerspectiveCamera(Transform* transform) : Camera(transform)
	{
	}

    PerspectiveCamera(Transform* transform, float aspect) : Camera(transform, aspect)
	{
	}

	EIGEN_ALWAYS_INLINE Type GetType() const override
	{
		return Type::Perspective;
	}

	EIGEN_ALWAYS_INLINE Eigen::Matrix4f GetViewMatrix() const override
	{
		return transform->GetModelMatrix().inverse();
	}

	// view -> clip
	// w = -z(view)
	// P79_Shader入门精要_冯乐乐 
	// LANQ 25.2.5
	EIGEN_ALWAYS_INLINE Eigen::Matrix4f GetFrustumMatrix() const override
	{
		Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

		float nSize = 1 / tan(fov / 2);

		projection << nSize / aspect, 0, 0, 0,
			0, nSize, 0, 0,
			0, 0, -(zFar + zNear) / (zFar - zNear), -2 * zFar * zNear / (zFar - zNear),
			0, 0, -1, 0;

		return projection;
	}

	EIGEN_ALWAYS_INLINE Eigen::Matrix4f GetClipToWorldMatrix() const override
	{
		auto m = GetFrustumMatrix() * GetViewMatrix();
		return m.inverse();
	}
};

class OrthographicCamera : public Camera
{
public:
	float size = 5;

	explicit OrthographicCamera(Transform* transform) : Camera(transform)
	{
	}

	OrthographicCamera(Transform* transform, float aspect) : Camera(transform, aspect)
	{
	}

	EIGEN_ALWAYS_INLINE Type GetType() const override
	{
		return Type::Orthographic;
	}

	EIGEN_ALWAYS_INLINE Eigen::Matrix4f GetViewMatrix() const override
	{
		return transform->GetModelMatrix().inverse();
	}

	// view -> clip
	// w = -z(view)
	// P79_Shader入门精要_冯乐乐 
	// LANQ 25.2.5
	EIGEN_ALWAYS_INLINE Eigen::Matrix4f GetFrustumMatrix() const override
	{
		Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

		projection <<	1 / (aspect * size),	0,			0,						0,
						0,						1 / size,	0,						0,
						//0,						0,			2 / (zFar - zNear),	(zFar + zNear) / (zFar - zNear),
						0,						0,			-2 / (zFar - zNear),	-(zFar + zNear) / (zFar - zNear),
						0,						0,			0,						1;

		return projection;
	}

	EIGEN_ALWAYS_INLINE Eigen::Matrix4f GetClipToWorldMatrix() const override
	{
		auto m = GetFrustumMatrix() * GetViewMatrix();
		return m.inverse();
	}
};
