#include "../Public/BuildingSMActor.h"
#include "../../Inventory/Public/FortInventory.h"

void BuildingSMActor::OnDamageServer(ABuildingSMActor* Actor, float Damage, FGameplayTagContainer DamageTags, FVector Momentum, FHitResult HitInfo, AFortPlayerControllerAthena* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle EffectContext) {
	auto GameState = ((AFortGameStateAthena*)UWorld::GetWorld()->GameState);
	if (!InstigatedBy || Actor->bPlayerPlaced || Actor->GetHealth() == 1 || Actor->IsA(UObject::FindClassFast("B_Athena_VendingMachine_C")) || Actor->IsA(GameState->MapInfo->LlamaClass)) return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
	if (!DamageCauser || !DamageCauser->IsA<AFortWeapon>() || !((AFortWeapon*)DamageCauser)->WeaponData->IsA<UFortWeaponMeleeItemDefinition>()) return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	static auto PickaxeTag = FName(L"Weapon.Melee.Impact.Pickaxe");
	auto Entry = DamageTags.GameplayTags.Search([](FGameplayTag& Entry) {
		return Entry.TagName.ComparisonIndex == PickaxeTag.ComparisonIndex;
		});
	if (!Entry) return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	auto Resource = UFortKismetLibrary::K2_GetResourceItemDefinition(Actor->ResourceType);
	if (!Resource) return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	auto MaxMat = Resource->MaxStackSize;

	FCurveTableRowHandle& BuildingResourceAmountOverride = Actor->BuildingResourceAmountOverride;
	int ResCount = 0;

	if (Actor->BuildingResourceAmountOverride.RowName.ComparisonIndex > 0) {
		float Out;
		UDataTableFunctionLibrary::EvaluateCurveTableRow(Actor->BuildingResourceAmountOverride.CurveTable, Actor->BuildingResourceAmountOverride.RowName, 0.f, nullptr, &Out, FString());

		float RC = Out / (Actor->GetMaxHealth() / Damage);

		ResCount = (int)round(RC);
	}

	if (ResCount > 0) {
		auto itemEntry = InstigatedBy->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& Entry) {
			return Entry.ItemDefinition == Resource;
			});

		if (itemEntry) {
			itemEntry->Count += ResCount;
			if (itemEntry->Count > MaxMat)
			{
				FortInventory::SpawnPickup(InstigatedBy->Pawn->K2_GetActorLocation(), itemEntry->ItemDefinition, itemEntry->Count - MaxMat, 0, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, InstigatedBy->MyFortPawn);
				itemEntry->Count = MaxMat;
			}
			FortInventory::ReplaceEntry(InstigatedBy, *itemEntry);
		}
		else {
			if (ResCount > MaxMat) {
				FortInventory::SpawnPickup(InstigatedBy->Pawn->K2_GetActorLocation(), Resource, ResCount - MaxMat, 0, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, InstigatedBy->MyFortPawn);
				ResCount = MaxMat;
			}
			FortInventory::GiveItem(InstigatedBy, Resource, ResCount, 0, 0, false);
		}
	}
	InstigatedBy->ClientReportDamagedResourceBuilding(Actor, ResCount == 0 ? EFortResourceType::None : Actor->ResourceType, ResCount, false, Damage == 100.f);
	return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
}

