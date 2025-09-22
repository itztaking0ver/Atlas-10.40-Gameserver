#include "../Public/FortInventory.h"
#include "../../Athena/Public/FortPlayerControllerAthena.h"

void FortInventory::TriggerInventoryUpdate(AFortPlayerController* PlayerController, FFortItemEntry* Entry)
{
	if (!PlayerController) return;
	AFortInventory* Inventory = PlayerController->WorldInventory;
	Inventory->bRequiresLocalUpdate = true;
	Inventory->HandleInventoryLocalUpdate();

	return Entry ? Inventory->Inventory.MarkItemDirty(*Entry) : Inventory->Inventory.MarkArrayDirty();
}

UFortWorldItem* FortInventory::GiveItem(AFortPlayerController* PlayerController, UFortItemDefinition* Definition, int Count, int LoadedAmmo, int Level, bool bUpdateInventory)
{
	if (!PlayerController || !Definition) return nullptr;

	UFortWorldItem* Item = (UFortWorldItem*)Definition->CreateTemporaryItemInstanceBP(Count, Level);
	Item->SetOwningControllerForTemporaryItem(PlayerController);
	Item->ItemEntry.LoadedAmmo = LoadedAmmo;

	PlayerController->WorldInventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);
	PlayerController->WorldInventory->Inventory.ItemInstances.Add(Item);

	if (bUpdateInventory) TriggerInventoryUpdate(PlayerController, &Item->ItemEntry);

	return Item;
}

FFortRangedWeaponStats* FortInventory::GetStats(UFortWeaponItemDefinition* Def)
{
	if (!Def || !Def->WeaponStatHandle.DataTable) return nullptr;

	if (!Def->IsA<UFortWeaponRangedItemDefinition>()) return nullptr;

	auto Val = Def->WeaponStatHandle.DataTable->RowMap.Search([Def](FName& Key, uint8_t* Value)
		{ return Def->WeaponStatHandle.RowName == Key && Value; });

	return Val ? *(FFortRangedWeaponStats**)Val : nullptr;
}

int FortInventory::GetLevel(const FDataTableCategoryHandle& CategoryHandle)
{
	auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
	auto GameState = (AFortGameStateAthena*)GameMode->GameState;

	if (!CategoryHandle.DataTable) return 0;
	if (!CategoryHandle.ColumnName) return 0;
	if (!CategoryHandle.RowContents.ComparisonIndex) return 0;

	TArray<FFortLootLevelData*> LootLevelData;

	for (auto& LootLevelDataPair : (TMap<FName, FFortLootLevelData*>)CategoryHandle.DataTable->RowMap)
	{
		if (LootLevelDataPair.Value()->Category != CategoryHandle.RowContents) continue;
		LootLevelData.Add(LootLevelDataPair.Value());
	}

	if (LootLevelData.Num() > 0)
	{
		int ind = -1;
		int ll = 0;

		for (int i = 0; i < LootLevelData.Num(); i++)
		{
			if (LootLevelData[i]->LootLevel <= GameState->WorldLevel && LootLevelData[i]->LootLevel > ll)
			{
				ll = LootLevelData[i]->LootLevel;
				ind = i;
			}
		}

		if (ind != -1)
		{
			auto subbed = LootLevelData[ind]->MaxItemLevel - LootLevelData[ind]->MinItemLevel;

			if (subbed <= -1) subbed = 0;
			else
			{
				auto calc = (int)(((float)rand() / 32767) * (float)(subbed + 1));
				if (calc <= subbed) subbed = calc;
			}

			return subbed + LootLevelData[ind]->MinItemLevel;
		}
	}

	return 0;
}

