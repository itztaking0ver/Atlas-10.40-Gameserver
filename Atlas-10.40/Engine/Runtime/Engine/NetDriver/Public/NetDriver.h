#pragma once
#include "../Runtime.h"

namespace NetDriver
{
	DefHookOg(void, TickFlush, UNetDriver*, float, bool);

	void Patch();
}