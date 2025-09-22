#pragma once
#include "../Runtime.h"

namespace BuildingSMActor {
	static bool CanBePlacedByPlayer(UClass* BuildClass);

	static void ServerBeginEditingBuildingActor(UObject*, FFrame&);
	static void ServerEditBuildingActor(UObject*, FFrame&);
	static void ServerEndEditingBuildingActor(UObject*, FFrame&);
	static void ServerRepairBuildingActor(UObject*, FFrame&);

	DefHookOg(void, ServerCreateBuildingActor, UObject*, FFrame&);
	DefHookOg(void, OnDamageServer, ABuildingSMActor*, float, FGameplayTagContainer, FVector, FHitResult, AFortPlayerControllerAthena*, AActor*, FGameplayEffectContextHandle);

	void Patch();
}