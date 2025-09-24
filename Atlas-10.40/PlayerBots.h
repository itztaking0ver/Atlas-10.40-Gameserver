#pragma once
#include "framework.h"
#include "Engine/Runtime/FortniteGame/Inventory/Public/FortInventory.h"
#include "Engine/Runtime/FortniteGame/Building/Public/BuildingContainer.h"

namespace PlayerBots {
    enum class EBotBuildingType {
        Wall,
        Floor,
        Stair,
        Cone,
        Edit,
        Max
    };

    enum class EBotLootingFilter {
        Meds,
        BigMeds,
        Shields,
        BigShields,
        Ammo,
        Weapons,
        MAX
    };

    enum class EBotSwapItemType {
        HealthHeals,
        ShieldHeals,
        CloseRangeGun,
        MediumRangeGun,
        LongRangeGun,
        Other,
        MAX,
    };

    enum class EBotHealingType {
        Health,
        Shield,
        Both,
        MAX
    };

    enum class EBotStrafeType {
        StrafeLeft,
        StrafeRight
    };

    enum class EBotWarmupChoice {
        Emote, // Just stand still and emote
        ApproachPlayersAndEmote, // Approach random players and emote infront of them
        LookAtRandomPlayers, // Look at random players
        PickaxeOtherPlayers, // Pickaxe Random Players
        FightOtherPlayers, // Fight Random Players
        BuildStructures, // Build random structures (Not gonna implement this yet because it will be a pain)
        //AFK, // Would just stand still doing nothing but this is boring
        MAX // The default before bot picks choice
    };

    enum class EBotState {
        Warmup,
        PreBus,
        Bus,
        Skydiving,
        Gliding,
        Landed,
        Looting,
        MovingToSafeZone,
        Combat,
        Healing,
        MAX
    };

    enum class ELootableType {
        None = -1,
        Chest = 0,
        Pickup = 1
    };

    std::vector<class PlayerBot*> PlayerBotArray{};
    struct PlayerBot
    {
    public:
        // Choices that the bot makes
        EBotWarmupChoice WarmupChoice = EBotWarmupChoice::MAX;

        // The loot the bot wants, This isnt a strict filter only the targeted loot so if the bot finds something better that isnt in the filter-
        // Then the bot will still pick it up
        EBotLootingFilter CurrentLootingFilter = EBotLootingFilter::MAX;

        // What does the bot need to heal
        EBotHealingType CurrentHealingType = EBotHealingType::MAX;
    public:
        // Incase we need to pause ticking to run other logic
        bool bPauseTicking = false;

        // So we can track the current tick that the bot is doing (If we arent using the native behavior tree)
        uint64_t tick_counter = 0;

        // The Pawn of the bot
        AFortPlayerPawnAthena* Pawn = nullptr;

        // The playercontroller of the bot
        AFortAthenaAIBotController* PC = nullptr;

        // The playerstate of the bot
        AFortPlayerStateAthena* PlayerState = nullptr;

        EBotState BotState = EBotState::MAX;

        // So we dont have to keep calling k2_getactorloc all the time :sob:
        FVector LastUpdatedBotLocation = FVector();

        // Is the bot currently strafing?
        bool bIsCurrentlyStrafing = false;

        // The strafe type used by the bot, determines what direction
        EBotStrafeType StrafeType = EBotStrafeType::StrafeLeft;

        // When should the current strafe end?
        float StrafeEndTime = 0.0f;

        bool bPotentiallyUnderAttack = false;

        // The nearest player closest to the bot
        APawn* NearestPlayerPawn = nullptr;

        AFortPickup* NearestPickup = nullptr;

        ABuildingContainer* NearestChest = nullptr;

        // To Stop Crashes
        FVector ClosestLootableLoc = FVector();

        // Which ones closer?
        ELootableType ClosestLootableType = ELootableType::None;

        APawn* CurrentlyFocusedPawn = nullptr;

        // Has the bot completed the action that it was focused on?
        bool bHasCompletedCurrentAction = false;

        // The dropzone that the bot will attempt to land at
        FVector TargetDropZone = FVector();

        // The closest distance achieved to the targetdropzone, will be used mostly for determining bus drop
        float ClosestDistToDropZone = FLT_MAX;

        // Has the bot thanked the bus driver
        bool bHasThankedBusDriver = false;

        // Has the bot jumped from the bus, if not then set the botstate to bus.
        bool bHasJumpedFromBus = false;

