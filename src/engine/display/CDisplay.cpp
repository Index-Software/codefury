//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "display/CDisplay.h"
#include "utils/Time.h"
#ifndef PLATFORM_PS3
#include <GLFW/glfw3.h>
#endif // !PLATFORM_PS3

#ifndef PLATFORM_PS3
static int32 s_WindowedRect[4] = { 0 };
#endif // !PLATFORM_PS3
static void* s_pNativeWindow = CF_NULL;
static float64 s_FrameDeadline = 0.0;
static int32 s_FrameLimit = 0;
static bool s_IsVSync = true;

void CDisplay::SetVSync(bool isEnabled)
{
	s_IsVSync = isEnabled;
#ifndef PLATFORM_PS3
	if (s_pNativeWindow)
	{
		glfwSwapInterval(isEnabled ? 1 : 0);
	}
#endif // !PLATFORM_PS3
}

void CDisplay::SetFrameLimit(int32 fps)
{
	s_FrameLimit = fps > 0 ? fps : 0;
}

void CDisplay::SetFullscreen(bool isFullscreen)
{
#ifndef PLATFORM_PS3
	GLFWwindow* pWindow = static_cast<GLFWwindow*>(s_pNativeWindow);
	if (!pWindow || isFullscreen == (glfwGetWindowMonitor(pWindow) != CF_NULL))
	{
		return;
	}

	if (isFullscreen)
	{
		glfwGetWindowPos(pWindow, &s_WindowedRect[0], &s_WindowedRect[1]);
		glfwGetWindowSize(pWindow, &s_WindowedRect[2], &s_WindowedRect[3]);
		GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);
		glfwSetWindowMonitor(
			pWindow,
			pMonitor,
			0,
			0,
			pMode->width,
			pMode->height,
			pMode->refreshRate);
	}
	else
	{
		glfwSetWindowMonitor(
			pWindow,
			CF_NULL,
			s_WindowedRect[0],
			s_WindowedRect[1],
			s_WindowedRect[2],
			s_WindowedRect[3],
			0);
	}

	glfwSwapInterval(s_IsVSync ? 1 : 0);
#endif // !PLATFORM_PS3
}

bool CDisplay::IsFullscreen()
{
#ifndef PLATFORM_PS3
	GLFWwindow* pWindow = static_cast<GLFWwindow*>(s_pNativeWindow);

	return pWindow && glfwGetWindowMonitor(pWindow) != CF_NULL;
#else // !PLATFORM_PS3
	return false;
#endif // PLATFORM_PS3
}

void CDisplay::SetNativeWindow(void* pWindow)
{
	s_pNativeWindow = pWindow;
#ifndef PLATFORM_PS3
	GLFWwindow* pGLFWWindow = static_cast<GLFWwindow*>(pWindow);
	if (pGLFWWindow)
	{
		glfwGetWindowPos(pGLFWWindow, &s_WindowedRect[0], &s_WindowedRect[1]);
		glfwGetWindowSize(pGLFWWindow, &s_WindowedRect[2], &s_WindowedRect[3]);
	}
#endif // !PLATFORM_PS3
}

void CDisplay::WaitForFrame()
{
#ifndef PLATFORM_PS3
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
		if (s_FrameDeadline - now > 0.002)
		{
			glfwWaitEventsTimeout(s_FrameDeadline - now - 0.001);
		}

		now = CTime::GetTimeSinceStartup();
	}
#endif // !PLATFORM_PS3
}

void CDisplay::CloseWindow()
{
#ifndef PLATFORM_PS3
	if (s_pNativeWindow)
	{
		glfwSetWindowShouldClose(
			static_cast<GLFWwindow*>(s_pNativeWindow),
			GLFW_TRUE);
	}
#endif // !PLATFORM_PS3
}

