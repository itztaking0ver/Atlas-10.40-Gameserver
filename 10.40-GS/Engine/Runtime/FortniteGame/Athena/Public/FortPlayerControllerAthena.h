#pragma once
#include "../Runtime.h"

namespace FortPlayerControllerAthena
{
    static void ServerGiveCreativeItem(AFortPlayerControllerAthena* Controller, const struct FFortItemEntry& CreativeItem);
    static void ServerSaveIslandCheckpoint(AFortPlayerControllerAthena* Controller, AFortAthenaCreativePortal* Portal);
    static void UpdateCreativePlotData(AFortPlayerControllerAthena* Controller, AFortVolume* VolumeToPublish, FCreativeIslandInfo& MyInfo);
    static void ServerUpdateActorOptions(AFortPlayerControllerAthena* Controller, AActor* OptionsTarget, TArray<FString>& OptionsKeys, TArray<FString>& OptionsValues);
    static void ServerStartLoadingVolume(AFortPlayerControllerAthena* Controller, AFortVolume* VolumeToLoad);
    static void ServerSendClientProgressUpdate(AFortPlayerControllerAthena* Controller, int32 ClientProgressState, TArray<uint64>& ClientProgressUpdate);

    DefHookOg(void, ServerEndMinigame, AFortPlayerControllerAthena* PlayerController, FFrame& Stack);
    DefHookOg(void, ServerStartMinigame, AFortPlayerControllerAthena* PlayerController, FFrame& Stack);

    void InternalPickup(AFortPlayerControllerAthena* PlayerController, FFortItemEntry PickupEntry);
    void ServerLoadPlotForPortal(AFortPlayerControllerAthena* Controller, AFortAthenaCreativePortal* Portal, FString& PlotItemId);

    DefHookOg(void, ClientOnPawnDied, AFortPlayerControllerAthena*, FFortPlayerDeathReport&);
    DefHookOg(void, NetMulticast_Athena_BatchedDamageCues, AFortPlayerPawnAthena*, FAthenaBatchedDamageGameplayCues_Shared, FAthenaBatchedDamageGameplayCues_NonShared);

    void Patch();
}