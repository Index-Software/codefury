//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef SHADER_TYPES_H
#define SHADER_TYPES_H

#pragma once

#include "Matrix4.h"
#include "Vector4.h"
#include "renderer/Renderer.h"
#include "shaders/ShaderBindings.h"

// CF_BINDING_SCENE
struct CfScene_t
{
	CMatrix4 m_LightSpaceMatrix;
	CVector4 m_ShadowMapParams;
	CVector4 m_ShadowBiasParams;
	CVector4 m_Time;
	CVector4 m_ShadowLightDir;
	CVector4 m_ShadowProjX;
	CVector4 m_ShadowProjY;
	CVector4 m_ShadowProjZ;
}; // 176 bytes

// CF_BINDING_LIGHTING
struct CfLighting_t
{
	CVector4 m_LightData[CF_MAX_LIGHTS * 2];
	float32 m_NumLights;
	float32 m_IsAddPass;
	float32 m_IBLLuminance;
	float32 m_CurrentLightIndex;
	// Irradiance SH as L0/L1 (g_SHA*) and L2 (g_SHB*, g_SHC) vectors
	CVector4 m_SHAr;
	CVector4 m_SHAg;
	CVector4 m_SHAb;
	CVector4 m_SHBr;
	CVector4 m_SHBg;
	CVector4 m_SHBb;
	CVector4 m_SHC;
}; // 384 bytes

// CF_BINDING_CAMERA
struct CfCamera_t
{
	CVector4 m_Pos;
	CVector4 m_Right;
	CVector4 m_Up;
	CVector4 m_Forward;
	float32 m_TanHalfFOV;
	float32 m_AspectRatio;
	float32 m_Padding[2];
}; // 80 bytes

// CF_BINDING_MODEL
struct CfModel_t
{
	CMatrix4 m_MVP;
	CMatrix4 m_Model;
}; // 128 bytes

class CShaderLayouts
{
public:
	static UniformBlockLayout_t GetSceneLayout()
	{
		UniformBlockLayout_t layout;
		layout.m_UniformNames.AddToTail("g_LightSpaceMatrix");
		layout.m_UniformNames.AddToTail("g_ShadowMapParams");
		layout.m_UniformNames.AddToTail("g_ShadowBiasParams");
		layout.m_UniformNames.AddToTail("g_Time");
		layout.m_UniformNames.AddToTail("g_ShadowLightDir");
		layout.m_UniformNames.AddToTail("g_ShadowProjX");
		layout.m_UniformNames.AddToTail("g_ShadowProjY");
		layout.m_UniformNames.AddToTail("g_ShadowProjZ");
		layout.m_UniformOffsets.AddToTail(0);
		layout.m_UniformOffsets.AddToTail(64);
		layout.m_UniformOffsets.AddToTail(80);
		layout.m_UniformOffsets.AddToTail(96);
		layout.m_UniformOffsets.AddToTail(112);
		layout.m_UniformOffsets.AddToTail(128);
		layout.m_UniformOffsets.AddToTail(144);
		layout.m_UniformOffsets.AddToTail(160);
		layout.m_UniformSizes.AddToTail(64);
		layout.m_UniformSizes.AddToTail(16);
		layout.m_UniformSizes.AddToTail(16);
		layout.m_UniformSizes.AddToTail(16);
		layout.m_UniformSizes.AddToTail(16);
		layout.m_UniformSizes.AddToTail(16);
		layout.m_UniformSizes.AddToTail(16);
		layout.m_UniformSizes.AddToTail(16);
		layout.m_Binding = CF_BINDING_SCENE;
		layout.m_Size = sizeof(CfScene_t);

		return layout;
	}

	static UniformBlockLayout_t GetLightLayout()
	{
		UniformBlockLayout_t layout;
		layout.m_UniformNames.AddToTail("g_LightData");
		layout.m_UniformOffsets.AddToTail(0);
		layout.m_UniformSizes.AddToTail(CF_MAX_LIGHTS * 2 * sizeof(CVector4));
		layout.m_UniformNames.AddToTail("g_NumLights");
		layout.m_UniformOffsets.AddToTail(256);
		layout.m_UniformSizes.AddToTail(4);
		layout.m_UniformNames.AddToTail("g_IsAddPass");
		layout.m_UniformOffsets.AddToTail(260);
		layout.m_UniformSizes.AddToTail(4);
		layout.m_UniformNames.AddToTail("g_IBLLuminance");
		layout.m_UniformOffsets.AddToTail(264);
		layout.m_UniformSizes.AddToTail(4);
		static const char* s_SHNames[7] = {
			"g_SHAr",
			"g_SHAg",
			"g_SHAb",
			"g_SHBr",
			"g_SHBg",
			"g_SHBb",
			"g_SHC"
		};
		for (uint32 i = 0; i < 7; i++)
		{
			layout.m_UniformNames.AddToTail(s_SHNames[i]);
			layout.m_UniformOffsets.AddToTail(272 + i * sizeof(CVector4));
			layout.m_UniformSizes.AddToTail(sizeof(CVector4));
		}
		layout.m_Binding = CF_BINDING_LIGHTING;
		layout.m_Size = sizeof(CfLighting_t);

		return layout;
	}

	static UniformBlockLayout_t GetCameraLayout()
	{
		UniformBlockLayout_t layout;
		layout.m_UniformNames.AddToTail("g_CameraPos");
		layout.m_UniformNames.AddToTail("g_CameraRight");
		layout.m_UniformNames.AddToTail("g_CameraUp");
		layout.m_UniformNames.AddToTail("g_CameraForward");
		layout.m_UniformNames.AddToTail("g_TanHalfFOV");
		layout.m_UniformNames.AddToTail("g_AspectRatio");
		layout.m_UniformOffsets.AddToTail(0);
		layout.m_UniformOffsets.AddToTail(16);
		layout.m_UniformOffsets.AddToTail(32);
		layout.m_UniformOffsets.AddToTail(48);
		layout.m_UniformOffsets.AddToTail(64);
		layout.m_UniformOffsets.AddToTail(68);
		layout.m_UniformSizes.AddToTail(16);
		layout.m_UniformSizes.AddToTail(16);
		layout.m_UniformSizes.AddToTail(16);
		layout.m_UniformSizes.AddToTail(16);
		layout.m_UniformSizes.AddToTail(4);
		layout.m_UniformSizes.AddToTail(4);
		layout.m_Binding = CF_BINDING_CAMERA;
		layout.m_Size = sizeof(CfCamera_t);

		return layout;
	}

	static UniformBlockLayout_t GetModelLayout()
	{
		UniformBlockLayout_t layout;
		layout.m_UniformNames.AddToTail("g_MVP");
		layout.m_UniformNames.AddToTail("g_Model");
		layout.m_UniformOffsets.AddToTail(0);
		layout.m_UniformOffsets.AddToTail(64);
		layout.m_UniformSizes.AddToTail(64);
		layout.m_UniformSizes.AddToTail(64);
		layout.m_Binding = CF_BINDING_MODEL;
		layout.m_Size = sizeof(CfModel_t);

		return layout;
	}

};

#endif // SHADER_TYPES_H