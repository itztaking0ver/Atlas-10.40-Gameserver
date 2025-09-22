#include "../Public/FortPlayerController.h"
#include "../../../GameplayAbilities/Public/AbilitySystemComponent.h"
#include "../../../../Plugins/Crystal/Public/Crystal.h"
#include "../../Inventory/Public/FortInventory.h"
#include "../../../../Plugins/Crystal/Lategame/Public/Lategame.h"

void FortPlayerController::ServerCheat(AFortPlayerController* Controllerss, FString Message)
{
    auto Controller = (AFortPlayerControllerAthena*)Controllerss;
    if (!Controller) return;
}

void FortPlayerController::ServerAcknowledgePossession(UObject* Context, FFrame& Stack)
{
    APawn* Pawn;
    Stack.StepCompiledIn(&Pawn);
    Stack.IncrementCode();
    auto PlayerController = (AFortPlayerController*)Context;
    PlayerController->AcknowledgedPawn = Pawn;

    ((AFortPlayerStateAthena*)PlayerController->PlayerState)->HeroType = PlayerController->CosmeticLoadoutPC.Character->HeroDefinition;
    ((void (*)(APlayerState*, APawn*)) Runtime::Offsets::ApplyCharacterCustomization)(PlayerController->PlayerState, PlayerController->Pawn);

    AbilitySystemComponent::GiveAbilitySet(((AFortPlayerStateAthena*)PlayerController->PlayerState)->AbilitySystemComponent, Runtime::StaticFindObject<UFortAbilitySet>("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer"));
}

void FortPlayerController::GetPlayerViewPoint(AFortPlayerController* PlayerController, FVector& Loc, FRotator& Rot) {
    static auto State = FName(L"Spectating");

    if (PlayerController->StateName == State)
    {
        Loc = PlayerController->LastSpectatorSyncLocation;
        Rot = PlayerController->LastSpectatorSyncRotation;
    }
    else if (PlayerController->GetViewTarget())
    {
        Loc = PlayerController->GetViewTarget()->K2_GetActorLocation();
        Rot = PlayerController->GetControlRotation();
    }

    else return GetPlayerViewPointOG(PlayerController, Loc, Rot);
}

void FortPlayerController::ServerLoadingScreenDropped(UObject* Context, FFrame& Stack, void*)
{
    Stack.IncrementCode();
    auto PlayerController = (AFortPlayerController*)Context;
    return callOG(PlayerController, "/Script/FortniteGame.FortPlayerController", ServerLoadingScreenDropped);
}

void FortPlayerController::ServerAttemptInteract(UFortControllerComponent_Interaction* Interaction, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, ETInteractionType InteractType, UObject* OptionalObjectData)
{
    return ServerAttemptInteractOG(Interaction, ReceivingActor, InteractComponent, InteractType, OptionalObjectData);
}

void FortPlayerController::ServerExecuteInventoryItem(UObject* Context, FFrame& Stack)
{
    FGuid ItemGuid;
    Stack.StepCompiledIn(&ItemGuid);
    Stack.IncrementCode();

    auto PlayerController = (AFortPlayerController*)Context;
    if (!PlayerController) return;

    auto Entry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& Ent) { return Ent.ItemGuid == ItemGuid; });
    if (!Entry || !PlayerController->MyFortPawn) return;

    UFortWeaponItemDefinition* ItemDefinition = Entry->ItemDefinition->IsA<UFortGadgetItemDefinition>() ? ((UFortGadgetItemDefinition*)Entry->ItemDefinition)->GetWeaponItemDefinition() : (UFortWeaponItemDefinition*)Entry->ItemDefinition;

    if (auto Deco = (UFortContextTrapItemDefinition*)ItemDefinition->Cast<UFortDecoItemDefinition>()) {
        PlayerController->MyFortPawn->PickUpActor(nullptr, Deco);
        PlayerController->MyFortPawn->CurrentWeapon->ItemEntryGuid = ItemGuid;

        if (auto ContextTrap = PlayerController->MyFortPawn->CurrentWeapon->Cast<AFortDecoTool_ContextTrap>()) ContextTrap->ContextTrapItemDefinition = Deco;
        return;
    }

    PlayerController->MyFortPawn->EquipWeaponDefinition(ItemDefinition, ItemGuid);
}

