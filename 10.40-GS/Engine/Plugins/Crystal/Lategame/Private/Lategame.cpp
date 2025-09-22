#include "../Public/Lategame.h"

FItemAndCount Lategame::GetShotguns()
{
    static xvector<FItemAndCount> Shotguns{
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_VR_Ore_T03.WID_Shotgun_Standard_Athena_VR_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_SR_Ore_T03.WID_Shotgun_Standard_Athena_SR_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_HighSemiAuto_Athena_VR_Ore_T03.WID_Shotgun_HighSemiAuto_Athena_VR_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_HighSemiAuto_Athena_SR_Ore_T03.WID_Shotgun_HighSemiAuto_Athena_SR_Ore_T03")), 
    };
    return Shotguns[rand() % (Shotguns.size() - 1)];
}

FItemAndCount Lategame::GetAssaultRifles()
{
    static xvector<FItemAndCount> AssaultRifles{
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_R_Ore_T03.WID_Assault_Heavy_Athena_R_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_R_Ore_T03.WID_Assault_Auto_Athena_R_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_VR_Ore_T03.WID_Assault_AutoHigh_Athena_VR_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Suppressed_Athena_VR_Ore_T03.WID_Assault_Suppressed_Athena_VR_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Suppressed_Athena_SR_Ore_T03.WID_Assault_Suppressed_Athena_SR_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_R_Ore_T03.WID_Assault_Heavy_Athena_R_Ore_T03")),
    };

    return AssaultRifles[rand() % (AssaultRifles.size() - 1)];
}


FItemAndCount Lategame::GetSnipers()
{
    static xvector<FItemAndCount> Snipers{
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_VR_Ore_T03.WID_Sniper_Heavy_Athena_VR_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_SR_Ore_T03.WID_Sniper_Heavy_Athena_SR_Ore_T03")),
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_SR_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_SR_Ore_T03")),
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_VR_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_VR_Ore_T03")), 
    };
    return Snipers[rand() % (Snipers.size() - 1)];
}

FItemAndCount Lategame::GetHeals()
{
    static xvector<FItemAndCount> Heals{
        FItemAndCount(3, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Shields/Athena_Shields.Athena_Shields")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Medkit/Athena_Medkit.Athena_Medkit")),
        FItemAndCount(6, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall")), 
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Hook_Gun_VR_Ore_T03.WID_Hook_Gun_VR_Ore_T03")),
        FItemAndCount(6, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/ShockwaveGrenade/Athena_ShockGrenade.Athena_ShockGrenade")),
        FItemAndCount(1, {}, Runtime::StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Badger_Grape_VR.WID_Badger_Grape_VR")), 
    };
    return Heals[rand() % (Heals.size() - 1)];
}

UFortAmmoItemDefinition* Lategame::GetAmmo(EAmmoType AmmoType) {
    static auto Assault = Runtime::StaticFindObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
    static auto Shotgun = Runtime::StaticFindObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
    static auto Submachine = Runtime::StaticFindObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
    static auto Rocket = Runtime::StaticFindObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets");
    static auto Sniper = Runtime::StaticFindObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");


    switch (AmmoType) {
    case EAmmoType::Assault: return Assault;
    case EAmmoType::Shotgun: return Shotgun;
    case EAmmoType::Submachine: return Submachine;
    case EAmmoType::Rocket: return Rocket;
    case EAmmoType::Sniper: return Sniper;
    default: return nullptr;
    }
}

UFortResourceItemDefinition* Lategame::GetResource(EFortResourceType ResourceType) {

    static auto Wood = Runtime::StaticFindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
    static auto Stone = Runtime::StaticFindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
    static auto Metal = Runtime::StaticFindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

    switch (ResourceType) {
    case EFortResourceType::Wood: return Wood;
    case EFortResourceType::Stone: return Stone;
    case EFortResourceType::Metal: return Metal;
    default: return nullptr;
    }
}