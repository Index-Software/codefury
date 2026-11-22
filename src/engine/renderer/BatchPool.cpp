//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "renderer/BatchPool.h"
#include "utils/Time.h"

static const uint64 s_UnusedFrame = ~uint64(0);

static BufferHandle CreateQuadIndexBuffer(
	IRenderer* pRenderer,
	uint32 quadsPerBatch)
{
	CUtlVector<uint32> indices;
	indices.EnsureCapacity(quadsPerBatch * 6);
	for (uint32 quad = 0; quad < quadsPerBatch; quad++)
	{
		const uint32 base = quad * 4;

		indices.AddToTail(base);
		indices.AddToTail(base + 1);
		indices.AddToTail(base + 2);
		indices.AddToTail(base);
		indices.AddToTail(base + 2);
		indices.AddToTail(base + 3);
	}

	return pRenderer->CreateIndexBuffer(
		indices.Base(),
		indices.Count() * sizeof(uint32),
		IndexFormat_t::UInt32,
		BufferUsage_t::Static);
}

CBatchPool::CBatchPool() :
	m_pRenderer(CF_NULL),
	m_hQuadIndexBuffer(0),
	m_VertexBytes(0),
	m_InitialSlots(0),
	m_MaxFrameLatency(1),
	m_NextSlot(0)
{
}

CBatchPool::~CBatchPool()
{
	Shutdown();
}

bool CBatchPool::Init(
	IRenderer* pRenderer,
	uint32 vertexBytes,
	uint32 quadsPerBatch,
	uint32 initialSlots,
	uint32 maxFrameLatency)
{
	m_pRenderer = pRenderer;
	m_VertexBytes = vertexBytes;
	m_InitialSlots = (initialSlots == 0) ? 1 : initialSlots;
	m_MaxFrameLatency = (maxFrameLatency == 0) ? 1 : maxFrameLatency;
	m_NextSlot = 0;

	m_hQuadIndexBuffer = CreateQuadIndexBuffer(
		m_pRenderer,
		quadsPerBatch);
	if (!m_hQuadIndexBuffer)
	{
		return false;
	}

	return Expand();
}

void CBatchPool::Shutdown()
{
	if (m_pRenderer)
	{
		for (int32 i = 0; i < m_VertexBuffers.Count(); i++)
		{
			if (m_VertexBuffers[i])
			{
				m_pRenderer->DestroyBuffer(m_VertexBuffers[i]);
			}
		}

		if (m_hQuadIndexBuffer)
		{
			m_pRenderer->DestroyBuffer(m_hQuadIndexBuffer);
		}
	}

	m_hQuadIndexBuffer = 0;
	m_VertexBuffers.RemoveAll();
	m_BufferFrames.RemoveAll();
}

bool CBatchPool::GetBatch(BufferHandle& outVertexBuffer)
{
	if (!m_pRenderer)
	{
		return false;
	}

	uint64 frame = CTime::GetFrameCount();
	int32 count = m_VertexBuffers.Count();
	int32 i = (m_NextSlot < count) ? m_NextSlot : 0;

	for (int32 step = 0; step < count; step++)
	{
		uint64 written = m_BufferFrames[i];
		if (written == s_UnusedFrame ||
			frame - written >= m_MaxFrameLatency)
		{
			m_BufferFrames[i] = frame;
			outVertexBuffer = m_VertexBuffers[i];

			m_NextSlot = i + 1;
			if (m_NextSlot >= count)
			{
				m_NextSlot = 0;
			}

			return true;
		}

		if (++i >= count)
		{
			i = 0;
		}
	}

	if (!Expand())
	{
		return false;
	}

	int32 slot = m_VertexBuffers.Count() - 1;
	m_BufferFrames[slot] = frame;
	m_NextSlot = 0;
	outVertexBuffer = m_VertexBuffers[slot];

	return true;
}

bool CBatchPool::Expand()
{
	if (!m_pRenderer)
	{
		return false;
	}

	int32 currentSize = m_VertexBuffers.Count();
	int32 newSize = (currentSize == 0)
		? static_cast<int32>(m_InitialSlots) : currentSize * 2;

	for (int32 i = currentSize; i < newSize; i++)
	{
		BufferHandle hVertexBuffer = m_pRenderer->CreateVertexBuffer(
			CF_NULL,
			m_VertexBytes,
			BufferUsage_t::Dynamic);
		if (!hVertexBuffer)
		{
			return false;
		}

		m_VertexBuffers.AddToTail(hVertexBuffer);
		m_BufferFrames.AddToTail(s_UnusedFrame);
	}

	return true;
}