        // General purpose timer
        float TimeToNextAction = 0.f;

        // The start time of this current lootable
        float LootTargetStartTime = 0.f;

        // Reservation of lootables, stops pileup and tracks current lootable
        AActor* TargetLootable = nullptr;

        // The distance between the bot and the lootable
        float LastLootTargetDistance = 0.f;

        float ZoneTargetStartTime = 0.f;

        float LastZoneTargetDistance = 0.f;

        // Is the bot stressed, will be determined every 10 ticks.
        bool bIsStressed = false;

        bool bIsInSafeZone = true;

    public:
        PlayerBot(AFortPlayerPawnAthena* Pawn, AFortAthenaAIBotController* PC, AFortPlayerStateAthena* PlayerState) {
            this->Pawn = Pawn;
            this->PC = PC;
            this->PlayerState = PlayerState;
            PlayerBotArray.push_back(this);
        }

        void GiveItem(UFortItemDefinition* Def, int Count = 1, int LoadedAmmo = 0)
        {
            if (!Def) {
                Log("ItemDef is nullptr!");
                return;
            }

            UFortWorldItem* Item = (UFortWorldItem*)Def->CreateTemporaryItemInstanceBP(Count, 0);
            Item->OwnerInventory = PC->Inventory;
            Item->ItemEntry.LoadedAmmo = LoadedAmmo;
            PC->Inventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);
            PC->Inventory->Inventory.ItemInstances.Add(Item);
            PC->Inventory->Inventory.MarkItemDirty(Item->ItemEntry);
            PC->Inventory->HandleInventoryLocalUpdate();
        }

        void Emote()
        {
            auto EmoteDef = Pawn->CosmeticLoadout.Dances[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, Pawn->CosmeticLoadout.Dances.Num() - 1)];
            if (!EmoteDef)
                return;

            static UClass* EmoteAbilityClass = StaticLoadObject<UClass>("/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C");

