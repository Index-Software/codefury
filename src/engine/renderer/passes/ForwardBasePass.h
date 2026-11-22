//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef FORWARDBASEPASS_H
#define FORWARDBASEPASS_H

#pragma once

#include "renderer/RenderQueue.h"

class CForwardBasePass : public IRenderPass
{
public:
	virtual void Render(
		RenderPassContext_t& context,
		CRenderQueueManager* pManager) CF_OVERRIDE;
};

#endif // FORWARDBASEPASS_H