void FortPlayerController::ServerExecuteInventoryWeapon(UObject* Context, FFrame& Stack)
{
    AFortWeapon* Weapon;
    Stack.StepCompiledIn(&Weapon);
    Stack.IncrementCode();
    auto PlayerController = (AFortPlayerController*)Context;
    if (!PlayerController) return;

    auto Entry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& Ent) { return Ent.ItemGuid == Weapon->ItemEntryGuid; });
    if (!Entry || !PlayerController->MyFortPawn) return;

    UFortWeaponItemDefinition* ItemDefinition = Entry->ItemDefinition->IsA<UFortGadgetItemDefinition>() ? ((UFortGadgetItemDefinition*)Entry->ItemDefinition)->GetWeaponItemDefinition() : (UFortWeaponItemDefinition*)Entry->ItemDefinition;
    if (auto Deco = (UFortContextTrapItemDefinition*)ItemDefinition->Cast<UFortDecoItemDefinition>()) {
        PlayerController->MyFortPawn->PickUpActor(nullptr, Deco);
        PlayerController->MyFortPawn->CurrentWeapon->ItemEntryGuid = Weapon->ItemEntryGuid;

        if (auto ContextTrap = PlayerController->MyFortPawn->CurrentWeapon->Cast<AFortDecoTool_ContextTrap>()) ContextTrap->ContextTrapItemDefinition = Deco;
        return;
    }

    PlayerController->MyFortPawn->EquipWeaponDefinition(ItemDefinition, Weapon->ItemEntryGuid);
}

void FortPlayerController::ServerHandlePickup(UObject* Context, FFrame& Stack)
{
    AFortPickup* Pickup;
    float InFlyTime;
    FVector InStartDirection;
    bool bPlayPickupSound;
    Stack.StepCompiledIn(&Pickup);
    Stack.StepCompiledIn(&InFlyTime);
    Stack.StepCompiledIn(&InStartDirection);
    Stack.StepCompiledIn(&bPlayPickupSound);
    Stack.IncrementCode();

    auto Pawn = (AFortPlayerPawn*)Context;
    if (!Pawn || !Pickup || Pickup->bPickedUp) return;

    if (Pawn->GetDistanceTo(Pickup) > 500.f) return;

    Pawn->IncomingPickups.Add(Pickup);

    Pickup->PickupLocationData.bPlayPickupSound = bPlayPickupSound;
    Pickup->PickupLocationData.FlyTime = 0.4f;
    Pickup->PickupLocationData.ItemOwner = Pawn;
    Pickup->PickupLocationData.PickupGuid = Pickup->PrimaryPickupItemEntry.ItemGuid;
    Pickup->PickupLocationData.PickupTarget = Pawn;
    Pickup->PickupLocationData.StartDirection = (FVector_NetQuantizeNormal)InStartDirection;
    Pickup->OnRep_PickupLocationData();

    Pickup->bPickedUp = true;
    Pickup->OnRep_bPickedUp();
}

void FortPlayerController::ReloadWeapon(AFortWeapon* Weapon, int AmmoToRemove)
{
    if (!Weapon) return;

    AFortPlayerControllerAthena* Controller = (AFortPlayerControllerAthena*)((AFortPlayerPawnAthena*)Weapon->Owner)->Controller;
    if (!Controller) return;

    auto Inventory = Controller->WorldInventory;
    if (!Inventory) return;

    auto Ammo = Weapon->WeaponData->GetAmmoWorldItemDefinition_BP();
    auto Entry = Inventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& Ent) { return Weapon->WeaponData == Ammo ? Ent.ItemGuid == Weapon->ItemEntryGuid : Ent.ItemDefinition == Ammo; });

    auto WeaponEntry = Inventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& Ent) { return Ent.ItemGuid == Weapon->ItemEntryGuid; });
    if (!WeaponEntry) return;

    if (Entry) {
        Entry->Count -= AmmoToRemove;
        if (Entry->Count >= 0) {
            FortInventory::Remove(Controller, Entry->ItemGuid);
        }
        else {
            FortInventory::ReplaceEntry(Controller, *Entry);
        }
    }

    WeaponEntry->LoadedAmmo -= AmmoToRemove;
    FortInventory::ReplaceEntry(Controller, *WeaponEntry);
}

