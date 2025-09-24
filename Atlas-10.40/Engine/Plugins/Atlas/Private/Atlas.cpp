#include "../Public/Atlas.h"
#include "../../../Runtime/FortniteGame/Athena/Public/FortGameModeAthena.h"
#include "../../../Runtime/Engine/NetDriver/Public/NetDriver.h"
#include "../../../Runtime/FortniteGame/Athena/Public/FortPlayerControllerAthena.h"
#include "../../../Runtime/GameplayAbilities/Public/AbilitySystemComponent.h"
#include "../../../Runtime/FortniteGame/Quests/Public/FortQuestManager.h"
#include "../../../Runtime/FortniteGame/Building/Public/BuildingContainer.h"
#include "../../../Runtime/FortniteGame/Inventory/Public/FortInventory.h"
#include "../../../Runtime/FortniteGame/Athena/Public/FortAthenaVehicle.h"
#include "../../../Runtime/FortniteGame/Building/Public/BuildingSMActor.h"
#include "../../../Runtime/FortniteGame/Player/Public/FortPlayerPawn.h"
#include "../../../Runtime/FortniteGame/Creative/Public/FortAthenaCreativePortal.h"
#include "../../../Runtime/Engine/DispatchRequest/Public/DispatchRequest.h"
#include "../../../Runtime/FortniteGame/Creative/Public/FortCreativeMoveTool.h"
#include "../../../Runtime/FortniteGame/Creative/Public/FortMinigame.h"
#include "../../../Runtime/FortniteGame/Creative/Public/FortMinigameSettingsBuilding.h"
#include "../../../Runtime/Engine/Actor/Public/Actor.h"
#include "Bots.h"
#include "PlayerBots.h"
#include "ServerBotManager.h"
#include "Vehicles.h"

void Atlas::SetState(const std::string& State)
{
	std::wostringstream oss;
	oss << L"10.40 | " << std::wstring(State.begin(), State.end());

	SetConsoleTitleW(oss.str().c_str());
}

void Atlas::Initialize()
{
	Sleep(5000);
	MH_Initialize();

    FortGameModeAthena::Patch();
    NetDriver::Patch();
    FortPlayerControllerAthena::Patch();
    AbilitySystemComponent::Patch();
    FortQuestManager::Patch();
    BuildingContainer::Patch();
    FortInventory::Patch();
    FortAthenaVehicle::Patch();
    BuildingSMActor::Patch();
    FortPlayerPawn::Patch();
    FortAthenaCreativePortal::Patch();
    DispatchRequest::Patch();
    FortCreativeMoveTool::Patch();
    FortMinigame::Patch();
    FortMinigameSettingsBuilding::Patch();
    Actor::Patch();

    Runtime::Hook(Runtime::Offsets::GetMaxTickRate, GetMaxTickRate);
    Runtime::Hook(Runtime::Offsets::ImageBase + 0x17F07B0, KickPlayer);

    Runtime::Patch(Runtime::Offsets::EncryptionPatch, 0x74);
    Runtime::Patch(Runtime::Offsets::GameSessionPatch, 0x85);
    Runtime::Patch(Runtime::Offsets::GameSessionPatch2, 0x85);
    Runtime::Patch(Runtime::Offsets::ImageBase + 0x2190ac0, 0xeb);

    for (auto& RetTrueFunc : Runtime::Offsets::RetTrueFuncs) Runtime::Hook(RetTrueFunc, RetTrue);
    Runtime::_HookVT(AFortPlayerControllerAthena::GetDefaultObj()->VTable, 0x43F, RetTrue);

    for (auto& NullFunc : Runtime::Offsets::NullFuncs) Runtime::Patch(NullFunc, 0xC3);

    // Initialize bot system
    if (UAtlas->bBotsEnabled) {
        ServerBotManager::HookAll();
        Log("Bot system initialized - MaxBots: " + std::to_string(UAtlas->MaxBotsToSpawn));
    }

    // Initialize vehicle system (only if lategame is off)
    if (!UAtlas->bLategame && UAtlas->bVehiclesEnabled) {
        Vehicles::EnableVehicles();
        Log("Vehicle system initialized");
    }
    else if (UAtlas->bLategame) {
        Log("Vehicle system disabled - lategame is on");
    }

	UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);
	*(bool*)Runtime::Offsets::GIsClient = false;
	*(bool*)(Runtime::Offsets::GIsClient + 1) = true;

	UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"open Athena_Terrain", nullptr);
}
