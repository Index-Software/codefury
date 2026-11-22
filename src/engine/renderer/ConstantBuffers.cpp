//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "renderer/ConstantBuffers.h"
#include "renderer/RenderQueue.h"
#include "renderer/shader/ShaderTypes.h"
#include "renderer/ibl/EnvironmentLighting.h"
#include "ecs/components/Camera.h"
#include "ecs/components/Light.h"
#include "mathsfury/Maths.h"
#include "utils/Time.h"
#include "config/ConfigVar.h"
#include <math.h>

static CConfigVar s_rIBLLuminance(
	"r_ibl_luminance",
	1.0f,
	ConfigVarArchive,
	0.0f,
	16.0f);

CConstantBuffers::CConstantBuffers() :
	m_hSceneUniformLayout(0),
	m_hSceneConstantBuffer(0),
	m_hCameraUniformLayout(0),
	m_hCameraConstantBuffer(0),
	m_hModelUniformLayout(0),
	m_hModelConstantBuffer(0),
	m_hLightUniformLayout(0),
	m_hLightConstantBuffer(0)
{
}

void CConstantBuffers::Init(IRenderer& renderer)
{
	UniformBlockLayout_t sceneLayout = CShaderLayouts::GetSceneLayout();
	m_hSceneUniformLayout = renderer.CreateUniformBlockLayout(sceneLayout);
	m_hSceneConstantBuffer = renderer.CreateConstantBuffer(
		sceneLayout.m_Size,
		BufferUsage_t::Dynamic);

	UniformBlockLayout_t cameraLayout = CShaderLayouts::GetCameraLayout();
	m_hCameraUniformLayout = renderer.CreateUniformBlockLayout(cameraLayout);
	m_hCameraConstantBuffer = renderer.CreateConstantBuffer(
		cameraLayout.m_Size,
		BufferUsage_t::Dynamic);

	UniformBlockLayout_t modelLayout = CShaderLayouts::GetModelLayout();
	m_hModelUniformLayout = renderer.CreateUniformBlockLayout(modelLayout);
	m_hModelConstantBuffer = renderer.CreateConstantBuffer(
		modelLayout.m_Size,
		BufferUsage_t::Dynamic);

	UniformBlockLayout_t lightLayout = CShaderLayouts::GetLightLayout();
	m_hLightUniformLayout = renderer.CreateUniformBlockLayout(lightLayout);
	m_hLightConstantBuffer = renderer.CreateConstantBuffer(
		lightLayout.m_Size,
		BufferUsage_t::Dynamic);
}

void CConstantBuffers::UpdateScene(
	IRenderer& renderer,
	const CMatrix4& lightSpaceMatrix,
	const Light_t* pDirLight,
	float32 shadowMapResolution)
{
	if (m_hSceneConstantBuffer == 0)
	{
		return;
	}

	CfScene_t sceneData;
	sceneData.m_LightSpaceMatrix = lightSpaceMatrix;

	float32 frustumWidth = pDirLight
		? (pDirLight->m_ShadowOrthoSize * 2.0f)
		: 80.0f;
	float32 depthRange = pDirLight
		? (pDirLight->m_ShadowFarPlane - pDirLight->m_ShadowNearPlane)
		: 100.0f;
	float32 resolution = shadowMapResolution;
	float32 texelWidth = frustumWidth / resolution;
	float32 baseBias = texelWidth / depthRange;
	float32 shadowBias = pDirLight ? pDirLight->m_ShadowBias : 0.02f;
	float32 shadowStrength = pDirLight ? pDirLight->m_ShadowStrength : 1.0f;

	sceneData.m_ShadowMapParams = CVector4(
		1.0f / resolution,
		1.0f / resolution,
		resolution,
		resolution);
	sceneData.m_ShadowBiasParams = CVector4(
		baseBias,
		shadowBias,
		shadowStrength,
		0.0f);

	float32 time = float32(CTime::GetTime());
	sceneData.m_Time = CVector4(
		time,
		sinf(time),
		cosf(time),
		0.0f);

	sceneData.m_ShadowLightDir = pDirLight
		? CVector4(
			-pDirLight->m_Direction.m_X,
			-pDirLight->m_Direction.m_Y,
			-pDirLight->m_Direction.m_Z,
			0.0f)
		: CVector4(0.0f, 0.0f, 0.0f, 0.0f);

	const float32* pLightSpace = lightSpaceMatrix.m_Data;
	sceneData.m_ShadowProjX = CVector4(
		0.5f * (pLightSpace[0] + pLightSpace[3]),
		0.5f * (pLightSpace[4] + pLightSpace[7]),
		0.5f * (pLightSpace[8] + pLightSpace[11]),
		0.5f * (pLightSpace[12] + pLightSpace[15]));
	sceneData.m_ShadowProjY = CVector4(
		0.5f * (pLightSpace[3] - pLightSpace[1]),
		0.5f * (pLightSpace[7] - pLightSpace[5]),
		0.5f * (pLightSpace[11] - pLightSpace[9]),
		0.5f * (pLightSpace[15] - pLightSpace[13]));
	sceneData.m_ShadowProjZ = CVector4(
		0.5f * (pLightSpace[2] + pLightSpace[3]),
		0.5f * (pLightSpace[6] + pLightSpace[7]),
		0.5f * (pLightSpace[10] + pLightSpace[11]),
		0.5f * (pLightSpace[14] + pLightSpace[15]));

	renderer.UpdateBuffer(
		m_hSceneConstantBuffer,
		&sceneData,
		sizeof(CfScene_t),
		0);
}