AFortPickupAthena* FortInventory::SpawnPickup(FVector Loc, FFortItemEntry& Entry, EFortPickupSourceTypeFlag SourceTypeFlag, EFortPickupSpawnSource SpawnSource, AFortPlayerPawn* Pawn, int OverrideCount, bool Toss, bool RandomRotation, bool bCombine)
{
	if (!&Entry) return nullptr;

	AFortPickupAthena* NewPickup = Runtime::SpawnActor<AFortPickupAthena>(Loc, {});
	if (!NewPickup) return nullptr;

	NewPickup->bRandomRotation = RandomRotation;
	((FVector * (*)(AFortPickup*, FFortItemEntry*, TArray<FFortItemEntry>, bool))(Runtime::Offsets::ImageBase + 0x170B2D0))(NewPickup, &Entry, TArray<FFortItemEntry>(), false);
	NewPickup->PrimaryPickupItemEntry.Count = OverrideCount != -1 ? OverrideCount : Entry.Count;
	NewPickup->OnRep_PrimaryPickupItemEntry();
	NewPickup->PawnWhoDroppedPickup = Pawn;

	NewPickup->TossPickup(Loc, Pawn, -1, Toss, SourceTypeFlag, SpawnSource);
	NewPickup->bTossedFromContainer = SpawnSource == EFortPickupSpawnSource::Chest || SpawnSource == EFortPickupSpawnSource::AmmoBox;
	if (NewPickup->bTossedFromContainer) NewPickup->OnRep_TossedFromContainer();

	return NewPickup;
}


FFortItemEntry* FortInventory::MakeItemEntry(UFortItemDefinition* ItemDefinition, int32 Count, int32 Level) {
	FFortItemEntry* IE = new FFortItemEntry();

	IE->MostRecentArrayReplicationKey = -1;
	IE->ReplicationID = -1;
	IE->ReplicationKey = -1;

	IE->ItemDefinition = ItemDefinition;
	IE->Count = Count;
	IE->LoadedAmmo = ItemDefinition->IsA<UFortWeaponItemDefinition>() ? GetStats((UFortWeaponItemDefinition*)ItemDefinition)->ClipSize : 0;
	IE->Durability = 1.f;
	IE->GameplayAbilitySpecHandle = FGameplayAbilitySpecHandle(-1);
	IE->ParentInventory.ObjectIndex = -1;
	IE->Level = Level;

	return IE;
}

AFortPickupAthena* FortInventory::SpawnPickup(FVector Loc, UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo, EFortPickupSourceTypeFlag SourceTypeFlag, EFortPickupSpawnSource SpawnSource, AFortPlayerPawn* Pawn, bool Toss)
{
	return SpawnPickup(Loc, *MakeItemEntry(ItemDefinition, Count, 0), SourceTypeFlag, SpawnSource, Pawn, -1, Toss, true, true);
}

UFortWorldItem* FortInventory::GetItemInstance(AFortPlayerController* PlayerController, const FGuid& Item)
{
	if (!PlayerController->WorldInventory) return nullptr;

	for (int i = 0; i < PlayerController->WorldInventory->Inventory.ItemInstances.Num(); i++)
	{
		auto ItemInstance = PlayerController->WorldInventory->Inventory.ItemInstances[i];

		if (ItemInstance && ItemInstance->ItemEntry.ItemGuid == Item) return ItemInstance;
	}

	return nullptr;
}

