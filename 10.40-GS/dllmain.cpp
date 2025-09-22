#include "framework.h"
#include "Engine/Plugins/Crystal/Public/Crystal.h"

void Main() {
    AllocConsole();
    FILE* F;
    freopen_s(&F, "CONOUT$", "w", stdout);

    UCrystal->SetState("Loading");
    UCrystal->Initialize();
}

bool DllMain(HMODULE Module, DWORD Reason, LPVOID Reserved)
{
    if (Reason == DLL_PROCESS_ATTACH) std::thread(Main).detach();
    return true;
}

