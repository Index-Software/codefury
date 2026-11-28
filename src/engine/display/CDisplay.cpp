//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "display/CDisplay.h"
#include "input/InputActions.h"
#include "utils/Time.h"
#include "window/Window.h"

static const float64 s_FrameWaitThreshold = 0.002;
static const float64 s_FrameWaitMargin = 0.001;

static IWindow* s_pWindow = CF_NULL;
static float64 s_FrameDeadline = 0.0;
static int32 s_FrameLimit = 0;

static const char* const s_ToggleFullscreenAction = "ToggleFullscreen";

static const int32 s_KeyEnter = 257;
static const int32 s_KeyF11 = 300;

void CDisplay::Init()
{
	CInputActions::Bind(s_ToggleFullscreenAction, s_KeyF11, InputMods_t::None);
	CInputActions::Bind(
		s_ToggleFullscreenAction,
		s_KeyEnter,
		InputMods_t::Alt);
}

void CDisplay::Update()
{
	if (CInputActions::WasPressed(s_ToggleFullscreenAction))
	{
		SetFullscreen(!IsFullscreen());
	}
}

void CDisplay::SetVSync(bool isEnabled)
{
	if (s_pWindow)
	{
		s_pWindow->SetVSync(isEnabled);
	}
}

void CDisplay::SetFrameLimit(int32 fps)
{
	s_FrameLimit = fps > 0 ? fps : 0;
}

void CDisplay::SetFullscreen(bool isFullscreen)
{
	if (s_pWindow)
	{
		s_pWindow->SetFullscreen(isFullscreen);
	}
}

bool CDisplay::IsFullscreen()
{
	return s_pWindow && s_pWindow->IsFullscreen();
}

void CDisplay::SetWindow(IWindow* pWindow)
{
	s_pWindow = pWindow;
}

void CDisplay::WaitForFrame()
{
	if (s_FrameLimit <= 0)
	{
		s_FrameDeadline = CTime::GetTimeSinceStartup();

		return;
	}

	const float64 period = 1.0 / float64(s_FrameLimit);
	float64 now = CTime::GetTimeSinceStartup();
	s_FrameDeadline += period;
	if (s_FrameDeadline < now - period)
	{
		s_FrameDeadline = now;
	}

	while (now < s_FrameDeadline)
	{
		if (s_pWindow && s_FrameDeadline - now > s_FrameWaitThreshold)
		{
			s_pWindow->WaitEvents(
				s_FrameDeadline - now - s_FrameWaitMargin);
		}

		now = CTime::GetTimeSinceStartup();
	}
}

void CDisplay::CloseWindow()
{
	if (s_pWindow)
	{
		s_pWindow->RequestClose();
	}
}