#pragma once
#include "../Runtime.h"

namespace DispatchRequest
{
	DefHookOg(__int64, DispatchRequest, _int64 a1, __int64* a2, int a3);

	void Patch();
}