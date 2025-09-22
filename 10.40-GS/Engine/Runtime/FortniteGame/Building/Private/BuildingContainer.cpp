#include "../Public/BuildingContainer.h"
#include "../../Inventory/Public/FortInventory.h"
#include <algorithm>

// reminder, this is crystal gs that was made in like a week. so this code below is skidded
template<typename T>
T* SelectWeightedEntry(const std::vector<T*>& Entries)
{
	if (Entries.empty()) return nullptr;

	float TotalWeight = 0.0f;
	for (const auto& Entry : Entries) TotalWeight += Entry->Weight;

	if (TotalWeight <= 0.0f) return nullptr;

	float RandomFloat = UKismetMathLibrary::RandomFloat() * TotalWeight;
	float CumulativeProbability = 0.0f;

	for (const auto& Entry : Entries)
	{
		CumulativeProbability += Entry->Weight;
		if (RandomFloat <= CumulativeProbability) return Entry;
	}

	return nullptr;
}

FFortLootTierData* BuildingContainer::SelectLootTierData(std::vector<FFortLootTierData*> LootTierDatas)
{
	return SelectWeightedEntry(LootTierDatas);
}

FFortLootPackageData* BuildingContainer::SelectLootPackage(std::vector<FFortLootPackageData*> LootPackages)
{
	return SelectWeightedEntry(LootPackages);
}

std::vector<FFortItemEntry> BuildingContainer::InternalPickLootDrops(FName LootTierGroup)
{
	std::vector<FFortItemEntry> LootDrops;

	auto* GameState = UWorld::GetWorld()->GameState->Cast<AFortGameStateAthena>();
	if (!GameState) return LootDrops;

	auto* CurrentPlaylist = GameState->CurrentPlaylistInfo.BasePlaylist;
	if (!CurrentPlaylist) return LootDrops;

	std::string LootTierDataPath, LootPackagesPath;

	if (CurrentPlaylist->bIsDefaultPlaylist)
	{
		LootTierDataPath = "/Game/Items/DataTables/AthenaLootTierData_Client.AthenaLootTierData_Client";
		LootPackagesPath = "/Game/Items/DataTables/AthenaLootPackages_Client.AthenaLootPackages_Client";
	}
	else
	{
		LootTierDataPath = UKismetStringLibrary::Conv_NameToString(CurrentPlaylist->LootTierData.ObjectID.AssetPathName).ToString();
		LootPackagesPath = UKismetStringLibrary::Conv_NameToString(CurrentPlaylist->LootPackages.ObjectID.AssetPathName).ToString();
	}

	static UDataTable* LootTierData = Runtime::StaticLoadObject<UDataTable>(LootTierDataPath);
	static UDataTable* LootPackages = Runtime::StaticLoadObject<UDataTable>(LootPackagesPath);
	if (!LootTierData || !LootPackages) return LootDrops;

	std::vector<FFortLootTierData*> ValidTierData;
	for (const auto& [RowName, RowPtr] : LootTierData->RowMap)
	{
		auto* Data = reinterpret_cast<FFortLootTierData*>(RowPtr);
		if (Data && Data->TierGroup == LootTierGroup && Data->Weight > 0) ValidTierData.push_back(Data);
	}

	auto* ChosenTier = SelectLootTierData(ValidTierData);
	if (!ChosenTier) return LootDrops;

	std::vector<FFortLootPackageData*> ValidPackages;
	for (const auto& [RowName, RowPtr] : LootPackages->RowMap)
	{
		auto* Data = reinterpret_cast<FFortLootPackageData*>(RowPtr);
		if (Data && Data->LootPackageID == ChosenTier->LootPackage && Data->Weight > 0) ValidPackages.push_back(Data);
	}

	const int NumDrops = static_cast<int>(std::floor(ChosenTier->NumLootPackageDrops));
	for (int i = 0; i < NumDrops && i < ValidPackages.size(); ++i)
	{
		auto* Package = ValidPackages[i];
		std::vector<FFortLootPackageData*> LootPackageCalls;

		if (Package->LootPackageCall.ToString().empty())
		{
			LootPackageCalls.push_back(Package);
		}
		else
		{
			for (const auto& [RowName, RowPtr] : LootPackages->RowMap)
			{
				auto* Data = reinterpret_cast<FFortLootPackageData*>(RowPtr);
				if (!Data || Data->Weight <= 0) continue;

				std::string PackageIDStr = UKismetStringLibrary::Conv_NameToString(Data->LootPackageID).ToString();
				if (PackageIDStr == Package->LootPackageCall.ToString()) LootPackageCalls.push_back(Data);
			}
		}

		auto* FinalPackage = SelectLootPackage(LootPackageCalls);
		if (!FinalPackage) continue;

		auto ItemDefPath = UKismetStringLibrary::Conv_NameToString(FinalPackage->ItemDefinition.ObjectID.AssetPathName).ToString();
		auto* ItemDef = Runtime::StaticLoadObject<UFortItemDefinition>(ItemDefPath);
		if (!ItemDef) continue;

		FFortItemEntry Entry{};
		Entry.ItemDefinition = ItemDef;
		Entry.Count = FinalPackage->Count;

		LootDrops.push_back(Entry);
	}

	return LootDrops;
}