void CConstantBuffers::UpdateLighting(
	IRenderer& renderer,
	const LightContext_t& lights,
	const CEnvironmentLighting& environmentLighting,
	bool isAddPass,
	int32 currentLightIndex)
{
	if (m_hLightConstantBuffer == 0)
	{
		return;
	}

	CfLighting_t lightingData;
	for (int32 i = 0; i < CF_MAX_LIGHTS; i++)
	{
		if (i < lights.m_Count)
		{
			const Light_t& light = lights.m_Lights[i];
			if (light.m_Type == LightType_t::Directional)
			{
				lightingData.m_LightData[i * 2] = CVector4(
					-light.m_Direction.m_X,
					-light.m_Direction.m_Y,
					-light.m_Direction.m_Z,
					light.m_Intensity);
				lightingData.m_LightData[i * 2 + 1] = CVector4(
					light.m_Color.m_R,
					light.m_Color.m_G,
					light.m_Color.m_B,
					-1.0f);
			}
			else
			{
				lightingData.m_LightData[i * 2] = CVector4(
					light.m_Position.m_X,
					light.m_Position.m_Y,
					light.m_Position.m_Z,
					light.m_Intensity);
				lightingData.m_LightData[i * 2 + 1] = CVector4(
					light.m_Color.m_R,
					light.m_Color.m_G,
					light.m_Color.m_B,
					light.m_Radius);
			}
		}
		else
		{
			lightingData.m_LightData[i * 2] = CVector4(0.0f, 0.0f, 0.0f, 0.0f);
			lightingData.m_LightData[i * 2 + 1] = CVector4(
				0.0f,
				0.0f,
				0.0f,
				0.0f);
		}
	}
	lightingData.m_NumLights = float32(lights.m_Count);
	lightingData.m_IsAddPass = isAddPass ? 1.0f : 0.0f;
	lightingData.m_IBLLuminance = s_rIBLLuminance.GetFloat();
	lightingData.m_CurrentLightIndex = float32(currentLightIndex);

	const CVector3* pIrradianceSH = environmentLighting.GetIrradianceSH();
	CVector3 sh[CF_IBL_SH_COUNT];
	for (int32 i = 0; i < CF_IBL_SH_COUNT; i++)
	{
		sh[i] = pIrradianceSH[i];
	}

	// ZH3 divides by the L0 value and the L1 length of each channel
	const float32 epsilon = 1e-5f;
	for (int32 c = 0; c < 3; c++)
	{
		if (sh[0][c] < epsilon)
		{
			sh[0][c] = epsilon;
		}

		const float32 l1LengthSq =
			sh[1][c] * sh[1][c] +
			sh[2][c] * sh[2][c] +
			sh[3][c] * sh[3][c];
		if (l1LengthSq < epsilon * epsilon)
		{
			sh[1][c] = -epsilon;
		}
	}

	// 1, y, z, x, yx, yz, 3zz - 1, zx, xx - yy
	CVector4* pSHA[3] =
	{
		&lightingData.m_SHAr,
		&lightingData.m_SHAg,
		&lightingData.m_SHAb
	};
	CVector4* pSHB[3] =
	{
		&lightingData.m_SHBr,
		&lightingData.m_SHBg,
		&lightingData.m_SHBb
	};
	for (int32 c = 0; c < 3; c++)
	{
		*pSHA[c] = CVector4(
			sh[3][c],
			sh[1][c],
			sh[2][c],
			sh[0][c] - sh[6][c]);
		*pSHB[c] = CVector4(
			sh[4][c],
			sh[5][c],
			3.0f * sh[6][c],
			sh[7][c]);
	}
	lightingData.m_SHC = CVector4(sh[8][0], sh[8][1], sh[8][2], 1.0f);

	renderer.UpdateBuffer(
		m_hLightConstantBuffer,
		&lightingData,
		sizeof(CfLighting_t),
		0);
}

