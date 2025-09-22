#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include "SDK/SDK.hpp"
#include <algorithm>
#include <numeric>
#include <map>
#include <intrin.h>
#include <array>
#include <tlhelp32.h>
#include <future>
#include <iomanip>  
#include "Minhook/MinHook.h"
#include "Offsets.h"
#include <set>

using namespace SDK;

template <class X>
using xset = std::set<X, TMemoryAllocator<X>>;
template <class X>
using xvector = std::vector<X, TMemoryAllocator<X>>;
template <class X, class Y>
using xmap = std::map<X, Y, std::less<X>, TMemoryAllocator<std::pair<const X, Y>>>;

float GetMaxTickRate() { return 30.0f; }
int RetTrue() { return true; }
void KickPlayer(AGameSession* GameSession, AFortPlayerControllerAthena* Controller) {};


#define DefUHookOgRet(_Rt, _Name) static inline _Rt (*_Name##OG)(UObject*, FFrame&, _Rt*); static _Rt _Name(UObject *, FFrame&, _Rt*); 
#define DefHookOg(_Rt, _Name, ...) static inline _Rt (*_Name##OG)(##__VA_ARGS__); static _Rt _Name(##__VA_ARGS__); 