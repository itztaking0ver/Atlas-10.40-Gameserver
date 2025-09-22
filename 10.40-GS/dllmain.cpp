#include "framework.h"
#include "Engine/Plugins/Atlas/Public/Atlas.h"

void Main() {
    AllocConsole();
    FILE* F;
    freopen_s(&F, "CONOUT$", "w", stdout);

    UAtlas->SetState("Loading");
    UAtlas->Initialize();
}

bool DllMain(HMODULE Module, DWORD Reason, LPVOID Reserved)
{
    if (Reason == DLL_PROCESS_ATTACH) std::thread(Main).detach();
    return true;
}

