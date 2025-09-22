#include "../Public/FortAthenaCreativePortal.h"
#include "../../Inventory/Public/FortInventory.h"
#include "../../../../Plugins/Crystal/Public/Crystal.h"
#include "../../../../Plugins/Crystal/Lategame/Public/Lategame.h"

// scuffed phone system and islands
static std::vector<std::string> Islands;

void FortAthenaCreativePortal::TeleportPlayerToLinkedVolume(AFortAthenaCreativePortal* Portal, FFrame& Stack)
{
    AFortPlayerPawn* PlayerPawn = nullptr;
    bool bUseSpawnTags = false;

    Stack.StepCompiledIn(&PlayerPawn);
    Stack.StepCompiledIn(&bUseSpawnTags);
    Stack.IncrementCode();

    if (!PlayerPawn || !Portal || !Portal->LinkedVolume) return;

    auto Volume = Portal->LinkedVolume;
    auto Location = Volume->K2_GetActorLocation();
    Location.Z = 10000;

    PlayerPawn->K2_TeleportTo(Location, FRotator());
    PlayerPawn->BeginSkydiving(false);

    auto Controller = static_cast<AFortPlayerControllerAthena*>(PlayerPawn->Controller);
    if (!Controller || !Controller->PlayerState) return;

    FortInventory::GiveItem(Controller, Lategame::GetResource(EFortResourceType::Wood), 500);
    FortInventory::GiveItem(Controller, Lategame::GetResource(EFortResourceType::Stone), 500);
    FortInventory::GiveItem(Controller, Lategame::GetResource(EFortResourceType::Metal), 500);

    auto Phone = Runtime::StaticFindObject<UFortItemDefinition>(
        "/Game/Athena/Items/Weapons/Prototype/WID_CreativeTool.WID_CreativeTool"
    );

    bool bHasPhone = false;

    for (auto& Entry : Controller->WorldInventory->Inventory.ReplicatedEntries) {
        if (Entry.ItemDefinition == Phone) {
            bHasPhone = true;
            break;
        }
    }

    if (!bHasPhone) {
        FortInventory::GiveItem(Controller, Phone);
    }

    auto PlayerName = Controller->PlayerState->GetPlayerName().ToString();

    if (std::find(Islands.begin(), Islands.end(), PlayerName) == Islands.end())
    {
        Islands.push_back(PlayerName);

        //FortVolume::LoadIsland(Controller->CreativePlotLinkedVolume, "Island");
    }

    Controller->bBuildFree = true;
}

void FortAthenaCreativePortal::ServerTeleportToPlaygroundLobbyIsland(AFortPlayerControllerAthena* Controller)
{
    if (!Controller) return;

    auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
    auto GameState = (AFortGameStateAthena*)GameMode->GameState;

    if (Controller->WarmupPlayerStart) Controller->GetPlayerPawn()->K2_TeleportTo(Controller->WarmupPlayerStart->K2_GetActorLocation(), Controller->GetPlayerPawn()->K2_GetActorRotation());
    else
    {
        AActor* Actor = GameMode->ChoosePlayerStart(Controller);
        Controller->GetPlayerPawn()->K2_TeleportTo(Actor->K2_GetActorLocation(), Actor->K2_GetActorRotation());
    }

    Controller->bBuildFree = false;
}

void FortAthenaCreativePortal::Patch()
{
    if (UCrystal->bCreative) {
        Runtime::Exec("/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayerToLinkedVolume", TeleportPlayerToLinkedVolume);
        Runtime::Exec("/Script/FortniteGame.FortPlayerControllerAthena.ServerTeleportToPlaygroundLobbyIsland", ServerTeleportToPlaygroundLobbyIsland);
    }
}
