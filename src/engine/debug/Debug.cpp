//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "Debug.h"
#include "utils/Color.h"
#include "utils/Time.h"
#include "renderer/ui/UI.h"
#include <stdio.h>

CDebug::CDebug(CUI* pGUI) :
	m_pGUI(pGUI),
	m_LastFPS(0.0),
	m_SumFPS(0.0),
	m_LastFrameTime(0.0),
	m_SumFrameTime(0.0),
	m_FPS(0.0f),
	m_FPSAverage(0.0f),
	m_FrameTime(0.0f),
	m_FrameTimeAverage(16.67f),
	m_FPSSampleCount(0),
	m_FrameTimeSampleCount(0),
	m_IsEnabled(true)
{
}

CDebug::~CDebug()
{
}

bool CDebug::Init()
{
	m_LastFPS = CTime::GetTime();
	m_LastFrameTime = CTime::GetTime();

	return true;
}

void CDebug::Update()
{
	if (!m_IsEnabled)
	{
		return;
	}

	float64 currentTime = CTime::GetTime();

	if (m_LastFrameTime > 0.0)
	{
		m_FrameTime = static_cast<float32>(
			(currentTime - m_LastFrameTime) * 1000.0);
		m_SumFrameTime += m_FrameTime;
		m_FrameTimeSampleCount++;

		m_FPS = (m_FrameTime > 0.0f) ? (1000.0f / m_FrameTime) : 0.0f;

		m_SumFPS += m_FPS;
		m_FPSSampleCount++;
	}
	m_LastFrameTime = currentTime;

	if (currentTime - m_LastFPS >= 1.0)
	{
		if (m_FrameTimeSampleCount > 0)
		{
			m_FrameTimeAverage = static_cast<float32>(
				m_SumFrameTime / static_cast<float64>(m_FrameTimeSampleCount));

			m_FPSAverage = static_cast<float32>(
				static_cast<float64>(
					m_FrameTimeSampleCount) / (currentTime - m_LastFPS));
		}
		else
		{
			m_FrameTimeAverage = 0.0f;
			m_FPSAverage = 0.0f;
		}

		m_SumFrameTime = 0.0;
		m_FrameTimeSampleCount = 0;
		m_SumFPS = 0.0;
		m_FPSSampleCount = 0;
		m_LastFPS = currentTime;
	}
}

void CDebug::Render()
{
	if (!m_IsEnabled || !m_pGUI)
	{
		return;
	}

	char buf[64];
	sprintf(buf, "FPS: %.0f (%.2f ms)", m_FPSAverage, m_FrameTimeAverage);

	m_pGUI->BeginDraw();

	m_pGUI->SetDrawColor(CColor::White);
	m_pGUI->DrawTextAnchoredOutline(
		buf,
		Anchor_t::TopLeft,
		12.0f,
		12.0f,
		CColor::Black,
		0.03f);

	m_pGUI->EndDraw();
}

void CDebug::SetEnabled(bool enabled)
{
	m_IsEnabled = enabled;
}

bool CDebug::IsEnabled() const
{
	return m_IsEnabled;
}