void FortPlayerController::ServerPlayEmoteItem(UObject* Context, FFrame& Stack)
{
    UFortMontageItemDefinitionBase* Asset;
    Stack.StepCompiledIn(&Asset);
    Stack.IncrementCode();
    auto PlayerController = (AFortPlayerController*)Context;
    if (!PlayerController || !PlayerController->MyFortPawn || !Asset) return;

    auto AbilitySystemComponent = ((AFortPlayerStateAthena*)PlayerController->PlayerState)->AbilitySystemComponent;
    FGameplayAbilitySpec NewSpec;
    UGameplayAbility* Ability = nullptr;

    if (Asset->Cast<UAthenaSprayItemDefinition>()) {
        Ability = UGAB_Spray_Generic_C::GetDefaultObj();
    }
    else if (Asset->Cast<UAthenaDanceItemDefinition>())
    {
        auto Emote = Asset->Cast<UAthenaDanceItemDefinition>();
        auto DA = Emote->CustomDanceAbility;
        Ability = DA ? (UGameplayAbility*)DA->DefaultObject : UGAB_Emote_Generic_C::GetDefaultObj();
        PlayerController->MyFortPawn->bMovingEmote = Emote->bMovingEmote;
        PlayerController->MyFortPawn->bMovingEmoteForwardOnly = Emote->bMoveForwardOnly;
        PlayerController->MyFortPawn->EmoteWalkSpeed = Emote->WalkForwardSpeed;
    }

    if (Ability) {
        ((void (*)(FGameplayAbilitySpec*, UObject*, int, int, UObject*))(Runtime::Offsets::ConstructAbilitySpec))(&NewSpec, Ability, 1, -1, Asset);
        FGameplayAbilitySpecHandle handle;
        ((void (*)(UFortAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec*, void*))(Runtime::Offsets::ImageBase + 0x935130))(AbilitySystemComponent, &handle, &NewSpec, nullptr);
    }
}

void FortPlayerController::ServerAttemptInventoryDrop(UObject* Context, FFrame& Stack)
{
    FGuid Guid;
    int32 Count;

    Stack.StepCompiledIn(&Guid);
    Stack.StepCompiledIn(&Count);
    Stack.IncrementCode();

    auto PlayerController = (AFortPlayerControllerAthena*)Context;
    if (!PlayerController || !PlayerController->Pawn) return;

    auto ItemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry) { return entry.ItemGuid == Guid; });
    if (!ItemEntry || (ItemEntry->Count - Count) < 0) return;

    ItemEntry->Count -= Count;
    FortInventory::SpawnPickup(PlayerController->Pawn->K2_GetActorLocation() + PlayerController->Pawn->GetActorForwardVector() * 70.f + FVector(0, 0, 50), *ItemEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PlayerController->MyFortPawn, Count);
    if (ItemEntry->Count == 0) {
        FortInventory::Remove(PlayerController, Guid);
    }
    else {
        FortInventory::ReplaceEntry(PlayerController, *ItemEntry);
    }
}