FFortItemEntry* FortInventory::GetReplicatedEntry(AFortPlayerController* PlayerController, const FGuid& Item)
{
	for (int i = 0; i < PlayerController->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
	{
		if (PlayerController->WorldInventory->Inventory.ReplicatedEntries[i].ItemGuid == Item) return &PlayerController->WorldInventory->Inventory.ReplicatedEntries.Data[i];
	}

	return nullptr;
}

void FortInventory::Remove(AFortPlayerController* Controller, FGuid Item, int Count)
{
	if (!Controller || !Controller->WorldInventory) return;

	auto ItemInstance = GetItemInstance(Controller, Item);
	auto ReplicatedEntry = GetReplicatedEntry(Controller, Item);
	auto ReplicatedEntries = &Controller->WorldInventory->Inventory.ReplicatedEntries;
	auto ItemInstances = &Controller->WorldInventory->Inventory.ItemInstances;

	if (ItemInstance && ReplicatedEntry)
	{
		auto NewCount = ReplicatedEntry->Count - Count;
		auto ItemDefinition = reinterpret_cast<UFortWorldItemDefinition*>(ReplicatedEntry->ItemDefinition);

		if (NewCount > 0 || (ItemDefinition && ItemDefinition->bPersistInInventoryWhenFinalStackEmpty))
		{
			if (ItemDefinition && ItemDefinition->bPersistInInventoryWhenFinalStackEmpty) NewCount = NewCount < 0 ? 0 : NewCount;

			ItemInstance->ItemEntry.Count = NewCount;
			ReplicatedEntry->Count = NewCount;

			Controller->WorldInventory->Inventory.MarkItemDirty(ItemInstance->ItemEntry);
			Controller->WorldInventory->Inventory.MarkItemDirty(*ReplicatedEntry);
		}
		else
		{
			for (int i = 0; i < ItemInstances->Num(); i++)
			{
				auto ItemInstance = ItemInstances->operator[](i);
				if (!ItemInstance) continue;

				auto CurrentGuid = ItemInstance->ItemEntry.ItemGuid;
				if (CurrentGuid == Item)
				{
					ItemInstance->ItemEntry.StateValues.Free();
					ItemInstances->Remove(i);
					break;
				}
			}

			for (int i = 0; i < ReplicatedEntries->Num(); i++)
			{
				auto& Entry = ReplicatedEntries->operator[](i);
				auto CurrentGuid = Entry.ItemGuid;

				if (CurrentGuid == Item)
				{
					Entry.StateValues.Free();
					ReplicatedEntries->Remove(i);
					break;
				}
			}

			auto Pawn = Controller->MyFortPawn;

			if (Pawn)
			{
				for (int i = 0; i < Pawn->CurrentWeaponList.Num(); i++)
				{
					auto CurrentWeaponInList = Pawn->CurrentWeaponList[i];

					if (CurrentWeaponInList->ItemEntryGuid == Item)
					{
						CurrentWeaponInList->K2_DestroyActor();
						Pawn->CurrentWeaponList.Remove(i);
					}
				}
			}
		}
	}
}

EFortQuickBars FortInventory::GetQuickbar(UFortItemDefinition* ItemDefinition)
{
	if (!ItemDefinition) return EFortQuickBars::Max_None;
	return ItemDefinition->IsA<UFortWeaponMeleeItemDefinition>() || ItemDefinition->IsA<UFortResourceItemDefinition>() || ItemDefinition->IsA<UFortAmmoItemDefinition>() || ItemDefinition->IsA<UFortTrapItemDefinition>() || ItemDefinition->IsA<UFortBuildingItemDefinition>() || ItemDefinition->IsA<UFortEditToolItemDefinition>() || ((UFortWorldItemDefinition*)ItemDefinition)->bForceIntoOverflow ? EFortQuickBars::Secondary : EFortQuickBars::Primary;
}

void FortInventory::GiveItemToInventoryOwner(UObject* Object, FFrame& Stack)
{
	TScriptInterface<class IFortInventoryOwnerInterface> InventoryOwner;
	UFortWorldItemDefinition* ItemDefinition;
	FGuid ItemVariantGuid;
	int32 NumberToGive;
	bool bNotifyPlayer;
	int32 ItemLevel;
	int32 PickupInstigatorHandle;
	bool bUseItemPickupAnalyticEvent;
	Stack.StepCompiledIn(&InventoryOwner);
	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&ItemVariantGuid);
	Stack.StepCompiledIn(&NumberToGive);
	Stack.StepCompiledIn(&bNotifyPlayer);
	Stack.StepCompiledIn(&ItemLevel);
	Stack.StepCompiledIn(&PickupInstigatorHandle);
	Stack.StepCompiledIn(&bUseItemPickupAnalyticEvent);
	Stack.IncrementCode();

	auto PlayerController = (AFortPlayerControllerAthena*)InventoryOwner.ObjectPointer;
	auto ItemEntry = MakeItemEntry(ItemDefinition, NumberToGive, ItemLevel);
	FortPlayerControllerAthena::InternalPickup(PlayerController, *ItemEntry);
}