bool BuildingSMActor::CanBePlacedByPlayer(UClass* BuildClass) {
	return ((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->AllPlayerBuildableClasses.Search([BuildClass](UClass* Class) { return Class == BuildClass; });
}

void BuildingSMActor::ServerCreateBuildingActor(UObject* Context, FFrame& Stack)
{
	FCreateBuildingActorData CreateBuildingData;
	Stack.StepCompiledIn(&CreateBuildingData);
	Stack.IncrementCode();

	auto PlayerController = (AFortPlayerController*)Context;
	if (!PlayerController) return callOG(PlayerController, "/Script/FortniteGame.FortPlayerController", ServerCreateBuildingActor, CreateBuildingData);

	auto BuildingClassPtr = ((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->AllPlayerBuildableClassesIndexLookup.SearchForKey([&](UClass* Class, int32 Handle) {
		return Handle == CreateBuildingData.BuildingClassHandle;
		});

	if (!BuildingClassPtr) return callOG(PlayerController, "/Script/FortniteGame.FortPlayerController", ServerCreateBuildingActor, CreateBuildingData);
	auto BuildingClass = *BuildingClassPtr;

	TArray<ABuildingSMActor*> RemoveBuildings;
	char _Unknown;
	static auto CantBuild = (__int64 (*)(UWorld*, UObject*, FVector, FRotator, bool, TArray<ABuildingSMActor*> *, char*))(Runtime::Offsets::ImageBase + 0x1601820);

	if (CantBuild(UWorld::GetWorld(), BuildingClass, CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot, CreateBuildingData.bMirrored, &RemoveBuildings, &_Unknown)) return callOG(PlayerController, "/Script/FortniteGame.FortPlayerController", ServerCreateBuildingActor, CreateBuildingData);

	auto Resource = UFortKismetLibrary::K2_GetResourceItemDefinition(((ABuildingSMActor*)BuildingClass->DefaultObject)->ResourceType);
	auto ItemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& Entry) { return Entry.ItemDefinition == Resource; });

	if (!ItemEntry || ItemEntry->Count < 10) return callOG(PlayerController, "/Script/FortniteGame.FortPlayerController", ServerCreateBuildingActor, CreateBuildingData);

	ItemEntry->Count -= 10;
	if (ItemEntry->Count <= 0) {
		FortInventory::Remove(PlayerController, ItemEntry->ItemGuid);
	}

	FortInventory::ReplaceEntry((AFortPlayerControllerAthena*)PlayerController, *ItemEntry);

	for (auto& RemoveBuilding : RemoveBuildings) RemoveBuilding->K2_DestroyActor();
	RemoveBuildings.Free();

	ABuildingSMActor* Building = Runtime::SpawnActorV2<ABuildingSMActor>(CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot, BuildingClass, PlayerController);
	Building->bPlayerPlaced = true;
	Building->InitializeKismetSpawnedBuildingActor(Building, PlayerController, true);
	Building->TeamIndex = ((AFortPlayerStateAthena*)PlayerController->PlayerState)->TeamIndex;
	Building->Team = EFortTeam(Building->TeamIndex);
	if (auto ControllerAthena = PlayerController->Cast<AFortPlayerControllerAthena>()) ControllerAthena->BuildingsCreated++;

	return callOG(PlayerController, "/Script/FortniteGame.FortPlayerController", ServerCreateBuildingActor, CreateBuildingData);
}

void BuildingSMActor::ServerBeginEditingBuildingActor(UObject* Context, FFrame& Stack)
{
	ABuildingSMActor* Building;
	Stack.StepCompiledIn(&Building);
	Stack.IncrementCode();
	auto PlayerController = (AFortPlayerController*)Context;
	if (!PlayerController || !PlayerController->MyFortPawn || !Building || Building->TeamIndex != static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState)->TeamIndex) return;

	AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;
	if (!PlayerState) return;

	auto ItemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([](FFortItemEntry& Entry) { return Entry.ItemDefinition->IsA<UFortEditToolItemDefinition>(); });
	if (!ItemEntry) return;

	PlayerController->ServerExecuteInventoryItem(ItemEntry->ItemGuid);

	auto EditTool = PlayerController->MyFortPawn->CurrentWeapon->Cast<AFortWeap_EditingTool>();
	EditTool->EditActor = Building;
	EditTool->OnRep_EditActor();
}

void BuildingSMActor::ServerEditBuildingActor(UObject* Context, FFrame& Stack)
{
	ABuildingSMActor* Building;
	TSubclassOf<ABuildingSMActor> NewClass;
	uint8 RotationIterations;
	bool bMirrored;
	Stack.StepCompiledIn(&Building);
	Stack.StepCompiledIn(&NewClass);
	Stack.StepCompiledIn(&RotationIterations);
	Stack.StepCompiledIn(&bMirrored);
	Stack.IncrementCode();

	auto PlayerController = (AFortPlayerController*)Context;
	if (!PlayerController || !Building || !NewClass || !Building->IsA<ABuildingSMActor>() || !CanBePlacedByPlayer(NewClass) || Building->TeamIndex != static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState)->TeamIndex) return;

	Building->SetNetDormancy(ENetDormancy::DORM_DormantAll);
	Building->EditingPlayer = nullptr;

	static auto ReplaceBuildingActor = (ABuildingSMActor * (*)(ABuildingSMActor*, unsigned int, UObject*, unsigned int, int, bool, AFortPlayerController*))(Runtime::Offsets::ImageBase + 0x13d0de0);

	ABuildingSMActor* NewBuild = ReplaceBuildingActor(Building, 1, NewClass, Building->CurrentBuildingLevel, RotationIterations, bMirrored, PlayerController);
	if (NewBuild) NewBuild->bPlayerPlaced = true;
}