void FortPlayerController::ServerUpdateGameplayOptions(UObject* Context, FFrame& Stack)
{
    TArray<FString> UserOptionsKeys;
    TArray<FString> UserOptionsValues;

    Stack.StepCompiledIn(&UserOptionsKeys);
    Stack.StepCompiledIn(&UserOptionsValues);
    Stack.IncrementCode();

    auto Controller = (AFortPlayerControllerAthena*)Context;
    if (!Controller) return;

    auto PlayerState = (AFortPlayerStateAthena*)Controller->PlayerState;
    if (UserOptionsKeys.Num() != UserOptionsValues.Num()) return;

    UFortLevelSaveComponent* Save = (UFortLevelSaveComponent*)Controller->CreativePlotLinkedVolume->GetComponentByClass(UFortLevelSaveComponent::StaticClass());

    if (AFortMinigame* Minigame = Controller->GetMinigame())
    {
        if (Minigame->CurrentState >= EFortMinigameState::Setup && Minigame->CurrentState <= EFortMinigameState::PostGameAbandon) return;
    }

    auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;

    UFortMutatorListComponent* MutatorList = GameMode->MutatorListComponent;
    UPlaylistUserOptions* UserOptions = nullptr;
    if (MutatorList && MutatorList->UserOptions)
    {
        UserOptions = MutatorList->UserOptions;
    }
    else
    {
        return;
    }

    TMap<FString, FString> NewOptions;
    for (int32 i = 0; i < UserOptionsKeys.Num(); ++i) NewOptions.Add(UserOptionsKeys[i], UserOptionsKeys[i]);

    if (MutatorList)
    {
        MutatorList->SetPropertyOverrides(NewOptions);
    }
}

void FortPlayerController::ServerHandlePickupWithSwap(UObject* Context, FFrame& Stack, void*)
{
    AFortPickup* Pickup;
    FGuid Swap;
    float InFlyTime;
    FVector InStartDirection;
    bool bPlayPickupSound;
    Stack.StepCompiledIn(&Pickup);
    Stack.StepCompiledIn(&Swap);
    Stack.StepCompiledIn(&InFlyTime);
    Stack.StepCompiledIn(&InStartDirection);
    Stack.StepCompiledIn(&bPlayPickupSound);
    Stack.IncrementCode();
    auto Pawn = (AFortPlayerPawnAthena*)Context;
    if (!Pawn || !Pickup || Pickup->bPickedUp || FortInventory::GetQuickbar(Pickup->PrimaryPickupItemEntry.ItemDefinition) == EFortQuickBars::Secondary) return;
    auto PlayerController = Pawn->Controller->Cast<AFortPlayerControllerAthena>();
    if (!PlayerController) return;

    auto Entry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& Ent) {
        return Ent.ItemGuid == Swap && FortInventory::GetQuickbar(Pickup->PrimaryPickupItemEntry.ItemDefinition) == EFortQuickBars::Primary && !Ent.ItemDefinition->IsA<UFortWeaponMeleeItemDefinition>();
        });
    if (!Entry) return;

    PlayerController->SwappingItemDefinition = (UFortWorldItemDefinition*)Entry;

    return Pawn->ServerHandlePickup(Pickup, InFlyTime, InStartDirection, bPlayPickupSound);
}

