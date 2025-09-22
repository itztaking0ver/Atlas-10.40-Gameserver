#pragma once
#include "../Runtime.h"

namespace FortInventory
{
    UFortWorldItem* GiveItem(AFortPlayerController*, UFortItemDefinition*, int = 1, int = 0, int = 0, bool = true);
    UFortWorldItem* GiveItem(AFortPlayerController*, FFortItemEntry, int = -1, bool = true, bool = true);

    FFortRangedWeaponStats* GetStats(UFortWeaponItemDefinition*);

    AFortPickupAthena* SpawnPickup(FVector, FFortItemEntry&, EFortPickupSourceTypeFlag = EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource = EFortPickupSpawnSource::Unset, AFortPlayerPawn* = nullptr, int = -1, bool = true, bool = true, bool = true);
    AFortPickupAthena* SpawnPickup(FVector, UFortItemDefinition*, int, int, EFortPickupSourceTypeFlag = EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource = EFortPickupSpawnSource::Unset, AFortPlayerPawn* = nullptr, bool = true);

    FFortItemEntry* MakeItemEntry(UFortItemDefinition*, int32, int32);

    EFortQuickBars GetQuickbar(UFortItemDefinition*);

    UFortWorldItem* GetItemInstance(AFortPlayerController* Controller, const FGuid& Item);
    FFortItemEntry* GetReplicatedEntry(AFortPlayerController* Controller, const FGuid& Item);

    int GetLevel(const FDataTableCategoryHandle&);

    void TriggerInventoryUpdate(AFortPlayerController* PlayerController, FFortItemEntry* Entry);
    void ReplaceEntry(AFortPlayerController*, FFortItemEntry&);
    void Remove(AFortPlayerController*, FGuid);
    void Remove(AFortPlayerController* Controller, FGuid Item, int Count);

    static void GiveItemToInventoryOwner(UObject*, FFrame&);
    static void SpawnFloorLoot();

    bool ServerRemoveInventoryItem(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid, int Count, bool bForceRemoveFromQuickBars, bool bForceRemoval);
    bool RemoveInventoryItem(__int64 InterfaceThing, FGuid ItemGuid, int Count, bool bForceRemoveFromQuickBars, bool bForceRemoval);

    DefHookOg(bool, CompletePickupAnimation, AFortPickup* Pickup);
    DefHookOg(int32, K2_RemoveItemFromPlayer, UObject*, FFrame&, int32*);
    DefHookOg(int32, K2_RemoveItemFromPlayerByGuid, UObject*, FFrame&, int32*);

    void Patch();
}