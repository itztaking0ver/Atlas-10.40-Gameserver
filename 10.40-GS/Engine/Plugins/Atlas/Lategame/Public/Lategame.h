#pragma once
#include "../Runtime.h"

enum class EAmmoType : uint8
{
    Assault = 0,
    Shotgun = 1,
    Submachine = 2,
    Rocket = 3,
    Sniper = 4,
    EFortAmmoType_MAX = 5,
};

class Lategame {
public:

    static FItemAndCount GetShotguns();
    static FItemAndCount GetAssaultRifles();
    static FItemAndCount GetSnipers();
    static FItemAndCount GetHeals();

    static UFortAmmoItemDefinition* GetAmmo(EAmmoType);
    static UFortResourceItemDefinition* GetResource(EFortResourceType);
};