void CConstantBuffers::UpdateCamera(IRenderer& renderer, const CCamera* pCamera)
{
	if (!pCamera || m_hCameraConstantBuffer == 0)
	{
		return;
	}

	CVector3 pos = pCamera->GetPosition();
	CVector3 target = pCamera->GetTarget();
	CVector3 forward = CMaths::Normalise(target - pos);
	CVector3 right = CMaths::Normalise(
		CMaths::Cross(forward, CVector3(0.0f, 1.0f, 0.0f)));
	CVector3 up = CMaths::Normalise(CMaths::Cross(right, forward));

	float32 tanHalfFOV = tanf(pCamera->GetFOV() * 0.5f * CMaths::Deg2Rad);
	float32 aspect = pCamera->GetAspect();

	CfCamera_t cameraData;

	cameraData.m_Pos = CVector4(pos.m_X, pos.m_Y, pos.m_Z, 1.0f);
	cameraData.m_Right = CVector4(right.m_X, right.m_Y, right.m_Z, 0.0f);
	cameraData.m_Up = CVector4(up.m_X, up.m_Y, up.m_Z, 0.0f);
	cameraData.m_Forward = CVector4(
		forward.m_X,
		forward.m_Y,
		forward.m_Z,
		0.0f);
	cameraData.m_TanHalfFOV = tanHalfFOV;
	cameraData.m_AspectRatio = aspect;

	renderer.UpdateBuffer(
		m_hCameraConstantBuffer,
		&cameraData,
		sizeof(CfCamera_t),
		0);
}

void CConstantBuffers::BindShared(IRenderer& renderer) const
{
	if (m_hSceneConstantBuffer != 0)
	{
		renderer.SetConstantBuffer(
			m_hSceneConstantBuffer,
			m_hSceneUniformLayout,
			CF_BINDING_SCENE,
			ShaderStageAll);
	}

	if (m_hCameraConstantBuffer != 0)
	{
		renderer.SetConstantBuffer(
			m_hCameraConstantBuffer,
			m_hCameraUniformLayout,
			CF_BINDING_CAMERA,
			ShaderStageAll);
	}

	if (m_hLightConstantBuffer != 0)
	{
		renderer.SetConstantBuffer(
			m_hLightConstantBuffer,
			m_hLightUniformLayout,
			CF_BINDING_LIGHTING,
			ShaderStageFragment);
	}
}

void CConstantBuffers::SetModel(
	IRenderer& renderer,
	const CMatrix4& mvp,
	const CMatrix4& model)
{
	if (m_hModelConstantBuffer == 0)
	{
		return;
	}

	CfModel_t modelData;
	modelData.m_MVP = mvp;
	modelData.m_Model = model;

	renderer.UpdateBuffer(
		m_hModelConstantBuffer,
		&modelData,
		sizeof(CfModel_t),
		0);
	renderer.SetConstantBuffer(
		m_hModelConstantBuffer,
		m_hModelUniformLayout,
		CF_BINDING_MODEL,
		ShaderStageVertex);
}