void FortPlayerController::ServerAttemptAircraftJump(UObject* Context, FFrame& Stack)
{
    FRotator Rotation;
    Stack.StepCompiledIn(&Rotation);
    Stack.IncrementCode();

    auto PlayerController = (AFortPlayerController*)Context;
    auto GameState = UWorld::GetWorld()->GameState->Cast<AFortGameStateAthena>();
    auto GameMode = UWorld::GetWorld()->AuthorityGameMode->Cast<AFortGameModeAthena>();

    if (!UCrystal->bLategame) callOG(PlayerController, "/Script/FortniteGame.FortPlayerController", ServerAttemptAircraftJump, Rotation);
    else {
        GameMode->RestartPlayer(PlayerController);
        if (PlayerController->MyFortPawn) {
            PlayerController->MyFortPawn->BeginSkydiving(true);
            auto SpawnLocation = GameState->Aircrafts[0]->K2_GetActorLocation();
            SpawnLocation.Z -= 300;
            SpawnLocation.X += UKismetMathLibrary::RandomInteger(300);
            SpawnLocation.Y += UKismetMathLibrary::RandomInteger(300);

            PlayerController->MyFortPawn->K2_TeleportTo(SpawnLocation, {});
        }
    }

    if (UCrystal->bLategame && PlayerController->MyFortPawn) {
        PlayerController->MyFortPawn->SetHealth(100);
        PlayerController->MyFortPawn->SetShield(100);

        auto Shotgun = Lategame::GetShotguns();
        auto AssaultRifle = Lategame::GetAssaultRifles();
        auto Sniper = Lategame::GetSnipers();
        auto Heal = Lategame::GetHeals();
        auto HealSlot2 = Lategame::GetHeals();

        int ShotgunClipSize = FortInventory::GetStats((UFortWeaponItemDefinition*)Shotgun.Item)->ClipSize;
        int AssaultRifleClipSize = FortInventory::GetStats((UFortWeaponItemDefinition*)AssaultRifle.Item)->ClipSize;
        int SniperClipSize = FortInventory::GetStats((UFortWeaponItemDefinition*)Sniper.Item)->ClipSize;

        int HealClipSize = Heal.Item->IsA<UFortWeaponItemDefinition>() ? FortInventory::GetStats((UFortWeaponItemDefinition*)Heal.Item)->ClipSize : 0;
        int HealSlot2ClipSize = HealSlot2.Item->IsA<UFortWeaponItemDefinition>() ? FortInventory::GetStats((UFortWeaponItemDefinition*)HealSlot2.Item)->ClipSize : 0;

        FortInventory::GiveItem(PlayerController, Lategame::GetResource(EFortResourceType::Wood), 500);
        FortInventory::GiveItem(PlayerController, Lategame::GetResource(EFortResourceType::Stone), 500);
        FortInventory::GiveItem(PlayerController, Lategame::GetResource(EFortResourceType::Metal), 500);

        FortInventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Assault), 250);
        FortInventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Shotgun), 50);
        FortInventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Submachine), 400);
        FortInventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Rocket), 6);
        FortInventory::GiveItem(PlayerController, Lategame::GetAmmo(EAmmoType::Sniper), 20);

        UFortWorldItem* Entries[5]{};
        Entries[0] = FortInventory::GiveItem(PlayerController, AssaultRifle.Item, AssaultRifle.Count, AssaultRifleClipSize, true, false);
        Entries[1] = FortInventory::GiveItem(PlayerController, Shotgun.Item, Shotgun.Count, ShotgunClipSize, true, false);
        Entries[2] = FortInventory::GiveItem(PlayerController, Sniper.Item, Sniper.Count, SniperClipSize, true, false);
        Entries[4] = FortInventory::GiveItem(PlayerController, Heal.Item, Heal.Count, HealClipSize, true, false);
        Entries[3] = FortInventory::GiveItem(PlayerController, HealSlot2.Item, HealSlot2.Count, HealSlot2ClipSize, true, false);

        for (auto& Entry : Entries) {
            FortInventory::TriggerInventoryUpdate(PlayerController, &Entry->ItemEntry);
        }
    }
}

void FortPlayerController::ServerClientIsReadyToRespawn(UObject* Context, FFrame& Stack)
{
    Stack.IncrementCode();
    auto PlayerController = (AFortPlayerControllerAthena*)Context;
    auto PlayerState = PlayerController->PlayerState->Cast<AFortPlayerStateAthena>();

    if (PlayerState->RespawnData.bRespawnDataAvailable && PlayerState->RespawnData.bServerIsReady)
    {
        PlayerState->RespawnData.bClientIsReady = true;

        FTransform Transform(PlayerState->RespawnData.RespawnLocation, PlayerState->RespawnData.RespawnRotation);
        auto Pawn = (AFortPlayerPawnAthena*)UWorld::GetWorld()->AuthorityGameMode->SpawnDefaultPawnAtTransform(PlayerController, Transform);
        PlayerController->Possess(Pawn);
        Pawn->SetHealth(100);

        PlayerController->RespawnPlayerAfterDeath(true);
        Pawn->BeginSkydiving(true);
    }
}