bool BuildingContainer::SpawnLootHook(ABuildingContainer* Container)
{
	auto& RedirectedTierGroup = Container->SearchLootTierGroup;
	for (auto& Pair : UWorld::GetWorld()->AuthorityGameMode->Cast<AFortGameModeAthena>()->RedirectAthenaLootTierGroups)
	{
		if (Pair.Key() == RedirectedTierGroup) {
			RedirectedTierGroup = Pair.Value();
			break;
		}
	}

	for (FFortItemEntry& Entry : InternalPickLootDrops(RedirectedTierGroup)) FortInventory::SpawnPickup(Container->K2_GetActorLocation() + Container->GetActorRightVector() * 70.f + FVector{ 0, 0, 50 }, Entry);

	if (!Container->bAlreadySearched)
	{
		Container->bAlreadySearched = true;
		Container->OnRep_bAlreadySearched();
		Container->SearchBounceData.SearchAnimationCount++;
		Container->BounceContainer();
	}

	return true;
}

void BuildingContainer::SetupLDSForPackage(TArray<FFortItemEntry>& LootDrops, SDK::FName Package, int i, FName TierGroup, int WorldLevel) {
	xvector<FFortLootPackageData*> LPGroups;
	for (auto const& Val : LPGroupsAll)
	{
		if (!Val) continue;
		if (Val->LootPackageID != Package) continue;

		if (i != -1 && Val->LootPackageCategory != i) continue;
		if (WorldLevel >= 0) {
			if (Val->MaxWorldLevel >= 0 && WorldLevel > Val->MaxWorldLevel) continue;
			if (Val->MinWorldLevel >= 0 && WorldLevel < Val->MinWorldLevel) continue;
		}
		LPGroups.push_back(Val);
	}

	if (LPGroups.size() == 0) return;

	auto LootPackage = PickWeighted(LPGroups, [](float Total) { return ((float)rand() / 32767.f) * Total; });
	if (!LootPackage) return;

	if (LootPackage->LootPackageCall.Num() > 1)
	{
		for (int i = 0; i < LootPackage->Count; i++)
			SetupLDSForPackage(LootDrops, FName(LootPackage->LootPackageCall), 0, TierGroup);

		return;
	}

	auto ItemDefinition = LootPackage->ItemDefinition->Cast<UFortWorldItemDefinition>();
	if (!ItemDefinition) return;

	bool found = false;
	for (auto& LootDrop : LootDrops) {
		if (LootDrop.ItemDefinition == ItemDefinition) {
			LootDrop.Count += LootPackage->Count;
			if (LootDrop.Count > ItemDefinition->MaxStackSize) {
				auto OGCount = LootDrop.Count;
				LootDrop.Count = (int32)ItemDefinition->MaxStackSize;

				if (FortInventory::GetQuickbar(LootDrop.ItemDefinition) == EFortQuickBars::Secondary) LootDrops.Add(*FortInventory::MakeItemEntry(ItemDefinition, OGCount - ItemDefinition->MaxStackSize, std::clamp(FortInventory::GetLevel(ItemDefinition->LootLevelData), ItemDefinition->MinLevel, ItemDefinition->MaxLevel)));
			}
			if (FortInventory::GetQuickbar(LootDrop.ItemDefinition) == EFortQuickBars::Secondary) found = true;
		}
	}

	if (!found && LootPackage->Count > 0) LootDrops.Add(*FortInventory::MakeItemEntry(ItemDefinition, LootPackage->Count, std::clamp(FortInventory::GetLevel(ItemDefinition->LootLevelData), ItemDefinition->MinLevel, ItemDefinition->MaxLevel)));
}


