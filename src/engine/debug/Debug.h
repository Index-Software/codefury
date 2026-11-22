//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef DEBUG_H
#define DEBUG_H

#pragma once

#include "WrenBind.h"

class CUI;

class CDebug : public CWrenBindSingleton<CDebug>
{
	WREN_SINGLETON_DECL()
public:
	CDebug(CUI* pGUI);
	~CDebug();

	bool Init();
	void Update() WREN_BIND_SINGLETON("update", s_pDebug);
	void Render() WREN_BIND_SINGLETON("render", s_pDebug);

	void SetEnabled(bool enabled) WREN_BIND_SINGLETON("setEnabled(_)", s_pDebug);
	bool IsEnabled() const WREN_BIND_SINGLETON("isEnabled", s_pDebug);
private:
	CUI* m_pGUI;

	float64 m_LastFPS;
	float64 m_SumFPS;
	float64 m_LastFrameTime;
	float64 m_SumFrameTime;

	float32 m_FPS;
	float32 m_FPSAverage;
	float32 m_FrameTime;
	float32 m_FrameTimeAverage;

	int32 m_FPSSampleCount;
	int32 m_FrameTimeSampleCount;

	bool m_IsEnabled;
};

#endif // DEBUG_H