UFortWorldItem* FortInventory::GiveItem(AFortPlayerController* Controller, FFortItemEntry Entry, int Count, bool ShowPickupNoti, bool updateInventory)
{
	if (Count == -1) Count = Entry.Count;
	return GiveItem(Controller, Entry.ItemDefinition, Count, Entry.LoadedAmmo, Entry.Level, updateInventory);
}


bool FortInventory::CompletePickupAnimation(AFortPickup* Pickup) {
	auto Pawn = (AFortPlayerPawnAthena*)Pickup->PickupLocationData.PickupTarget;
	if (!Pawn) return CompletePickupAnimationOG(Pickup);
	auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;
	if (!PlayerController) return CompletePickupAnimationOG(Pickup);

	if (auto entry = (FFortItemEntry*)PlayerController->SwappingItemDefinition)
	{
		SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), *entry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PlayerController->MyFortPawn);
		Remove(PlayerController, entry->ItemGuid);
		GiveItem(PlayerController, Pickup->PrimaryPickupItemEntry);
		PlayerController->SwappingItemDefinition = nullptr;
	}
	else
	{
		FortPlayerControllerAthena::InternalPickup(PlayerController, Pickup->PrimaryPickupItemEntry);
	}

	return CompletePickupAnimationOG(Pickup);
}


void FortInventory::Remove(AFortPlayerController* PlayerController, FGuid Guid)
{
	if (!PlayerController) return;

	auto ItemEntryIdx = PlayerController->WorldInventory->Inventory.ReplicatedEntries.SearchIndex([&](FFortItemEntry& entry) { return entry.ItemGuid == Guid; });
	if (ItemEntryIdx != -1) PlayerController->WorldInventory->Inventory.ReplicatedEntries.Remove(ItemEntryIdx);

	auto ItemInstanceIdx = PlayerController->WorldInventory->Inventory.ItemInstances.SearchIndex([&](UFortWorldItem* entry) { return entry->ItemEntry.ItemGuid == Guid; });
	auto ItemInstance = PlayerController->WorldInventory->Inventory.ItemInstances.Search([&](UFortWorldItem* entry) { return entry->ItemEntry.ItemGuid == Guid; });

	auto Instance = ItemInstance ? *ItemInstance : nullptr;
	if (ItemInstanceIdx != -1) PlayerController->WorldInventory->Inventory.ItemInstances.Remove(ItemInstanceIdx);

	AFortInventory* Inventory = PlayerController->WorldInventory;
	Inventory->bRequiresLocalUpdate = true;
	Inventory->HandleInventoryLocalUpdate();
}

void FortInventory::ReplaceEntry(AFortPlayerController* PlayerController, FFortItemEntry& Entry)
{
	if (!PlayerController) return;

	auto ent = PlayerController->WorldInventory->Inventory.ItemInstances.Search([&](UFortWorldItem* item) { return item->ItemEntry.ItemGuid == Entry.ItemGuid; });
	if (ent) (*ent)->ItemEntry = Entry;

	AFortInventory* Inventory = PlayerController->WorldInventory;
	Inventory->bRequiresLocalUpdate = true;
	Inventory->HandleInventoryLocalUpdate();

	Entry.ItemDefinition ? Inventory->Inventory.MarkItemDirty(Entry) : Inventory->Inventory.MarkArrayDirty();
}

