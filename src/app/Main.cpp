//===----------------------------------------------------------------------===//
//
// Part of the codefury, under the MIT License.
// See LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//

#include "app/Engine.h"

int main(int argc, char* argv[])
{
	CEngine engine;
	if (!engine.Init())
	{
		return 1;
	}

	return engine.Run();
}