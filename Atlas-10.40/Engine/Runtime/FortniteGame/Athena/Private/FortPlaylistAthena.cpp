#include "../Public/FortPlaylistAthena.h"

void FortPlaylistAthena::AssignPlaylist(SDK::UFortPlaylistAthena* Playlist, SDK::AFortGameModeAthena* GameMode)
{
    AFortGameStateAthena* GameState = (AFortGameStateAthena*)GameMode->GameState;
    GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
    GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;
    GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
    GameState->CurrentPlaylistInfo.MarkArrayDirty();
    GameState->OnRep_CurrentPlaylistInfo();

    GameState->CurrentPlaylistId = GameMode->CurrentPlaylistId = Playlist->PlaylistId;
    GameState->OnRep_CurrentPlaylistId();

    GameMode->CurrentPlaylistName = Playlist->PlaylistName;

    GameState->OnRep_CurrentPlaylistInfo();
    GameState->OnRep_CurrentPlaylistId();

    GameMode->GameSession->MaxPlayers = Playlist->MaxPlayers;

    GameState->AirCraftBehavior = Playlist->AirCraftBehavior;
    GameState->CachedSafeZoneStartUp = Playlist->SafeZoneStartUp;
    GameState->WorldLevel = Playlist->LootLevel;

    for (auto& Level : Playlist->AdditionalLevels)
    {
        bool bSuccess = false;

        ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(UWorld::GetWorld(), Level, FVector(), FRotator(), &bSuccess);
        if (bSuccess) ((AFortGameStateAthena*)GameMode->GameState)->AdditionalPlaylistLevelsStreamed.Add(Level.ObjectID.AssetPathName);
    }

    ((AFortGameStateAthena*)GameMode->GameState)->OnRep_AdditionalPlaylistLevelsStreamed();
    GameState->OnRep_AdditionalPlaylistLevelsStreamed();
}