            FGameplayAbilitySpec Spec{};
            AbilitySpecConstructor(&Spec, reinterpret_cast<UGameplayAbility*>(EmoteAbilityClass->DefaultObject), 1, -1, EmoteDef);
            GiveAbilityAndActivateOnce(reinterpret_cast<AFortPlayerStateAthena*>(PC->PlayerState)->AbilitySystemComponent, &Spec.Handle, Spec);
        }

        void Run()
        {
            for (size_t i = 0; i < PlayerState->AbilitySystemComponent->ActivatableAbilities.Items.Num(); i++)
            {
                if (PlayerState->AbilitySystemComponent->ActivatableAbilities.Items[i].Ability->IsA(UFortGameplayAbility_Sprint::StaticClass()))
                {
                    if (PlayerState->AbilitySystemComponent->ActivatableAbilities.Items[i].ActivationInfo.PredictionKeyWhenActivated.bIsStale) {
                        continue;
                    }
                    if (PlayerState->AbilitySystemComponent->CanActivateAbilityWithMatchingTag(PlayerState->AbilitySystemComponent->ActivatableAbilities.Items[i].Ability->AbilityTags)) {
                        PlayerState->AbilitySystemComponent->ServerTryActivateAbility(PlayerState->AbilitySystemComponent->ActivatableAbilities.Items[i].Handle, PlayerState->AbilitySystemComponent->ActivatableAbilities.Items[i].InputPressed, PlayerState->AbilitySystemComponent->ActivatableAbilities.Items[i].ActivationInfo.PredictionKeyWhenActivated);
                    }
                    break;
                }
            }
        }

        void LaunchIntoAir() {
            FVector BotLoc = LastUpdatedBotLocation;
            BotLoc.Z = BotLoc.Z + 10000.f;

            FHitResult HitResult;
            Pawn->K2_SetActorLocation(BotLoc, false, &HitResult, true);
            Pawn->BeginSkydiving(false);
        }

        void Pickup(AFortPickup* Pickup) {
            if (!Pickup)
                return;

            // CompletePickupAnimation + InventoryManager Handles Now
            //this->GiveItem(Pickup->PrimaryPickupItemEntry.ItemDefinition, Pickup->PrimaryPickupItemEntry.Count, Pickup->PrimaryPickupItemEntry.LoadedAmmo);

            Pickup->PickupLocationData.bPlayPickupSound = true;
            Pickup->PickupLocationData.FlyTime = 0.3f;
            Pickup->PickupLocationData.ItemOwner = Pawn;
            Pickup->PickupLocationData.PickupGuid = Pickup->PrimaryPickupItemEntry.ItemGuid;
            Pickup->PickupLocationData.PickupTarget = Pawn;
            Pickup->OnRep_PickupLocationData();

            Pickup->bPickedUp = true;
            Pickup->OnRep_bPickedUp();
        }

        void PickupAllItemsInRange(float Range = 320.f) {
            if (!Pawn || !PC) {
                return;
            }

            TArray<AActor*> PickupArray;
            UGameplayStatics::GetDefaultObj()->GetAllActorsOfClass(UWorld::GetWorld(), AFortPickupAthena::StaticClass(), &PickupArray);

            for (size_t i = 0; i < PickupArray.Num(); i++)
            {
                if (!PickupArray[i] || PickupArray[i]->bHidden)
                    continue;

                if (PickupArray[i]->GetDistanceTo(Pawn) < Range)
                {
                    Pickup((AFortPickupAthena*)PickupArray[i]);
                }
            }

            PickupArray.Free();
        }

        FFortItemEntry* GetEntry(UFortItemDefinition* Def)
        {
            for (size_t i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); i++)
            {
                if (PC->Inventory->Inventory.ReplicatedEntries[i].ItemDefinition == Def)
                    return &PC->Inventory->Inventory.ReplicatedEntries[i];
            }

            return nullptr;
        }

        void EquipPickaxe()
        {
            if (!Pawn) {
                Log("No Pawn");
                return;
            }

            for (size_t i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); i++)
            {
                if (PC->Inventory->Inventory.ReplicatedEntries[i].ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
                {
                    Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)PC->Inventory->Inventory.ReplicatedEntries[i].ItemDefinition, PC->Inventory->Inventory.ReplicatedEntries[i].ItemGuid);
                    break;
                }
            }
        }

        ABuildingContainer* GetNearestChest()
        {
            static auto ChestClass = StaticLoadObject<UClass>("/Game/Building/ActorBlueprints/Containers/Tiered_Chest_Athena.Tiered_Chest_Athena_C");
            TArray<AActor*> ChestArray;

            UGameplayStatics::GetDefaultObj()->GetAllActorsOfClass(UWorld::GetWorld(), ChestClass, &ChestArray);

            ABuildingContainer* NearestChest = nullptr;
            float NearestDistance = FLT_MAX;

            for (size_t i = 0; i < ChestArray.Num(); i++)
            {
                ABuildingContainer* Chest = (ABuildingContainer*)ChestArray[i];
                if (Chest->bHidden || Chest->bAlreadySearched)
                    continue;

                if (!NearestChest || Chest->GetDistanceTo(Pawn) < NearestDistance)
                {
                    NearestDistance = Chest->GetDistanceTo(Pawn);
                    NearestChest = Chest;
                }
            }

            ChestArray.Free();
            return (ABuildingContainer*)NearestChest;
        }

        AFortPickup* GetNearestPickup() {
            TArray<AActor*> PickupArray;
            UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), AFortPickup::StaticClass(), &PickupArray);

            AActor* NearestPickup = nullptr;
            float NearestDistance = FLT_MAX;

            for (AActor* Pickup : PickupArray) {
                if (Pickup->GetDistanceTo(Pawn) < NearestDistance) {
                    NearestDistance = Pickup->GetDistanceTo(Pawn);
                    NearestPickup = Pickup;
                }
            }

            PickupArray.Free();
            return (AFortPickup*)NearestPickup;
        }

        APawn* GetNearestPawn() {
            auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
            AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

            APawn* NearestPlayer = nullptr;
            // I think using the float is faster than calling GetDistanceTo
            float NearestDistance = FLT_MAX;

            for (size_t i = 0; i < GameMode->AlivePlayers.Num(); i++)
            {
                if ((GameMode->AlivePlayers[i]->Pawn && GameMode->AlivePlayers[i]->Pawn->GetDistanceTo(Pawn) < NearestDistance))
                {
                    AFortPlayerStateAthena* PS = (AFortPlayerStateAthena*)GameMode->AlivePlayers[i]->PlayerState;
                    AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)GameMode->AlivePlayers[i];
                    AFortPlayerPawnAthena* Pawn = (AFortPlayerPawnAthena*)PC->Pawn;
                    if (!PS || !PC || !Pawn)
                        continue;
                    if (PS->TeamIndex != PlayerState->TeamIndex) {
                        if (PS->bInAircraft || Pawn->IsDead()) {
                            continue;
                        }
                        NearestDistance = GameMode->AlivePlayers[i]->Pawn->GetDistanceTo(Pawn);
                        NearestPlayer = GameMode->AlivePlayers[i]->Pawn;
                    }
                }
            }

            for (size_t i = 0; i < GameMode->AliveBots.Num(); i++)
            {
                if (GameMode->AliveBots[i]->Pawn != Pawn)
                {
                    if ((GameMode->AliveBots[i]->Pawn && GameMode->AliveBots[i]->Pawn->GetDistanceTo(Pawn) < NearestDistance))
                    {
                        if (GameMode->AliveBots[i]->Pawn == Pawn) {
                            continue;
                        }
                        AFortPlayerStateAthena* PS = (AFortPlayerStateAthena*)GameMode->AliveBots[i]->PlayerState;
                        if (PS->TeamIndex != PlayerState->TeamIndex) {
                            NearestDistance = GameMode->AliveBots[i]->Pawn->GetDistanceTo(Pawn);
                            NearestPlayer = GameMode->AliveBots[i]->Pawn;
                        }
                    }
                }
            }

            return NearestPlayer;
        }

        void UpdateLootableReservation(AActor* Lootable, bool RemoveReservation) {
            if (RemoveReservation && !TargetLootable) {
                return;
            }

            if (!RemoveReservation) {
                if (!Lootable) {
                    return;
                }
                TargetLootable = Lootable;
                Lootable->bHidden = true;
            }
            else {
                TargetLootable->bHidden = false;
                TargetLootable = nullptr;
            }
        }

        APawn* GetRandomPawn() {
            auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;

            APawn* RandomPlayer = GameMode->AlivePlayers[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, GameMode->AlivePlayers.Num() - 1)]->Pawn;
            APawn* RandomBot = GameMode->AliveBots[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, GameMode->AliveBots.Num() - 1)]->Pawn;
            if (UKismetMathLibrary::GetDefaultObj()->RandomBool()) {
                return RandomPlayer;
            }
            else {
                if (RandomBot == Pawn) {
                    return nullptr;
                }
                return RandomBot;
            }   
        }

        void ForceStrafe(bool override) {
            if (!bIsCurrentlyStrafing && override)
            {
                bIsCurrentlyStrafing = true;
                if (UKismetMathLibrary::RandomBool()) {
                    StrafeType = EBotStrafeType::StrafeLeft;
                }
                else {
                    StrafeType = EBotStrafeType::StrafeRight;
                }
                StrafeEndTime = UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld()) + UKismetMathLibrary::GetDefaultObj()->RandomFloatInRange(2.0f, 5.0f);
            }
            else
            {
                if (UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld()) < StrafeEndTime)
                {
                    if (StrafeType == EBotStrafeType::StrafeLeft) {
                        Pawn->AddMovementInput((Pawn->GetActorRightVector() * -1.0f), 1.5f, true);
                    }
                    else {
                        Pawn->AddMovementInput(Pawn->GetActorRightVector(), 1.5f, true);
                    }
                }
                else
                {
                    bIsCurrentlyStrafing = false;
                }
            }
        }

        void LookAt(AActor* Actor)
        {
            if (!Pawn || PC->GetFocusActor() == Actor)
                return;

            if (!Actor)
            {
                PC->K2_ClearFocus();
                return;
            }

            PC->K2_SetFocus(Actor);
        }

        bool IsPickaxeEquiped() {
            if (!Pawn || !Pawn->CurrentWeapon)
                return false;

            if (Pawn->CurrentWeapon->WeaponData->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
            {
                return true;
            }
            return false;
        }

        bool HasGun()
        {
            for (size_t i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); i++)
            {
                auto& Entry = PC->Inventory->Inventory.ReplicatedEntries[i];
                if (Entry.ItemDefinition) {
                    std::string ItemName = Entry.ItemDefinition->Name.ToString();
                    if (ItemName.find("Shotgun") != std::string::npos || ItemName.find("SMG") != std::string::npos || ItemName.find("Assault") != std::string::npos
                        || ItemName.find("Sniper") != std::string::npos || ItemName.find("Rocket") != std::string::npos || ItemName.find("Pistol") != std::string::npos) {
                        return true;
                        break;
                    }
                }
            }
            return false;
        }

        // Doesent work
        void SimpleSwitchToWeapon() {
            if (!Pawn || !Pawn->CurrentWeapon || !Pawn->CurrentWeapon->WeaponData || !PC || !PC->Inventory)
                return;

            if (Pawn->CurrentWeapon->WeaponData->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
            {
                for (size_t i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); i++)
                {
                    auto& Entry = PC->Inventory->Inventory.ReplicatedEntries[i];
                    if (Entry.ItemDefinition) {
                        std::string ItemName = Entry.ItemDefinition->Name.ToString();
                        if (ItemName.find("Shotgun") != std::string::npos || ItemName.find("SMG") != std::string::npos || ItemName.find("Assault") != std::string::npos
                            || ItemName.find("Sniper") != std::string::npos || ItemName.find("Rocket") != std::string::npos || ItemName.find("Pistol") != std::string::npos) {
                            //Log("SimpleSwitchToWeapon: " + ItemName);
                            Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
                            break;
                        }
                    }
                }
            }
        }

        void EquipBuildingItem(EBotBuildingType BuildingType) {
            if (!Pawn || !Pawn->CurrentWeapon || !PC || !PC->Inventory)
                return;

            for (size_t i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); i++)
            {
                auto& Entry = PC->Inventory->Inventory.ReplicatedEntries[i];
                if (!Entry.ItemDefinition->IsA(UFortBuildingItemDefinition::StaticClass())) {
                    continue;
                }
                if (Entry.ItemDefinition) {
                    std::string ItemName = Entry.ItemDefinition->Name.ToString();
                    switch (BuildingType) {
                    case EBotBuildingType::Wall:
                        if (ItemName.find("Wall") != std::string::npos) {
                            //Log("Wall");
                            Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
                        }
                        break;
                    case EBotBuildingType::Floor:
                        if (ItemName.find("Floor") != std::string::npos) {
                            Log("Floor");
                            Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
                        }
                        break;
                    case EBotBuildingType::Stair:
                        if (ItemName.find("Stair") != std::string::npos) {
                            Log("Stair");
                            Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
                        }
                        break;
                    case EBotBuildingType::Cone:
                        if (ItemName.find("Roof") != std::string::npos) {
                            Log("Roof");
                            Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
                        }
                        break;
                    case EBotBuildingType::Edit:
                        if (ItemName.find("EditTool") != std::string::npos) {
                            Log("EditTool");
                            Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
                        }
                        break;
                    }
                }
            }
        }
    };

    void TickBots() {
        auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
        auto GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

        for (PlayerBot* Bot : PlayerBotArray) {
            if (!Bot || !Bot->PC || !Bot->Pawn || !Bot->PlayerState) {
                continue;
            }

            if (Bot->Pawn->IsDead()) {
                for (int i = 0; i < PlayerBotArray.size(); i++) {
                    if (PlayerBotArray[i]->Pawn == Bot->Pawn) {
                        delete PlayerBotArray[i];
                        PlayerBotArray.erase(PlayerBotArray.begin() + i);
                        Log("Freed a dead bot from the array!");
                    }
                }
                continue;
            }

            if (GameState->GamePhase <= EAthenaGamePhase::Warmup) {
                if (Bot->tick_counter <= 150) {
                    Bot->tick_counter++;
                    continue;
                }
            }

            if (Bot->bPauseTicking) {
                continue;
            }

            // Basic bot AI logic
            Bot->LastUpdatedBotLocation = Bot->Pawn->K2_GetActorLocation();
            
            // Update bot state based on game phase
            if (GameState->GamePhase == EAthenaGamePhase::Warmup) {
                Bot->BotState = EBotState::Warmup;
            }
            else if (GameState->GamePhase == EAthenaGamePhase::Aircraft && !Bot->bHasJumpedFromBus) {
                Bot->BotState = EBotState::Bus;
            }
            else if (Bot->BotState == EBotState::Skydiving || Bot->BotState == EBotState::Gliding) {
                // Handle skydiving/gliding logic
            }
            else if (Bot->BotState == EBotState::Landed) {
                Bot->BotState = EBotState::Looting;
            }
            else if (Bot->BotState == EBotState::Looting) {
                // Handle looting logic
                if (Bot->bPotentiallyUnderAttack && Bot->HasGun()) {
                    Bot->BotState = EBotState::Combat;
                }
            }
            else if (Bot->BotState == EBotState::Combat) {
                // Handle combat logic
                if (!Bot->HasGun()) {
                    Bot->BotState = EBotState::Looting;
                }
            }

            Bot->tick_counter++;
        }
    }
}