void BuildingSMActor::ServerEndEditingBuildingActor(UObject* Context, FFrame& Stack)
{
	ABuildingSMActor* Building;
	Stack.StepCompiledIn(&Building);
	Stack.IncrementCode();

	auto PlayerController = (AFortPlayerController*)Context;
	if (!PlayerController || !PlayerController->MyFortPawn || !Building || Building->EditingPlayer != (AFortPlayerStateZone*)PlayerController->PlayerState || Building->TeamIndex != static_cast<AFortPlayerStateAthena*>(PlayerController->PlayerState)->TeamIndex) return;

	Building->SetNetDormancy(ENetDormancy::DORM_DormantAll);
	Building->EditingPlayer = nullptr;

	AFortWeap_EditingTool* EditTool = PlayerController->MyFortPawn->CurrentWeapon->Cast<AFortWeap_EditingTool>();

	if (!EditTool) return;

	EditTool->EditActor = nullptr;
	EditTool->OnRep_EditActor();

	if (auto ControllerAthena = PlayerController->Cast<AFortPlayerControllerAthena>()) ControllerAthena->BuildingsEdited++;
}

void BuildingSMActor::ServerRepairBuildingActor(UObject* Context, FFrame& Stack)
{
	ABuildingSMActor* Building;
	Stack.StepCompiledIn(&Building);
	Stack.IncrementCode();

	auto PlayerController = (AFortPlayerController*)Context;
	if (!PlayerController) return;

	auto Price = (int32)std::floor((10.f * (1.f - Building->GetHealthPercent())) * 0.75f);
	auto ResourceType = UFortKismetLibrary::K2_GetResourceItemDefinition(Building->ResourceType);

	auto ItemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([ResourceType](FFortItemEntry& Entry) { return Entry.ItemDefinition == ResourceType; });

	ItemEntry->Count -= Price;
	if (ItemEntry->Count <= 0) {
		FortInventory::Remove(PlayerController, ItemEntry->ItemGuid);
	}
	else {
		FortInventory::ReplaceEntry(PlayerController, *ItemEntry);
	}

	Building->RepairBuilding(PlayerController, Price);
	if (auto ControllerAthena = PlayerController->Cast<AFortPlayerControllerAthena>()) ControllerAthena->BuildingsRepaired++;
}

void BuildingSMActor::Patch() {
	Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerCreateBuildingActor", ServerCreateBuildingActor, ServerCreateBuildingActorOG);
	Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerBeginEditingBuildingActor", ServerBeginEditingBuildingActor);
	Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerEditBuildingActor", ServerEditBuildingActor);
	Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerEndEditingBuildingActor", ServerEndEditingBuildingActor);
	Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerRepairBuildingActor", ServerRepairBuildingActor);
	Runtime::Hook(Runtime::Offsets::ImageBase + 0x1cc36a0, OnDamageServer, (void**)&OnDamageServerOG);
}