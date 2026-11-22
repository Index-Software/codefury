//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef LIGHT_H
#define LIGHT_H

#pragma once

#include "ecs/EntityComponent.h"
#include "mathsfury/Vector3.h"
#include "utils/Color.h"
#include "WrenBind.h"

struct LightType_t
{
	enum Enum
	{
		Point = 0,
		Directional = 1
	};
};

class CLight : public CEntityComponent
{
public:
	CLight() WREN_CONSTRUCTOR("init new()") :
		m_Color(1.0f, 1.0f, 1.0f),
		m_Position(0.0f, 0.0f, 0.0f),
		m_Direction(0.0f, -1.0f, 0.0f),
		m_Intensity(1.0f),
		m_Radius(10.0f),
		m_ShadowCenterOffset(30.0f),
		m_ShadowDistance(100.0f),
		m_ShadowOrthoSize(100.0f),
		m_ShadowNearPlane(-100.0f),
		m_ShadowFarPlane(300.0f),
		m_ShadowResolution(4096.0f),
		m_ShadowStrength(1.0f),
		m_ShadowBias(0.001f),
		m_Type(LightType_t::Point)
	{
	}

	virtual ~CLight() CF_OVERRIDE
	{
	}

	virtual const char* TypeName() const CF_OVERRIDE
	{
		return "CLight";
	}

	CColor m_Color;
	CVector3 m_Position;
	CVector3 m_Direction;
	float32 m_Intensity;
	float32 m_Radius;
	float32 m_ShadowCenterOffset;
	float32 m_ShadowDistance;
	float32 m_ShadowOrthoSize;
	float32 m_ShadowNearPlane;
	float32 m_ShadowFarPlane;
	float32 m_ShadowResolution;
	float32 m_ShadowStrength;
	float32 m_ShadowBias;
	LightType_t::Enum m_Type;

	void SetColor(const CColor& v) WREN_BIND("setColor(_)")
	{
		m_Color = v;
	}

	CColor GetColor() const WREN_BIND("getColor")
	{
		return m_Color;
	}

	void SetPosition(const CVector3& v) WREN_BIND("setPosition(_)")
	{
		m_Position = v;
	}

	const CVector3& GetPosition() const WREN_BIND("getPosition")
	{
		return m_Position;
	}

	void SetDirection(const CVector3& v) WREN_BIND("setDirection(_)")
	{
		m_Direction = v;
	}

	const CVector3& GetDirection() const WREN_BIND("getDirection")
	{
		return m_Direction;
	}

	void SetIntensity(float32 v) WREN_BIND("setIntensity(_)")
	{
		m_Intensity = v;
	}

	float32 GetIntensity() const WREN_BIND("getIntensity")
	{
		return m_Intensity;
	}

	void SetRadius(float32 v) WREN_BIND("setRadius(_)")
	{
		m_Radius = v;
	}

	float32 GetRadius() const WREN_BIND("getRadius")
	{
		return m_Radius;
	}

	void SetShadowCenterOffset(float32 v) WREN_BIND("setShadowCenterOffset(_)")
	{
		m_ShadowCenterOffset = v;
	}

	float32 GetShadowCenterOffset() const WREN_BIND("getShadowCenterOffset")
	{
		return m_ShadowCenterOffset;
	}

	void SetShadowDistance(float32 v) WREN_BIND("setShadowDistance(_)")
	{
		m_ShadowDistance = v;
	}

	float32 GetShadowDistance() const WREN_BIND("getShadowDistance")
	{
		return m_ShadowDistance;
	}

	void SetShadowOrthoSize(float32 v) WREN_BIND("setShadowOrthoSize(_)")
	{
		m_ShadowOrthoSize = v;
	}

	float32 GetShadowOrthoSize() const WREN_BIND("getShadowOrthoSize")
	{
		return m_ShadowOrthoSize;
	}

	void SetShadowNearPlane(float32 v) WREN_BIND("setShadowNearPlane(_)")
	{
		m_ShadowNearPlane = v;
	}

	float32 GetShadowNearPlane() const WREN_BIND("getShadowNearPlane")
	{
		return m_ShadowNearPlane;
	}

	void SetShadowFarPlane(float32 v) WREN_BIND("setShadowFarPlane(_)")
	{
		m_ShadowFarPlane = v;
	}

	float32 GetShadowFarPlane() const WREN_BIND("getShadowFarPlane")
	{
		return m_ShadowFarPlane;
	}

	void SetShadowResolution(float32 v) WREN_BIND("setShadowResolution(_)")
	{
		m_ShadowResolution = v;
	}

	float32 GetShadowResolution() const WREN_BIND("getShadowResolution")
	{
		return m_ShadowResolution;
	}

	void SetShadowStrength(float32 v) WREN_BIND("setShadowStrength(_)")
	{
		m_ShadowStrength = v;
	}

	float32 GetShadowStrength() const WREN_BIND("getShadowStrength")
	{
		return m_ShadowStrength;
	}

	void SetShadowBias(float32 v) WREN_BIND("setShadowBias(_)")
	{
		m_ShadowBias = v;
	}

	float32 GetShadowBias() const WREN_BIND("getShadowBias")
	{
		return m_ShadowBias;
	}

	void SetType(LightType_t::Enum v) WREN_BIND("setType(_)")
	{
		m_Type = v;
	}

	LightType_t::Enum GetType() const WREN_BIND("getType")
	{
		return m_Type;
	}
};

#endif // LIGHT_H