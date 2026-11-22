//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef MAINLIGHTSHADOWPASS_H
#define MAINLIGHTSHADOWPASS_H

#pragma once

#include "renderer/RenderQueue.h"

class CMainLightShadowPass : public IRenderPass
{
public:
	virtual void Render(
		RenderPassContext_t& context,
		CRenderQueueManager* pManager) CF_OVERRIDE;
};

#endif // MAINLIGHTSHADOWPASS_H