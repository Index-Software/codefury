//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef CONSTANTBUFFERS_H
#define CONSTANTBUFFERS_H

#pragma once

#include "renderer/Renderer.h"

class CCamera;
class CEnvironmentLighting;
struct CMatrix4;
struct Light_t;
struct LightContext_t;

class CConstantBuffers
{
public:
	CConstantBuffers();

	void Init(IRenderer& renderer);

	void UpdateScene(
		IRenderer& renderer,
		const CMatrix4& lightSpaceMatrix,
		const Light_t* pDirLight,
		float32 shadowMapResolution);
	void UpdateLighting(
		IRenderer& renderer,
		const LightContext_t& lights,
		const CEnvironmentLighting& environmentLighting,
		bool isAddPass,
		int32 currentLightIndex);
	void UpdateCamera(IRenderer& renderer, const CCamera* pCamera);

	void BindShared(IRenderer& renderer) const;
	void SetModel(
		IRenderer& renderer,
		const CMatrix4& mvp,
		const CMatrix4& model);
private:
	UniformBlockLayoutHandle m_hSceneUniformLayout;
	BufferHandle m_hSceneConstantBuffer;
	UniformBlockLayoutHandle m_hCameraUniformLayout;
	BufferHandle m_hCameraConstantBuffer;
	UniformBlockLayoutHandle m_hModelUniformLayout;
	BufferHandle m_hModelConstantBuffer;
	UniformBlockLayoutHandle m_hLightUniformLayout;
	BufferHandle m_hLightConstantBuffer;
};

#endif // CONSTANTBUFFERS_H