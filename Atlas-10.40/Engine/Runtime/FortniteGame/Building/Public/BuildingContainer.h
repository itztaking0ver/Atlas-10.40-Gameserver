#pragma once
#include "../Runtime.h"

namespace BuildingContainer
{
	static inline xvector<FFortLootTierData*> TierDataAllGroups;
	static inline xvector<FFortLootPackageData*> LPGroupsAll;

	void SpawnLoot(FName&, FVector);
	void SpawnFloorLootForContainer(UBlueprintGeneratedClass*);

	FFortLootTierData* SelectLootTierData(std::vector<FFortLootTierData*> LootTierDatas);
	FFortLootPackageData* SelectLootPackage(std::vector<FFortLootPackageData*> LootPackages);
	std::vector<FFortItemEntry> InternalPickLootDrops(FName LootTierGroup);

	bool SpawnLootHook(ABuildingContainer*);
	bool PickLootDrops(UObject*, FFrame&, bool*);

	AFortPickup* SpawnPickup(UObject*, FFrame&, AFortPickup**);

	static void SetupLDSForPackage(TArray<FFortItemEntry>&, SDK::FName, int, FName, int WorldLevel = ((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->WorldLevel);
	template <typename T>
	static T* PickWeighted(xvector<T*>& Map, float (*RandFunc)(float), bool bCheckZero = true) {
		float TotalWeight = std::accumulate(Map.begin(), Map.end(), 0.0f, [&](float acc, T*& p) { return acc + p->Weight; });
		float RandomNumber = RandFunc(TotalWeight);

		for (auto& Element : Map)
		{
			float Weight = Element->Weight;
			if (bCheckZero && Weight == 0)
				continue;

			if (RandomNumber <= Weight) return Element;

			RandomNumber -= Weight;
		}

		return nullptr;
	}

	TArray<FFortItemEntry> ChooseLootForContainer(FName, int = -1, int = ((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->WorldLevel);

	void Patch();
}