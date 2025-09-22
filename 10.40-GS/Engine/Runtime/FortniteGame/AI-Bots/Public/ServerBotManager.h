#pragma once
#include "framework.h"

#include "AIController.h"

namespace ServerBotManager {
	AFortPlayerPawnAthena* (*SpawnBotOG)(UFortServerBotManagerAthena* This, FVector SpawnLoc, FRotator SpawnRot, UFortAthenaAIBotCustomizationData* BotData);
	AFortPlayerPawnAthena* SpawnBot(UFortServerBotManagerAthena* This, FVector SpawnLoc, FRotator SpawnRot, UFortAthenaAIBotCustomizationData* BotData)
	{
		auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
		AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

		//Log("SpawnBot Called!");
		AFortPlayerPawnAthena* Pawn = SpawnBotOG(This, SpawnLoc, SpawnRot, BotData);
		AFortAthenaAIBotController* PC = (AFortAthenaAIBotController*)Pawn->Controller;
		if (!PC) {
			Log("UFortServerBotManagerAthena::SpawnBot, Spawning New Controller!");
			PC = SpawnActor<AFortAthenaAIBotController>({});
			PC->Possess(Pawn);
		}
		AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
		if (!PC || !Pawn) {
			Log("UFortServerBotManagerAthena::SpawnBot Failed! Unable to spawn pawn or get controller!");
			return Pawn;
		}

		if (!Characters.empty()) {
			auto CID = Characters[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, Characters.size() - 1)];
			if (CID->HeroDefinition)
			{
				if (CID->HeroDefinition->Specializations.IsValid())
				{
					for (size_t i = 0; i < CID->HeroDefinition->Specializations.Num(); i++)
					{
						UFortHeroSpecialization* Spec = StaticLoadObject<UFortHeroSpecialization>(UKismetStringLibrary::GetDefaultObj()->Conv_NameToString(CID->HeroDefinition->Specializations[i].ObjectID.AssetPathName).ToString());
						if (Spec)
						{
							for (size_t j = 0; j < Spec->CharacterParts.Num(); j++)
							{
								UCustomCharacterPart* Part = StaticLoadObject<UCustomCharacterPart>(UKismetStringLibrary::GetDefaultObj()->Conv_NameToString(Spec->CharacterParts[j].ObjectID.AssetPathName).ToString());
								if (Part)
								{
									PlayerState->CharacterData.Parts[(uintptr_t)Part->CharacterPartType] = Part;
								}
							}
						}
					}
				}
			}
			if (CID) {
				Pawn->CosmeticLoadout.Character = CID;
			}
		}
		if (!Backpacks.empty() && UKismetMathLibrary::GetDefaultObj()->RandomBoolWithWeight(0.5)) { // less likely to equip than skin cause lots of ppl prefer not to use backpack
			auto Backpack = Backpacks[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, Backpacks.size() - 1)];
			for (size_t j = 0; j < Backpack->CharacterParts.Num(); j++)
			{
				UCustomCharacterPart* Part = Backpack->CharacterParts[j];
				if (Part)
				{
					PlayerState->CharacterData.Parts[(uintptr_t)Part->CharacterPartType] = Part;
				}
			}
		}
		if (!Gliders.empty()) {
			auto Glider = Gliders[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, Gliders.size() - 1)];
			Pawn->CosmeticLoadout.Glider = Glider;
		}
		if (!Contrails.empty() && UKismetMathLibrary::GetDefaultObj()->RandomBoolWithWeight(0.95)) {
			auto Contrail = Contrails[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, Contrails.size() - 1)];
			Pawn->CosmeticLoadout.SkyDiveContrail = Contrail;
		}
		for (size_t i = 0; i < Dances.size(); i++)
		{
			Pawn->CosmeticLoadout.Dances.Add(Dances.at(i));
		}

		Pawn->OnRep_CosmeticLoadout();
		PlayerState->OnRep_CharacterData();

		PC->BehaviorTree = BotData->BehaviorTree;
		PC->RunBehaviorTree(BotData->BehaviorTree);

		Pawn->SetMaxHealth(100.f);
		Pawn->SetMaxShield(100.f);
		Pawn->SetHealth(100.f);
		Pawn->SetShield(0.f);

		if (!PC->Inventory) {
			// Until i hook SetUpInventoryBot
			//Log("Creating inv for bot!");
			PC->Inventory = SpawnActor<AFortInventory>({});
			PC->Inventory->Owner = PC;
			PC->Inventory->OnRep_Owner();
		}

		PlayerBots::PlayerBot* Bot = new PlayerBots::PlayerBot(Pawn, PC, PlayerState);

		if (PC->Inventory) {
			for (auto item : BotData->StartupInventory->Items)
			{
				if (item)
				{
					Bot->GiveItem(item, 1, 0);
				}
			}
		}
		else {
			Log("Bruh!!!");
		}

		Bot->EquipPickaxe();

		return Bot->Pawn;
	}

	void HookAll() {
		MH_CreateHook((LPVOID)(ImageBase + 0x12B1C50), SpawnBot, (LPVOID*)&SpawnBotOG);

		Log("ServerBotManager Hooked!");
	}
}