void FortPlayerController::TeleportPlayer(UObject* Context, FFrame& Frame)
{
    AFortPlayerPawn* PlayerPawn = nullptr;
    FRotator TeleportRotation;
    Frame.StepCompiledIn(&PlayerPawn);
    Frame.StepCompiledIn(&TeleportRotation);
    Frame.IncrementCode();

    if (!PlayerPawn) return;

    PlayerPawn->K2_TeleportTo(Context->Cast<AFortAthenaCreativePortal>()->TeleportLocation, TeleportRotation);
    FortInventory::GiveItem((AFortPlayerController*)PlayerPawn->Controller, Runtime::StaticFindObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/Prototype/WID_CreativeTool.WID_CreativeTool"));
}

void FortPlayerController::Athena_MedConsumable_Triggered(UObject* Context, FFrame& Stack)
{
    UGA_Athena_MedConsumable_Parent_C* Consumable = (UGA_Athena_MedConsumable_Parent_C*)Context;

    if (!Consumable || (!Consumable->HealsShields && !Consumable->HealsHealth) || !Consumable->PlayerPawn || Consumable->Class == UGA_Athena_Bandage_C::StaticClass()) return Athena_MedConsumable_TriggeredOG(Context, Stack);

    auto Handle = Consumable->PlayerPawn->AbilitySystemComponent->MakeEffectContext();

    FGameplayTag Tag{};
    static auto ShieldCue = FName(L"GameplayCue.Shield.PotionConsumed");
    static auto HealthCue = FName(L"GameplayCue.Athena.Health.HealUsed");
    FName CueName = Consumable->HealsShields ? ShieldCue : HealthCue;
    Tag.TagName = CueName;
    Consumable->PlayerPawn->AbilitySystemComponent->NetMulticast_InvokeGameplayCueAdded(Tag, FPredictionKey(), Handle);
    Consumable->PlayerPawn->AbilitySystemComponent->NetMulticast_InvokeGameplayCueExecuted(Tag, FPredictionKey(), Handle);

    return Athena_MedConsumable_TriggeredOG(Context, Stack);
}

void FortPlayerController::OnCapsuleBeginOverlap(UObject* Context, FFrame& Stack)
{
    UPrimitiveComponent* OverlappedComp;
    AActor* OtherActor;
    UPrimitiveComponent* OtherComp;
    int32 OtherBodyIndex;
    bool bFromSweep;
    FHitResult SweepResult;
    Stack.StepCompiledIn(&OverlappedComp);
    Stack.StepCompiledIn(&OtherActor);
    Stack.StepCompiledIn(&OtherComp);
    Stack.StepCompiledIn(&OtherBodyIndex);
    Stack.StepCompiledIn(&bFromSweep);
    Stack.StepCompiledIn(&SweepResult);
    Stack.IncrementCode();

    auto Pawn = (AFortPlayerPawn*)Context;
    if (!Pawn || !Pawn->Controller) return callOG(Pawn, "/Script/FortniteGame.FortPlayerPawn", OnCapsuleBeginOverlap, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    auto Pickup = OtherActor->Cast<AFortPickup>();
    if (!Pickup || !Pickup->PrimaryPickupItemEntry.ItemDefinition) return callOG(Pawn, "/Script/FortniteGame.FortPlayerPawn", OnCapsuleBeginOverlap, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    auto MaxStack = Pickup->PrimaryPickupItemEntry.ItemDefinition->MaxStackSize;
    auto itemEntry = ((AFortPlayerControllerAthena*)Pawn->Controller)->WorldInventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& Ent) { return Ent.ItemDefinition == Pickup->PrimaryPickupItemEntry.ItemDefinition && Ent.Count <= MaxStack; });

    if (Pickup && Pickup->PawnWhoDroppedPickup != Pawn)
    {
        if ((!itemEntry && FortInventory::GetQuickbar(Pickup->PrimaryPickupItemEntry.ItemDefinition) == EFortQuickBars::Secondary) || (itemEntry && itemEntry->Count < MaxStack)) {
            Pawn->ServerHandlePickup(Pickup, 0.4f, FVector(), true);
        }
    }

    return callOG(Pawn, "/Script/FortniteGame.FortPlayerPawn", OnCapsuleBeginOverlap, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void FortPlayerController::MakeNewCreativePlot(UObject* Context, FFrame& Stack)
{
    UFortCreativeRealEstatePlotItemDefinition* PlotType;
    FString Locale;
    FString Title;

    Stack.StepCompiledIn(&PlotType);
    Stack.StepCompiledIn(&Locale);
    Stack.StepCompiledIn(&Title);
    Stack.IncrementCode();

    auto Controller = (AFortPlayerControllerAthena*)Context;
    if (!Controller) return;

    return MakeNewCreativePlotOG(Context, Stack);
}

void FortPlayerController::ServerCreativeSetFlightSpeedIndex(UObject* Context, FFrame& Stack)
{
    int Index;
    Stack.StepCompiledIn(&Index);
    Stack.IncrementCode();

    auto PlayerController = (AFortPlayerControllerAthena*)Context;
    PlayerController->FlyingModifierIndex = Index;
    PlayerController->OnRep_FlyingModifierIndex();

    return callOG(PlayerController, "/Script/FortniteGame.FortPlayerControllerGameplay", ServerCreativeSetFlightSpeedIndex, Index);
}

void FortPlayerController::Patch()
{
    Runtime::Exec("/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayer", TeleportPlayer);
    Runtime::Exec("/Script/FortniteGame.FortPlayerControllerGameplay.ServerCreativeSetFlightSpeedIndex", ServerCreativeSetFlightSpeedIndex, ServerCreativeSetFlightSpeedIndexOG);
    Runtime::Exec("/Script/FortniteGame.FortPlayerControllerAthena.ServerClientIsReadyToRespawn", ServerClientIsReadyToRespawn);
    Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerPlayEmoteItem", ServerPlayEmoteItem);

    Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerAttemptInventoryDrop", ServerAttemptInventoryDrop);
    Runtime::Exec("/Script/FortniteGame.FortControllerComponent_Interaction.ServerAttemptInteract", ServerAttemptInteract, ServerAttemptInteractOG);
    Runtime::Exec("/Script/FortniteGame.FortPlayerPawn.ServerHandlePickup", ServerHandlePickup);
    Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerExecuteInventoryWeapon", ServerExecuteInventoryWeapon);

    Runtime::Exec("/Script/FortniteGame.FortPlayerPawn.ServerHandlePickupWithRequestedSwap", ServerHandlePickupWithSwap);
    Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerLoadingScreenDropped", ServerLoadingScreenDropped, ServerLoadingScreenDroppedOG);
    Runtime::Exec("/Script/Engine.PlayerController.ServerAcknowledgePossession", ServerAcknowledgePossession);
    Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerAttemptAircraftJump", ServerAttemptAircraftJump, ServerAttemptAircraftJumpOG);
    Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerExecuteInventoryItem", ServerExecuteInventoryItem);
    Runtime::Exec("/Script/FortniteGame.FortPlayerController.ServerUpdateGameplayOptions", ServerUpdateGameplayOptions);

    Runtime::Exec("/Game/Athena/Items/Consumables/Parents/GA_Athena_MedConsumable_Parent.GA_Athena_MedConsumable_Parent_C.Triggered_4C02BFB04B18D9E79F84848FFE6D2C32", Athena_MedConsumable_Triggered, Athena_MedConsumable_TriggeredOG);

    Runtime::Hook(Runtime::Offsets::ImageBase + 0x19A4780, GetPlayerViewPoint, (void**)GetPlayerViewPointOG);
    Runtime::Hook(Runtime::Offsets::ImageBase + 0x1c66a30, ReloadWeapon);
}
