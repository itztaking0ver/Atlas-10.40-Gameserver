#include "../Public/DispatchRequest.h"

__int64 DispatchRequest::DispatchRequest(__int64 a1, __int64* a2, int a3)
{
	a3 = 3;

	return DispatchRequestOG(a1, a2, a3);
}

void DispatchRequest::Patch()
{
	Runtime::Hook(Runtime::Offsets::DispatchRequest, DispatchRequest, (void**)&DispatchRequestOG);
}
