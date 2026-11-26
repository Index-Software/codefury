//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#pragma once

#include "platform.h"

class IAudio;

class CAudioManager
{
public:
	CAudioManager();
	~CAudioManager();

	int32 Start();
	void Shutdown();

	IAudio* Get() const;
private:
	IAudio* m_pAudio;
};

#endif // AUDIO_MANAGER_H