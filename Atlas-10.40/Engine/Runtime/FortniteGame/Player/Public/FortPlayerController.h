#pragma once
#include "../Runtime.h"

namespace FortPlayerController
{
    static void ServerAcknowledgePossession(UObject*, FFrame&);
    static void ServerExecuteInventoryItem(UObject*, FFrame&);
    static void ServerExecuteInventoryWeapon(UObject*, FFrame&);
    static void ServerHandlePickup(UObject*, FFrame&);
    static void ServerClientIsReadyToRespawn(UObject*, FFrame&);
    static void TeleportPlayer(UObject*, FFrame&);

    static void ServerPlayEmoteItem(UObject*, FFrame&);
    static void ReloadWeapon(AFortWeapon* Weapon, int AmmoToRemove);
    static void ServerAttemptInventoryDrop(UObject*, FFrame&);
    static void ServerUpdateGameplayOptions(UObject* Context, FFrame& Stack);

    void ServerCheat(AFortPlayerController* Controller, FString Message);
    void OnCapsuleBeginOverlap(UObject* Context, FFrame& Stack);

    DefHookOg(void, ServerCreativeSetFlightSpeedIndex, UObject*, FFrame&);
    DefHookOg(void, ServerAttemptAircraftJump, UObject*, FFrame&);
    DefHookOg(void, MakeNewCreativePlot, UObject* Context, FFrame& Stack);
    DefHookOg(bool, CompletePickupAnimation, AFortPickup*);
    DefHookOg(void, Athena_MedConsumable_Triggered, UObject* Context, FFrame& Stack);
    DefHookOg(void, ServerAttemptInteract, UFortControllerComponent_Interaction* Interaction, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, ETInteractionType InteractType, UObject* OptionalObjectData);
    DefHookOg(void, OnCapsuleBeginOverlap, UObject* Context, FFrame& Stack);
    DefHookOg(void, GetPlayerViewPoint, AFortPlayerController*, FVector&, FRotator&);

    DefUHookOgRet(void, ServerHandlePickupWithSwap);
    DefUHookOgRet(void, ServerLoadingScreenDropped);

    void Patch();
}