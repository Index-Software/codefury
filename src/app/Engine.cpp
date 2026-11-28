//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "app/Engine.h"
#include "app/App.h"
#include "audio/Audio.h"
#include "audio/AudioManager.h"
#include "config/ConfigVar.h"
#include "debug/Debug.h"
#include "ecs/EntityRegistry.h"
#include "engine/wren/WrenManager.h"
#include "input/Input.h"
#include "input/InputManager.h"
#include "renderer/RenderQueue.h"
#include "renderer/material/Material.h"
#include "renderer/Renderer.h"
#include "renderer/text/Font.h"
#include "renderer/ui/UI.h"
#include "scene/Scene.h"
#include "utils/Time.h"
#include "utils/Utils.h"
#include "WrenBind.h"
#include "window/Window.h"
#include "window/WindowManager.h"
#include "display/CDisplay.h"
#include "dbg.h"

#ifdef PLATFORM_PS3
static const WindowBackend_t s_WindowBackend = WindowBackendPS3;
#else // !PLATFORM_PS3
static const WindowBackend_t s_WindowBackend = WindowBackendGLFW;
#endif // !PLATFORM_PS3

CEngine::CEngine() :
	m_pRenderer(CF_NULL),
	m_pApp(CF_NULL)
{
}

CEngine::~CEngine()
{
	if (m_pApp)
	{
		m_pApp->Shutdown();
		DestroyApp(m_pApp);
		m_pApp = CF_NULL;
	}

	m_pWrenManager.Destroy();
	m_pRenderQueue.Destroy();
	m_pScene.Destroy();
	m_pRegistry.Destroy();
	ReleaseDefaultMaterial();
	m_pDebug.Destroy();
	m_pGUI.Destroy();
	m_pAudioManager.Destroy();
	m_pInputManager.Destroy();

	CConfigVarManager::Save(g_ConfigFilePath);

	if (m_pRenderer)
	{
		m_pRenderer->Shutdown();
		DestroyRenderer(m_pRenderer);
		m_pRenderer = CF_NULL;
	}

	m_pWindowManager.Destroy();
}