TArray<FFortItemEntry> BuildingContainer::ChooseLootForContainer(FName TierGroup, int LootTier, int WorldLevel) {
	xvector<FFortLootTierData*> TierDataGroups;
	for (auto const& Val : TierDataAllGroups) {
		if (Val->TierGroup == TierGroup && (LootTier == -1 ? true : LootTier == Val->LootTier)) TierDataGroups.push_back(Val);
	}

	auto LootTierData = PickWeighted(TierDataGroups, [](float Total) { return ((float)rand() / 32767.f) * Total; });
	if (!LootTierData) return {};

	float DropCount = 0;
	if (LootTierData->NumLootPackageDrops > 0) {
		DropCount = LootTierData->NumLootPackageDrops < 1 ? 1 : (float)((int)((LootTierData->NumLootPackageDrops * 2) - .5f) >> 1);
		if (LootTierData->NumLootPackageDrops > 1) {
			float idk = LootTierData->NumLootPackageDrops - DropCount;
			if (idk > 0.0000099999997f)
				DropCount += idk >= ((float)rand() / 32767);
		}
	}

	float AmountOfLootDrops = 0;
	float MinLootDrops = 0;

	for (auto& Min : LootTierData->LootPackageCategoryMinArray) AmountOfLootDrops += Min;

	int SumWeights = 0;

	for (int i = 0; i < LootTierData->LootPackageCategoryWeightArray.Num(); ++i) {
		if (LootTierData->LootPackageCategoryWeightArray[i] > 0 && LootTierData->LootPackageCategoryMaxArray[i] != 0) {
			SumWeights += LootTierData->LootPackageCategoryWeightArray[i];
		}
	}

	while (SumWeights > 0)
	{
		AmountOfLootDrops++;

		if (AmountOfLootDrops >= LootTierData->NumLootPackageDrops) {
			AmountOfLootDrops = AmountOfLootDrops;
			break;
		}

		SumWeights--;
	}

	if (!AmountOfLootDrops) AmountOfLootDrops = AmountOfLootDrops;
	TArray<FFortItemEntry> LootDrops;

	for (int i = 0; i < AmountOfLootDrops && i < LootTierData->LootPackageCategoryMinArray.Num(); i++) {
		for (int j = 0; j < LootTierData->LootPackageCategoryMinArray[i] && LootTierData->LootPackageCategoryMinArray[i] >= 1; j++) {
			SetupLDSForPackage(LootDrops, LootTierData->LootPackage, i, TierGroup, WorldLevel);
		}
	}

	return LootDrops;
}


void BuildingContainer::SpawnLoot(FName& TierGroup, FVector Loc) {
	auto& RealTierGroup = TierGroup;
	if (RealTierGroup == FName(L"Loot_Treasure"))
	{
		RealTierGroup = FName(L"Loot_AthenaTreasure");
	}
	else {
		RealTierGroup = FName(L"Loot_AthenaAmmoLarge");
	}

	for (auto& LootDrop : ChooseLootForContainer(RealTierGroup))
	{
		FortInventory::SpawnPickup(Loc, LootDrop);
	}
}

void BuildingContainer::SpawnFloorLootForContainer(UBlueprintGeneratedClass* ContainerType) {
	auto Containers = Runtime::GetAll<ABuildingContainer>(ContainerType);

	for (auto& BuildingContainer : Containers)
	{
		SpawnLoot(BuildingContainer->SearchLootTierGroup, BuildingContainer->K2_GetActorLocation() + BuildingContainer->GetActorForwardVector() * BuildingContainer->LootSpawnLocation_Athena.X + BuildingContainer->GetActorRightVector() * BuildingContainer->LootSpawnLocation_Athena.Y + BuildingContainer->GetActorUpVector() * BuildingContainer->LootSpawnLocation_Athena.Z);
		BuildingContainer->K2_DestroyActor();
	}

	Containers.Free();
}

bool BuildingContainer::PickLootDrops(UObject* Object, FFrame& Stack, bool* Ret) {
	UObject* WorldContextObject;
	FName TierGroupName;
	int32 WorldLevel;
	int32 ForcedLootTier;

	Stack.StepCompiledIn(&WorldContextObject);
	auto& OutLootToDrop = Stack.StepCompiledInRef<TArray<FFortItemEntry>>();
	Stack.StepCompiledIn(&TierGroupName);
	Stack.StepCompiledIn(&WorldLevel);
	Stack.StepCompiledIn(&ForcedLootTier);
	Stack.IncrementCode();

	for (FFortItemEntry& LootDrop : InternalPickLootDrops(TierGroupName)) OutLootToDrop.Add(LootDrop);

	return *Ret = true;
}

AFortPickup* BuildingContainer::SpawnPickup(UObject* Object, FFrame& Stack, AFortPickup** Ret)
{
	UFortWorldItemDefinition* ItemDefinition;
	int32 NumberToSpawn;
	AFortPawn* TriggeringPawn;
	FVector Position;
	FVector Direction;
	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&NumberToSpawn);
	Stack.StepCompiledIn(&TriggeringPawn);
	Stack.StepCompiledIn(&Position);
	Stack.StepCompiledIn(&Direction);
	Stack.IncrementCode();

	auto Pickup = FortInventory::SpawnPickup(Position, ItemDefinition, NumberToSpawn, ItemDefinition->IsA<UFortWeaponItemDefinition>() ? FortInventory::GetStats((UFortWeaponItemDefinition*)ItemDefinition)->ClipSize : 0, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::SupplyDrop);
	return *Ret = Pickup;
}

void BuildingContainer::Patch()
{
	Runtime::Hook(Runtime::Offsets::ImageBase + 0x13A91C0, SpawnLootHook);
	Runtime::Exec("/Script/FortniteGame.FortKismetLibrary.PickLootDrops", PickLootDrops);
	Runtime::Exec("/Script/FortniteGame.FortAthenaSupplyDrop.SpawnPickup", SpawnPickup);
}