bool FortInventory::ServerRemoveInventoryItem(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid, int Count, bool bForceRemoveFromQuickBars, bool bForceRemoval)
{
	auto ItemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry) { return entry.ItemGuid == ItemGuid; });
	if (!ItemEntry) return false;

	ItemEntry->Count -= Count;
	if (ItemEntry->Count <= 0) {
		FortInventory::Remove(PlayerController, ItemEntry->ItemGuid);
	}
	else {
		FortInventory::ReplaceEntry(PlayerController, *ItemEntry);
	}

	return true;
}

bool FortInventory::RemoveInventoryItem(__int64 InterfaceThing, FGuid ItemGuid, int Count, bool bForceRemoveFromQuickBars, bool bForceRemoval)
{
	return ServerRemoveInventoryItem((AFortPlayerControllerAthena*)(InterfaceThing - 1432), ItemGuid, Count, bForceRemoveFromQuickBars, bForceRemoval);
}

int32 FortInventory::K2_RemoveItemFromPlayer(UObject* Object, FFrame& Stack, int32* Ret)
{
	class AFortPlayerControllerAthena* PlayerController;
	UFortWorldItemDefinition* ItemDefinition;
	int32 AmountToRemove;
	bool bForceRemoval;
	Stack.StepCompiledIn(&PlayerController);
	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&AmountToRemove);
	Stack.StepCompiledIn(&bForceRemoval);

	auto ItemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry) { return entry.ItemDefinition == ItemDefinition; });
	if (!ItemEntry) return K2_RemoveItemFromPlayerOG(Object, Stack, Ret);

	ItemEntry->Count -= AmountToRemove;
	if (ItemEntry->Count <= 0 || ItemEntry->ItemDefinition->IsA<UFortGadgetItemDefinition>())
	{
		FortInventory::Remove(PlayerController, ItemEntry->ItemGuid);
	}
	else {
		FortInventory::ReplaceEntry(PlayerController, *ItemEntry);
	}

	return K2_RemoveItemFromPlayerOG(Object, Stack, Ret);
}

int32 FortInventory::K2_RemoveItemFromPlayerByGuid(UObject* Object, FFrame& Stack, int32* Ret)
{
	class AFortPlayerControllerAthena* PlayerController;
	struct FGuid ItemGuid;
	int32 AmountToRemove;
	bool bForceRemoval;
	Stack.StepCompiledIn(&PlayerController);
	Stack.StepCompiledIn(&ItemGuid);
	Stack.StepCompiledIn(&AmountToRemove);
	Stack.StepCompiledIn(&bForceRemoval);

	auto ItemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry) { return entry.ItemGuid == ItemGuid; });
	if (!ItemEntry) return K2_RemoveItemFromPlayerByGuidOG(Object, Stack, Ret);

	auto RemoveCount = AmountToRemove;
	ItemEntry->Count -= AmountToRemove;
	if (ItemEntry->Count <= 0 || ItemEntry->ItemDefinition->IsA<UFortGadgetItemDefinition>()) {
		RemoveCount += ItemEntry->Count;
		FortInventory::Remove(PlayerController, ItemEntry->ItemGuid);
	}
	else {
		FortInventory::ReplaceEntry(PlayerController, *ItemEntry);
	}

	return K2_RemoveItemFromPlayerByGuidOG(Object, Stack, Ret);
}

void FortInventory::Patch()
{
	Runtime::Hook(Runtime::Offsets::ImageBase + 0x16f7d10, CompletePickupAnimation, (void**)&CompletePickupAnimationOG);
	Runtime::Exec("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", GiveItemToInventoryOwner);
	Runtime::Hook(Runtime::Offsets::ImageBase + 0x1f10130, ServerRemoveInventoryItem);
	Runtime::Hook(Runtime::Offsets::ImageBase + 0x19c4410, RemoveInventoryItem);
	Runtime::Exec("/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayer", K2_RemoveItemFromPlayer, K2_RemoveItemFromPlayerOG);
	Runtime::Exec("/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayerByGuid", K2_RemoveItemFromPlayerByGuid, K2_RemoveItemFromPlayerByGuidOG);
}