bool CEngine::Init()
{
#ifdef PLATFORM_PS3
	CUtils::SetContentRoot("/dev_hdd0/game/CODF00001/USRDIR/");
#endif // PLATFORM_PS3

	CConfigVarManager::Load(g_ConfigFilePath);

	m_pApp = CreateApp();
	if (!m_pApp)
	{
		Error("[CEngine] Failed to create app\n");

		return false;
	}

	WindowConfig_t windowConfig;
	windowConfig.m_Width = 1280;
	windowConfig.m_Height = 720;
	windowConfig.m_Title = "codefury";
	m_pApp->GetWindowConfig(windowConfig);
	windowConfig.m_AspectRatio = float32(windowConfig.m_Width)
		/ float32(windowConfig.m_Height);

	if (!m_pWindowManager.Create())
	{
		Error("[CEngine] Failed to create the window manager\n");

		return false;
	}

	if (!m_pWindowManager->Start(s_WindowBackend, windowConfig))
	{
		Error("[CEngine] Failed to start window manager\n");

		return false;
	}

	IWindow* pWindow = m_pWindowManager->GetWindow();
	CDisplay::SetWindow(pWindow);

	m_pRenderer = CreateRenderer();
	if (!m_pRenderer)
	{
		Error("[CEngine] Failed to create renderer\n");

		return false;
	}

	RendererDesc_t rendererDesc = {
		pWindow->GetNativeHandle(),
		uint32(windowConfig.m_Width),
		uint32(windowConfig.m_Height),
		false,
		true,
		0
	};
	if (!m_pRenderer->Init(rendererDesc))
	{
		Error("[CEngine] Failed to initialize renderer\n");

		return false;
	}

	CTime::Init();

	if (!m_pInputManager.Create())
	{
		Error("[CEngine] Failed to create the input manager\n");

		return false;
	}

	if (m_pInputManager->Start() == 0)
	{
		Error("[CEngine] Failed to start input\n");

		return false;
	}

	IInput* pInput = m_pInputManager->Get();
	pInput->SetWindow(pWindow->GetNativeHandle());

	if (!m_pAudioManager.Create())
	{
		Error("[CEngine] Failed to create the audio manager\n");

		return false;
	}

	if (m_pAudioManager->Start() == 0)
	{
		Error("[CEngine] Failed to start audio\n");

		return false;
	}

	if (!m_pGUI.Create())
	{
		Error("[CEngine] Failed to create the UI\n");

		return false;
	}

	if (!m_pGUI->Init(*m_pRenderer))
	{
		Error("[CEngine] Failed to initialize UI\n");

		return false;
	}

	CFontHandle hDefaultFont = m_pGUI->LoadFont(
		"default",
		"assets/fonts/font_atlas.png",
		"assets/fonts/font_atlas.metrics");
	if (hDefaultFont == s_InvalidFontHandle)
	{
		Error("[CEngine] Failed to load default font\n");

		return false;
	}

	m_pGUI->SetDefaultFont(hDefaultFont);

	if (!m_pDebug.Create(m_pGUI))
	{
		Error("[CEngine] Failed to create the debug overlay\n");

		return false;
	}

	if (!m_pDebug->Init())
	{
		Error("[CEngine] Failed to initialize debug\n");

		return false;
	}

	if (!m_pRegistry.Create())
	{
		Error("[CEngine] Failed to create the entity registry\n");

		return false;
	}

	if (!m_pScene.Create(m_pRegistry))
	{
		Error("[CEngine] Failed to create the scene\n");

		return false;
	}

	if (!m_pRenderQueue.Create())
	{
		Error("[CEngine] Failed to create the render queue\n");

		return false;
	}

	m_pRenderQueue->Init(*m_pRenderer);

	if (!m_pWrenManager.Create())
	{
		Error("[CEngine] Failed to create the script manager\n");

		return false;
	}

	const char* pScriptPath = m_pApp->GetScriptPath();
	if (pScriptPath)
	{
		WREN_SET_SINGLETON(IRenderer, m_pRenderer);
		WREN_SET_SINGLETON(CUI, m_pGUI);
		WREN_SET_SINGLETON(CDebug, m_pDebug);
		WREN_SET_SINGLETON(CEntityRegistry, m_pRegistry);
		WREN_SET_SINGLETON(CScene, m_pScene);
		WREN_SET_SINGLETON(CRenderQueueManager, m_pRenderQueue);
		WREN_SET_SINGLETON(IInput, pInput);
		WREN_SET_SINGLETON(IAudio, m_pAudioManager->Get());

		if (!m_pWrenManager->Init())
		{
			Error("[CEngine] Failed to initialize script manager\n");

			return false;
		}

		if (!m_pWrenManager->LoadScript(pScriptPath))
		{
			Error("[CEngine] Failed to load script\n");

			return false;
		}
	}

	uint32 framebufferWidth;
	uint32 framebufferHeight;
	m_pRenderer->GetFramebufferSize(framebufferWidth, framebufferHeight);
	windowConfig.m_Width = int32(framebufferWidth);
	windowConfig.m_Height = int32(framebufferHeight);
	windowConfig.m_AspectRatio = m_pRenderer->GetAspectRatio();

	if (!m_pApp->Init(m_pRenderer, windowConfig))
	{
		Error("[CEngine] Failed to initialize app\n");

		return false;
	}

	return true;
}

int CEngine::Run()
{
	IWindow* pWindow = m_pWindowManager->GetWindow();
	IInput* pInput = m_pInputManager->Get();
	IAudio* pAudio = m_pAudioManager->Get();
	const char* pScriptPath = m_pApp->GetScriptPath();

	bool isRunning = true;
	while (isRunning)
	{
		pWindow->PollEvents();
		if (pWindow->ShouldClose())
		{
			break;
		}

		CTime::Update();

		pInput->Update();
		pAudio->Update();

		m_pRenderer->BeginFrame();
		m_pRenderer->Clear(ClearAll, CColor(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0);
		m_pRenderer->SetFullViewport();

		m_pDebug->Update();

		if (pScriptPath)
		{
			isRunning = m_pWrenManager->Update(CTime::GetDeltaTime());
		}
		isRunning = isRunning
			&& m_pApp->Update(m_pRenderer, CTime::GetDeltaTime());

		m_pDebug->Render();
		m_pRenderer->EndFrame();

		CDisplay::WaitForFrame();
	}

	return 0;
}