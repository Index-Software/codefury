//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef BATCHPOOL_H
#define BATCHPOOL_H

#pragma once

#include "platform.h"
#include "renderer/Renderer.h"
#include "utils/UtlVector.h"

class CBatchPool
{
public:
	CBatchPool();
	~CBatchPool();

	bool Init(
		IRenderer* pRenderer,
		uint32 vertexBytes,
		uint32 quadsPerBatch,
		uint32 initialSlots,
		uint32 maxFrameLatency);
	void Shutdown();

	bool GetBatch(BufferHandle& outVertexBuffer);

	BufferHandle GetQuadIndexBuffer() const
	{
		return m_hQuadIndexBuffer;
	}
private:
	bool Expand();

	CUtlVector<BufferHandle> m_VertexBuffers;

	CUtlVector<uint64> m_BufferFrames;

	IRenderer* m_pRenderer;
	BufferHandle m_hQuadIndexBuffer;
	uint32 m_VertexBytes;
	uint32 m_InitialSlots;
	uint32 m_MaxFrameLatency;
	int32 m_NextSlot;
};

#endif // BATCHPOOL_H