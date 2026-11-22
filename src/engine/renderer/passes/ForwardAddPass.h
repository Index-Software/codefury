//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef FORWARDADDPASS_H
#define FORWARDADDPASS_H

#pragma once

#include "renderer/RenderQueue.h"

class CForwardAddPass : public IRenderPass
{
public:
	virtual void Render(
		RenderPassContext_t& context,
		CRenderQueueManager* pManager) CF_OVERRIDE;
};

#endif // FORWARDADDPASS_H