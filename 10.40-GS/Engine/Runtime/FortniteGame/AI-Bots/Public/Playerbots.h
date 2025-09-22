#pragma once
#include "framework.h"

#include "FortInventory.h"
#include "BuildingContainer.h"

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
                    if (ItemName.contains("Shotgun") || ItemName.contains("SMG") || ItemName.contains("Assault")
                        || ItemName.contains("Sniper") || ItemName.contains("Rocket") || ItemName.contains("Pistol")) {
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
                        if (ItemName.contains("Shotgun") || ItemName.contains("SMG") || ItemName.contains("Assault")
                            || ItemName.contains("Sniper") || ItemName.contains("Rocket") || ItemName.contains("Pistol")) {
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
                        if (ItemName.contains("Wall")) {
                            //Log("Wall");
                            Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
                        }
                        break;
                    case EBotBuildingType::Floor:
                        if (ItemName.contains("Floor")) {
                            Log("Floor");
                            Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
                        }
                        break;
                    case EBotBuildingType::Stair:
                        if (ItemName.contains("Stair")) {
                            Log("Stair");
                            Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
                        }
                        break;
                    case EBotBuildingType::Cone:
                        if (ItemName.contains("Roof")) {
                            Log("Roof");
                            Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
                        }
                        break;
                    case EBotBuildingType::Edit:
                        if (ItemName.contains("EditTool")) {
                            Log("EditTool");
                            Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)Entry.ItemDefinition, Entry.ItemGuid);
                        }
                        break;
                    }
                }
            }
        }
	};

    class BotsBTService_InventoryManager {
    public:
        static void Update(AFortAthenaAIBotController* PC, FFortItemEntry* ItemEntry = nullptr)
        {
            PC->Inventory->HandleInventoryLocalUpdate();
            PC->Inventory->bRequiresLocalUpdate = true;
            PC->Inventory->ForceNetUpdate();
            if (ItemEntry == nullptr)
                PC->Inventory->Inventory.MarkArrayDirty();
            else
                PC->Inventory->Inventory.MarkItemDirty(*ItemEntry);
            
        }

        static UFortWorldItem* CreateNewItem(AFortAthenaAIBotController* PC, UFortItemDefinition* ItemDefinition, int Count)
        {
            UFortWorldItem* NewItem = (UFortWorldItem*)ItemDefinition->CreateTemporaryItemInstanceBP(Count, 1);
            return NewItem;
        }

        static UFortWorldItem* GiveItem(AFortAthenaAIBotController* PC, UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo = 0)
        {
            UFortWorldItem* NewInstance = CreateNewItem(PC, ItemDefinition, Count);
            NewInstance->ItemEntry.Count = Count;
            NewInstance->ItemEntry.LoadedAmmo = LoadedAmmo;
            PC->Inventory->Inventory.ReplicatedEntries.Add(NewInstance->ItemEntry);
            return NewInstance;
        }

        static UFortItemDefinition* FindItemDefinition(AFortAthenaAIBotController* PC, const FGuid& Guid)
        {
            if (!PC) return nullptr;

            for (int i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); i++)
            {
                FFortItemEntry* ItemEntries = &PC->Inventory->Inventory.ReplicatedEntries[i];
                if (!ItemEntries) continue;
                if (ItemEntries->ItemGuid == Guid) return ItemEntries->ItemDefinition;
            }

            return nullptr;
        }

        FFortItemEntry* FindItemEntry(AFortAthenaAIBotController* PC, UFortItemDefinition* ItemDefinition) {
            if (!PC || !ItemDefinition) return nullptr;

            for (int i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); i++)
            {
                FFortItemEntry* ItemEntry = &PC->Inventory->Inventory.ReplicatedEntries[i];
                if (!ItemEntry) continue;
                if (ItemEntry->ItemDefinition == ItemDefinition) return ItemEntry;
            }

            return nullptr;
        }

        void RemoveItem(AFortAthenaAIBotController* PC, UFortItemDefinition* ItemDefinition, int Count = 1)
        {
            if (!PC || !ItemDefinition) return;

            for (int i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); i++)
            {
                FFortItemEntry* ItemEntry = &PC->Inventory->Inventory.ReplicatedEntries[i];
                if (!ItemEntry) continue;

                if (ItemEntry->ItemDefinition == ItemDefinition)
                {
                    if (ItemEntry->Count > Count)
                    {
                        ItemEntry->Count -= Count;
                        Update(PC, ItemEntry);
                    }
                    else
                    {
                        PC->Inventory->Inventory.ReplicatedEntries.Remove(i);
                        Update(PC);
                    }

                    break;
                }
            }
        }

        bool IsPrimary(UFortItemDefinition* ItemDefinition)
        {
            if (!ItemDefinition) return false;
            if (!ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortEditToolItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortBuildingItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass())) return true;
            return false;
        }

        static EFortQuickBars GetQuickBars(UFortItemDefinition* ItemDefinition)
        {
            if (!ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) &&
                !ItemDefinition->IsA(UFortEditToolItemDefinition::StaticClass()) &&
                !ItemDefinition->IsA(UFortBuildingItemDefinition::StaticClass()) &&
                !ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) &&
                !ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) &&
                !ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
                return EFortQuickBars::Primary;

            return EFortQuickBars::Secondary;
        }

        bool IsFullInventory(AFortAthenaAIBotController* PC)
        {
            if (!PC || !PC->Inventory) return true;

            static constexpr int Max = 5;
            int Count = 0;

            for (int i = 0; i < PC->Inventory->Inventory.ReplicatedEntries.Num(); i++)
            {
                FFortItemEntry* Entries = &PC->Inventory->Inventory.ReplicatedEntries[i];
                if (!Entries) continue;
                if (!Entries->ItemDefinition) continue;

                if (GetQuickBars(Entries->ItemDefinition) == EFortQuickBars::Primary)
                {
                    ++Count;
                    //Log(std::format("BotInventory::IsFullInventory Count={}", Count).c_str());
                    if (Count >= Max)
                        return true;
                }
            }

            return false;
        }

        void DropItem(PlayerBot* bot, UFortItemDefinition* ItemDef) {
            FFortItemEntry* ItemToDrop = FindItemEntry(bot->PC, ItemDef);
            if (!ItemToDrop)
                return;

            FVector Drop = bot->Pawn->K2_GetActorLocation() + bot->Pawn->GetActorForwardVector() * 100.f;

            SpawnPickup(ItemDef, ItemToDrop->Count, ItemToDrop->LoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, bot->Pawn);

            RemoveItem(bot->PC, ItemDef, ItemToDrop->Count);
            Update(bot->PC);
        }

        bool IsCorrectItemType(EBotLootingFilter LootingFilter, UFortItemDefinition* ItemDef) {
            if (!ItemDef) {
                return false;
            }
            std::string ItemName = ItemDef->Name.ToString();

            switch (LootingFilter) {
            case EBotLootingFilter::Weapons:
                if (ItemDef->ItemType == EFortItemType::WeaponRanged) {
                    return true;
                    break;
                }
            case EBotLootingFilter::Shields:
                if (ItemName.contains("Shield") || ItemName.contains("ChugJug") || ItemName.contains("Slurp")) {
                    return true;
                }
            case EBotLootingFilter::BigShields:
                if (ItemName.contains("Shield") && !ItemName.contains("Small") && !ItemName.contains("Mini") || ItemName.contains("ChugJug") || ItemName.contains("Slurp")) {
                    return true;
                }
            case EBotLootingFilter::Meds:
                if (ItemName.contains("Medkit") || ItemName.contains("Bandage") || ItemName.contains("ChugJug") || ItemName.contains("Slurp")) {
                    return true;
                }
            case EBotLootingFilter::BigMeds:
                if (ItemName.contains("Medkit") || ItemName.contains("ChugJug") || ItemName.contains("Slurp")) {
                    return true;
                }
            case EBotLootingFilter::Ammo:
                if (ItemDef->ItemType == EFortItemType::Ammo) {
                    return true;
                }
            case EBotLootingFilter::MAX:
                return true;
            }
        }

        EBotSwapItemType GetSwapTypeFromDef(UFortItemDefinition* ItemDef) {
            std::string ItemDefName = ItemDef->Name.ToString();
            EBotSwapItemType ItemDefSwapType = EBotSwapItemType::MAX;

            if (ItemDefName.contains("Shotgun") || ItemDefName.contains("SMG") || ItemDefName.contains("Pistol")) {
                ItemDefSwapType = EBotSwapItemType::CloseRangeGun;
            }

            if (ItemDefName.contains("Assault") || ItemDefName.contains("Rocket")) {
                ItemDefSwapType = EBotSwapItemType::MediumRangeGun;
            }

            if (ItemDefName.contains("Sniper")) {
                ItemDefSwapType = EBotSwapItemType::LongRangeGun;
            }

            if (ItemDefName.contains("Medkit") || ItemDefName.contains("Bandage")) {
                ItemDefSwapType = EBotSwapItemType::HealthHeals;
            }

            if (ItemDefName.contains("Shield")) {
                ItemDefSwapType = EBotSwapItemType::ShieldHeals;
            }

            return ItemDefSwapType;
        }

        bool ShouldTakeItem(PlayerBot* bot, UFortItemDefinition* ItemDef) {
            if (!ItemDef)
                return false;

            if (ItemDef->ItemType == EFortItemType::Ammo || ItemDef->IsA(UFortResourceItemDefinition::StaticClass()))
                return true;

            std::string ItemDefName = ItemDef->Name.ToString();
            EBotSwapItemType ItemDefSwapType = GetSwapTypeFromDef(ItemDef);

            int HealthHealsCount = 0;
            int ShieldHealsCount = 0;
            int ShortRangeGunCount = 0;
            int MediumRangeGunCount = 0;
            int LongRangeGunCount = 0;

            for (FFortItemEntry ItemEntry : bot->PC->Inventory->Inventory.ReplicatedEntries) {
                UFortItemDefinition* InvDef = ItemEntry.ItemDefinition;
                if (!InvDef || InvDef->ItemType == EFortItemType::Ammo || ItemDef->IsA(UFortResourceItemDefinition::StaticClass()))
                    continue;

                EBotSwapItemType CurrentSwapItemType = GetSwapTypeFromDef(InvDef);
                if (CurrentSwapItemType == EBotSwapItemType::Other) {
                    DropItem(bot, InvDef);
                    continue;
                }

                if (CurrentSwapItemType == ItemDefSwapType && CurrentSwapItemType > EBotSwapItemType::ShieldHeals &&
                    ItemDefSwapType > EBotSwapItemType::ShieldHeals && ItemDefSwapType < EBotSwapItemType::Other) {
                    if (ItemDef->Rarity > InvDef->Rarity) {
                        return true;
                    }
                }

                switch (CurrentSwapItemType) {
                case EBotSwapItemType::HealthHeals:
                    HealthHealsCount++;
                    break;
                case EBotSwapItemType::ShieldHeals:
                    ShieldHealsCount++;
                    break;
                case EBotSwapItemType::CloseRangeGun:
                    ShortRangeGunCount++;
                    break;
                case EBotSwapItemType::MediumRangeGun:
                    MediumRangeGunCount++;
                    break;
                case EBotSwapItemType::LongRangeGun:
                    LongRangeGunCount++;
                    break;
                default:
                    break;
                }
            }

            switch (ItemDefSwapType) {
            case EBotSwapItemType::HealthHeals:
                if (HealthHealsCount == 0)
                    return true;
                break;
            case EBotSwapItemType::ShieldHeals:
                if (ShieldHealsCount == 0)
                    return true;
                break;
            case EBotSwapItemType::CloseRangeGun:
                if (ShortRangeGunCount == 0)
                    return true;
                break;
            case EBotSwapItemType::MediumRangeGun:
                if (MediumRangeGunCount == 0)
                    return true;
                break;
            case EBotSwapItemType::LongRangeGun:
                if (LongRangeGunCount == 0)
                    return true;
                break;
            default:
                break;
            }

            return false;
        }

        UFortItemDefinition* WantsToSwapItemFor(PlayerBot* bot, UFortItemDefinition* ItemDef) {
            if (!ItemDef)
                return nullptr;

            if (!IsFullInventory(bot->PC))
                return nullptr;
            std::string ItemDefName = ItemDef->Name.ToString();

            UFortItemDefinition* ItemDefTargetedSwap = nullptr;
            EBotSwapItemType ItemDefSwapType = GetSwapTypeFromDef(ItemDef);
            EBotSwapItemType SwapItemType = EBotSwapItemType::MAX;

            int HealthHealsCount = 0;
            int ShieldHealsCount = 0;
            int ShortRangeGunCount = 0;
            int MediumRangeGunCount = 0;
            int LongRangeGunCount = 0;

            for (FFortItemEntry ItemEntry : bot->PC->Inventory->Inventory.ReplicatedEntries) {
                UFortItemDefinition* InvDef = ItemEntry.ItemDefinition;
                if (!InvDef || InvDef->ItemType == EFortItemType::Ammo || InvDef->IsA(UFortResourceItemDefinition::StaticClass())) {
                    continue;
                }
                std::string InvDefName = InvDef->Name.ToString();
                EBotSwapItemType CurrentSwapItemType = GetSwapTypeFromDef(InvDef);

                if (CurrentSwapItemType == EBotSwapItemType::CloseRangeGun) {
                    ShortRangeGunCount++;
                    if (ItemDef->Rarity > InvDef->Rarity && ItemDefSwapType == CurrentSwapItemType) {
                        ItemDefTargetedSwap = InvDef;
                        SwapItemType = EBotSwapItemType::CloseRangeGun;
                    }
                    if (ShortRangeGunCount > 1) {
                        ItemDefTargetedSwap = InvDef;
                        SwapItemType = EBotSwapItemType::CloseRangeGun;
                    }
                    break;
                }

                if (CurrentSwapItemType == EBotSwapItemType::MediumRangeGun) {
                    MediumRangeGunCount++;
                    if (ItemDef->Rarity > InvDef->Rarity && ItemDefSwapType == CurrentSwapItemType) {
                        ItemDefTargetedSwap = InvDef;
                        SwapItemType = EBotSwapItemType::MediumRangeGun;
                    }
                    if (MediumRangeGunCount > 1) {
                        ItemDefTargetedSwap = InvDef;
                        SwapItemType = EBotSwapItemType::MediumRangeGun;
                    }
                    break;
                }

                if (CurrentSwapItemType == EBotSwapItemType::LongRangeGun) {
                    LongRangeGunCount++;
                    if (ItemDef->Rarity > InvDef->Rarity && ItemDefSwapType == CurrentSwapItemType) {
                        ItemDefTargetedSwap = InvDef;
                        SwapItemType = EBotSwapItemType::LongRangeGun;
                    }
                    if (LongRangeGunCount > 1) {
                        ItemDefTargetedSwap = InvDef;
                        SwapItemType = EBotSwapItemType::LongRangeGun;
                    }
                    break;
                }

                if (CurrentSwapItemType == EBotSwapItemType::HealthHeals) {
                    HealthHealsCount++;
                    if (InvDefName.contains("Bandage") && ItemDef->Name.ToString().contains("Medkit")) {
                        ItemDefTargetedSwap = InvDef;
                        SwapItemType = EBotSwapItemType::HealthHeals;
                    }
                    if (HealthHealsCount > 1) {
                        ItemDefTargetedSwap = InvDef;
                        SwapItemType = EBotSwapItemType::HealthHeals;
                    }
                    break;
                }

                if (CurrentSwapItemType == EBotSwapItemType::ShieldHeals) {
                    ShieldHealsCount++;
                    if ((InvDefName.contains("Small") || !InvDefName.contains("Mini")) &&
                        (ItemDefName.contains("Shield") && (!ItemDefName.contains("Small") || !ItemDefName.contains("Mini")))) {
                        ItemDefTargetedSwap = InvDef;
                        SwapItemType = EBotSwapItemType::ShieldHeals;
                    }
                    if (ShieldHealsCount > 1) {
                        ItemDefTargetedSwap = InvDef;
                        SwapItemType = EBotSwapItemType::ShieldHeals;
                    }
                    break;
                }

                if (InvDef->ItemType == EFortItemType::Weapon || InvDef->ItemType == EFortItemType::WeaponRanged || InvDef->ItemType == EFortItemType::Consumable) {
                    // Swap it cause we dont know what it is
                    ItemDefTargetedSwap = InvDef;
                    SwapItemType = EBotSwapItemType::Other;
                }
            }

            if (ItemDefTargetedSwap) {
                // We run the logic for determining swap in the part where we loop through the inventory so no point to this
                return ItemDefTargetedSwap;

                /*if (SwapItemType > EBotSwapItemType::ShieldHeals && SwapItemType != EBotSwapItemType::Other && SwapItemType != EBotSwapItemType::MAX) {
                    Log("ItemRarities: " + std::to_string((int)ItemDef->Rarity) + " SwapItemRarity: " + std::to_string((int)ItemDefTargetedSwap->Rarity));
                    if (ItemDef->Rarity > ItemDefTargetedSwap->Rarity && ItemDefSwapType == SwapItemType) {
                        Log("ItemToSwap: " + ItemDefTargetedSwap->Name.ToString());
                        return ItemDefTargetedSwap;
                    }
                }
                else if (SwapItemType == EBotSwapItemType::HealthHeals) {
                    if (ItemDefTargetedSwap->Name.ToString().contains("Bandage") && ItemDef->Name.ToString().contains("Medkit")) {
                        Log("ItemToSwap: " + ItemDefTargetedSwap->Name.ToString());
                        return ItemDefTargetedSwap;
                    }
                }
                else if (SwapItemType == EBotSwapItemType::ShieldHeals) {
                    std::string ItemDefTargetedSwapName = ItemDefTargetedSwap->Name.ToString();
                    if ((ItemDefTargetedSwapName.contains("Small") || !ItemDefTargetedSwapName.contains("Mini")) &&
                        (ItemDefName.contains("Shield") && (!ItemDefName.contains("Small") || !ItemDefName.contains("Mini")))) {
                        Log("ItemToSwap: " + ItemDefTargetedSwapName);
                        return ItemDefTargetedSwap;
                    }
                }
                else if (SwapItemType == EBotSwapItemType::Other) {
                    Log("ItemToSwap: " + ItemDefTargetedSwap->Name.ToString());
                    return ItemDefTargetedSwap;
                }*/
            }

            return nullptr;
        }

        AFortPickup* GetNearestFilteredPickup(PlayerBot* bot) {
            AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

            TArray<AActor*> PickupArray;
            UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), AFortPickup::StaticClass(), &PickupArray);

            AFortPickup* NearestPickup = nullptr;
            float NearestDistance = FLT_MAX;

            for (AActor* PickupActor : PickupArray) {
                AFortPickup* Pickup = (AFortPickup*)PickupActor;
                if (!Pickup || Pickup->bHidden) {
                    continue;
                }

                FFortItemEntry ItemEntry = Pickup->GetItemEntry(0);
                if (!ItemEntry.ItemDefinition) {
                    continue;
                }
                std::string ItemName = ItemEntry.ItemDefinition->Name.ToString();

                // We shouldnt do this anymore since we implemented the swapping
                bool bCorrectPickup = false;
                if (IsFullInventory(bot->PC)) {
                    if (WantsToSwapItemFor(bot, ItemEntry.ItemDefinition)) {
                        bCorrectPickup = true;
                    }
                }
                else {
                    bCorrectPickup = ShouldTakeItem(bot, ItemEntry.ItemDefinition);
                }
                if (bCorrectPickup) {
                    float DistanceTo = Pickup->GetDistanceTo(bot->Pawn);
                    if (DistanceTo < NearestDistance) {
                        NearestDistance = DistanceTo;
                        NearestPickup = Pickup;
                    }
                }
            }

            PickupArray.Free();
            return (AFortPickup*)NearestPickup;
        }

        void CompletePickupAnimation(PlayerBot* bot, AFortPickup* Pickup) {
            FFortPickupLocationData& PickupLocationData = Pickup->PickupLocationData;
            FFortItemEntry& PickupEntry = Pickup->PrimaryPickupItemEntry;

            UFortItemDefinition* PickupItemDefinition = PickupEntry.ItemDefinition;
            if (!PickupItemDefinition)
                return;
            FVector Drop = bot->Pawn->K2_GetActorLocation() + bot->Pawn->GetActorForwardVector() * 100.f;

            EBotSwapItemType PickupSwapItemType = GetSwapTypeFromDef(PickupItemDefinition);
            UFortItemDefinition* ItemToSwap = WantsToSwapItemFor(bot, PickupItemDefinition);
            if (ItemToSwap) {
                FFortItemEntry* SwapItemEntry = FindItemEntry(bot->PC, ItemToSwap);
                if (SwapItemEntry && SwapItemEntry->ItemDefinition) {
                    Log("ItemToSwap: " + SwapItemEntry->ItemDefinition->Name.ToString());
                    bot->Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)SwapItemEntry->ItemDefinition, SwapItemEntry->ItemGuid);
                }
            }
            else {
                if (!IsFullInventory(bot->PC)) {
                    if (!bot || !bot->PC || !bot->PC->Inventory) {
                        Log("Smth is null!");
                        SpawnPickup(PickupItemDefinition, PickupEntry.Count, PickupEntry.LoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, bot->Pawn);
                        return;
                    }
                    UFortItemDefinition* ItemToDrop = nullptr;

                    for (FFortItemEntry InvEntry : bot->PC->Inventory->Inventory.ReplicatedEntries) {
                        UFortItemDefinition* InvDef = InvEntry.ItemDefinition;
                        if (!InvDef || InvDef->ItemType == EFortItemType::Ammo || InvDef->IsA(UFortResourceItemDefinition::StaticClass()))
                            continue;

                        if (GetSwapTypeFromDef(InvDef) == PickupSwapItemType && PickupItemDefinition->Rarity > InvDef->Rarity) {
                            ItemToDrop = InvDef;
                            break;
                        }
                    }

                    if (ItemToDrop) {
                        DropItem(bot, ItemToDrop);
                    }
                }
            }

            int PickupCount = PickupEntry.Count;
            int PickupLoadedAmmo = PickupEntry.LoadedAmmo;
            int PickupMaxStackSize = PickupItemDefinition->MaxStackSize;
            FFortItemEntry* ItemEntry = FindItemEntry(bot->PC, PickupItemDefinition);

            if (!ItemEntry && PickupItemDefinition->IsStackable())
            {
                if (IsFullInventory(bot->PC))
                {
                    AFortWeapon* CurrentWeapon = bot->Pawn->CurrentWeapon;

                    if (CurrentWeapon)
                    {
                        UFortItemDefinition* OldWeaponDef = CurrentWeapon->WeaponData;

                        if (OldWeaponDef && OldWeaponDef->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
                        {
                            if (GetQuickBars(PickupItemDefinition) == EFortQuickBars::Secondary)
                            {
                                //Log("true");
                                GiveItem(bot->PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
                                Update(bot->PC);
                            }
                            else
                            {
                                //Log("False");
                                SpawnPickup(PickupItemDefinition, PickupCount, PickupLoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, bot->Pawn);
                            }
                        }
                        else
                        {
                            if (GetQuickBars(PickupItemDefinition) == EFortQuickBars::Primary)
                            {
                                FFortItemEntry* WeaponItemEntry = FindItemEntry(bot->PC, CurrentWeapon->WeaponData);
                                if (!WeaponItemEntry) return;

                                int OldWeaponCount = WeaponItemEntry->Count;
                                int OldWeaponAmmo = WeaponItemEntry->LoadedAmmo;

                                SpawnPickup(OldWeaponDef, OldWeaponCount, OldWeaponAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, bot->Pawn);

                                RemoveItem(bot->PC, OldWeaponDef, OldWeaponCount);
                                GiveItem(bot->PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
                                Update(bot->PC);
                            }
                            else if (GetQuickBars(PickupItemDefinition) == EFortQuickBars::Secondary)
                            {
                                GiveItem(bot->PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
                                Update(bot->PC);
                            }
                        }

                        Pickup->K2_DestroyActor();
                        return;
                    }
                    else
                    {
                        SpawnPickup(PickupItemDefinition, PickupCount, PickupLoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, bot->Pawn);

                        Pickup->K2_DestroyActor();
                        return;
                    }
                }
                else
                {
                    GiveItem(bot->PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
                    Update(bot->PC);

                    Pickup->K2_DestroyActor();
                    return;
                }
            }

            if (PickupItemDefinition->IsStackable() && ItemEntry && ItemEntry->Count < PickupMaxStackSize)
            {
                int Combined = ItemEntry->Count + PickupEntry.Count;
                if (Combined <= PickupMaxStackSize)
                {
                    ItemEntry->Count = Combined;
                    Update(bot->PC, ItemEntry);
                }
                else
                {
                    ItemEntry->Count = PickupMaxStackSize;
                    Update(bot->PC, ItemEntry);
                    int Overflow = Combined - PickupMaxStackSize;
                    GiveItem(bot->PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
                }

                Pickup->K2_DestroyActor();
                return;
            }

            if (IsFullInventory(bot->PC))
            {
                AFortWeapon* CurrentWeapon = bot->Pawn->CurrentWeapon;

                if (CurrentWeapon)
                {
                    UFortItemDefinition* OldWeaponDef = CurrentWeapon->WeaponData;

                    if (OldWeaponDef && OldWeaponDef->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
                    {
                        if (GetQuickBars(PickupItemDefinition) == EFortQuickBars::Secondary)
                        {
                            GiveItem(bot->PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
                            Update(bot->PC);
                        }
                        else
                        {
                            SpawnPickup(PickupItemDefinition, PickupCount, PickupLoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, bot->Pawn);
                        }
                    }
                    else
                    {
                        if (GetQuickBars(PickupItemDefinition) == EFortQuickBars::Primary)
                        {
                            FFortItemEntry* WeaponItemEntry = FindItemEntry(bot->PC, CurrentWeapon->WeaponData);
                            if (!WeaponItemEntry) return;

                            int OldWeaponCount = WeaponItemEntry->Count;
                            int OldWeaponAmmo = WeaponItemEntry->LoadedAmmo;

                            SpawnPickup(OldWeaponDef, OldWeaponCount, OldWeaponAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, bot->Pawn);

                            RemoveItem(bot->PC, OldWeaponDef, OldWeaponCount);
                            GiveItem(bot->PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
                            Update(bot->PC);
                        }
                        else if (FortInventory::GetQuickBars(PickupItemDefinition) == EFortQuickBars::Secondary)
                        {
                            GiveItem(bot->PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
                            Update(bot->PC);
                        }
                    }

                    Pickup->K2_DestroyActor();
                    return;
                }
                else
                {
                    SpawnPickup(PickupItemDefinition, PickupCount, PickupLoadedAmmo, Drop, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, bot->Pawn);

                    Pickup->K2_DestroyActor();
                    return;
                }
            }

            GiveItem(bot->PC, PickupItemDefinition, PickupCount, PickupLoadedAmmo);
            Update(bot->PC);

            Pickup->K2_DestroyActor();
            return;
        }
    };

    class BotsBTService_Healing {
    public:
        bool IsMaximumHealingReached(PlayerBot* bot, UFortItemDefinition* ItemDef) {
            if (!bot || !bot->Pawn || !ItemDef)
                return true; // can't heal if data is invalid

            std::string ItemName = ItemDef->Name.ToString();
            float Health = bot->Pawn->GetHealth();
            float Shield = bot->Pawn->GetShield();

            bool bMaximumHealingReached = false;

            if (ItemName.contains("Medkit") && Health >= 100.f)
                bMaximumHealingReached = true;

            if (ItemName.contains("Bandage") && Health >= 75.f)
                bMaximumHealingReached = true;

            if ((ItemName.contains("Shield") && !ItemName.contains("Small") && !ItemName.contains("Mini")) && Shield >= 100.f)
                bMaximumHealingReached = true;

            if ((ItemName.contains("Small") || ItemName.contains("Mini")) && Shield >= 50.f)
                bMaximumHealingReached = true;

            //Log("ItemDef: " + ItemName + " ReachedMaximum: " + std::to_string(bMaximumHealingReached) + " Health: " + std::to_string(Health) + " Shield: " + std::to_string(Shield));
            return bMaximumHealingReached;
        }

        FFortItemEntry GetCurrentHealingItemEntry(PlayerBot* bot) {
            float Health = bot->Pawn->GetHealth();
            float Shield = bot->Pawn->GetShield();

            for (FFortItemEntry ItemEntry : bot->PC->Inventory->Inventory.ReplicatedEntries) {
                if (!ItemEntry.ItemDefinition) {
                    continue;
                }
                std::string ItemName = ItemEntry.ItemDefinition->Name.ToString();
                bool bFound = false;

                if (bot->CurrentHealingType == EBotHealingType::Health || bot->CurrentHealingType == EBotHealingType::Both) {
                    if (Health >= 75.f) {
                        if (ItemName.contains("Medkit")) {
                            bFound = true;
                        }
                    }
                    else {
                        if (ItemName.contains("Medkit") || ItemName.contains("Bandage")) {
                            bFound = true;
                        }
                    }
                }

                if (bot->CurrentHealingType == EBotHealingType::Shield || bot->CurrentHealingType == EBotHealingType::Both) {
                    if (Shield >= 75) {
                        if (ItemName.contains("Shield") || !ItemName.contains("Small") || !ItemName.contains("Mini")) {
                            bFound = true;
                        }
                    }
                    else {
                        if (ItemName.contains("Shield")) {
                            bFound = true;
                        }
                    }
                }

                if (bFound) {
                    return ItemEntry;
                }
            }

            return FFortItemEntry{};
        }

        bool ShouldHeal(PlayerBot* bot) {
            float Health = bot->Pawn->GetHealth();
            float Shield = bot->Pawn->GetShield();

            if (Health != 100 && Shield != 100) {
                bot->CurrentHealingType = EBotHealingType::Both;
                return true;
            }
            else if (Health != 100) {
                bot->CurrentHealingType = EBotHealingType::Health;
                return true;
            }
            else if (Shield != 100) {
                bot->CurrentHealingType = EBotHealingType::Shield;
                return true;
            }
            else {
                // MAX meaning dont need to heal
                bot->CurrentHealingType = EBotHealingType::MAX;
                return false;
            }
        }

        bool CanHeal(PlayerBot* bot) {
            if (!ShouldHeal(bot)) {
                return false;
            }

            if (bot->bPotentiallyUnderAttack)
                return false;

            FFortItemEntry HealingItemEntry = GetCurrentHealingItemEntry(bot);
            if (!HealingItemEntry.ItemDefinition)
                return false;

            if (IsMaximumHealingReached(bot, HealingItemEntry.ItemDefinition)) {
                return false;
            }

            return true;
        }

        void StartHealing(PlayerBot* bot) {
            if (!CanHeal(bot)) {
                if (bot->bPotentiallyUnderAttack) {
                    if (bot->HasGun()) {
                        bot->SimpleSwitchToWeapon();
                        bot->BotState = EBotState::Combat;
                    }
                    else {
                        bot->EquipPickaxe();
                        bot->BotState = EBotState::Looting;
                    }
                }
                else {
                    bot->EquipPickaxe();
                    bot->BotState = EBotState::Looting;
                }
                return;
            }

            FFortItemEntry HealingItemEntry = GetCurrentHealingItemEntry(bot);
            if (!HealingItemEntry.ItemDefinition) {
                bot->EquipPickaxe();
                bot->BotState = EBotState::Looting;
                return;
            }

            //Log("SwitchToHealingItem: " + HealingItemEntry.ItemDefinition->GetFullName());
            bot->Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)HealingItemEntry.ItemDefinition, HealingItemEntry.ItemGuid);
            bot->Pawn->PawnStartFire(0);
        }
    };

    class BotsBTService_AIEvaluator {
    public:
        // When stressed the bot will handle combat situations with players or other bots differently
        bool IsStressed(AFortPlayerPawnAthena* Pawn) {
            // If the bots health is 75 or below then they are stressed
            if (Pawn->GetHealth() <= 75) {
                return true;
            }
            return false;
        }

        EBotWarmupChoice PickWarmupChoice() {
            //return EBotWarmupChoice::BuildStructures;
            return (EBotWarmupChoice)UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, (int)EBotWarmupChoice::MAX - 1);
		}

        EBotLootingFilter GetCurrentLootingChoice(PlayerBot* bot) {
            BotsBTService_Healing HealingManager;

            float Health = bot->Pawn->GetHealth();
            float Shield = bot->Pawn->GetShield();

            // We use this to check if the bot has any of the healing item it needs
            FFortItemEntry HealingItemEntry = HealingManager.GetCurrentHealingItemEntry(bot);

            if (bot->BotState == EBotState::Warmup) {
                if (!bot->HasGun()) {
                    return EBotLootingFilter::Weapons;
                }
                else {
                    return EBotLootingFilter::MAX;
                }
            }
            
            if (!bot->HasGun()) {
                return EBotLootingFilter::Weapons;
            }
            else if (Health != 100 && !HealingItemEntry.ItemDefinition) {
                if (Health >= 75) {
                    return EBotLootingFilter::BigMeds;
                }
                else {
                    return EBotLootingFilter::Meds;
                }
            }
            else if (Shield != 100 && !HealingItemEntry.ItemDefinition) {
                if (Shield >= 75) {
                    return EBotLootingFilter::BigShields;
                }
                else {
                    return EBotLootingFilter::Shields;
                }
            }

            return EBotLootingFilter::MAX;
        }

    public:
        void Tick(PlayerBot* bot) {
            BotsBTService_InventoryManager InventoryManager;
            BotsBTService_Healing HealingManager;

            if (bot->Pawn->IsDead()) {
                return;
            }

            bot->LastUpdatedBotLocation = bot->Pawn->K2_GetActorLocation();
            FVector Vel = bot->Pawn->GetVelocity();
            float Speed = 0.f;
            if (!Vel.IsZero()) {
                Speed = sqrtf(Vel.X * Vel.X + Vel.Y * Vel.Y);
            }

            if (bot->Pawn->bIsDBNO || !bot->NearestChest || !bot->NearestPickup || bot->NearestChest->bHidden || bot->NearestPickup->bHidden || bot->NearestChest->bAlreadySearched) {
                AActor* LastLootable = nullptr;
                if (bot->ClosestLootableType == ELootableType::Chest) {
                    LastLootable = bot->NearestChest;
                }
                else {
                    LastLootable = bot->NearestPickup;
                }
                bot->UpdateLootableReservation(LastLootable, true);
            }

            if (bot->tick_counter % 120 == 0) {
                bot->PC->K2_ClearFocus();
            }

            if (bot->tick_counter % 150 == 0) {
                bot->NearestPlayerPawn = bot->GetNearestPawn();

                if (!bot->Pawn->bIsDBNO) {
                    AActor* LastLootable = nullptr;
                    if (bot->ClosestLootableType == ELootableType::Chest) {
                        LastLootable = bot->NearestChest;
                    }
                    else {
                        LastLootable = bot->NearestPickup;
                    }
                    bot->UpdateLootableReservation(LastLootable, true);

                    bot->NearestChest = bot->GetNearestChest();
                    //bot->NearestPickup = bot->GetNearestPickup();
                    bot->NearestPickup = InventoryManager.GetNearestFilteredPickup(bot);
                    if (bot->NearestChest && bot->NearestPickup) {
                        if (bot->NearestChest->GetDistanceTo(bot->Pawn) > bot->NearestPickup->GetDistanceTo(bot->Pawn)) {
                            bot->ClosestLootableLoc = bot->NearestPickup->K2_GetActorLocation();
                            bot->ClosestLootableType = ELootableType::Pickup;
                            bot->UpdateLootableReservation(bot->NearestPickup, false);
                        }
                        else {
                            bot->ClosestLootableLoc = bot->NearestChest->K2_GetActorLocation();
                            bot->ClosestLootableType = ELootableType::Chest;
                            bot->UpdateLootableReservation(bot->NearestChest, false);
                        }
                    }
                    else if (bot->NearestChest) {
                        bot->ClosestLootableLoc = bot->NearestChest->K2_GetActorLocation();
                        bot->ClosestLootableType = ELootableType::Chest;
                        bot->UpdateLootableReservation(bot->NearestChest, false);
                    }
                    else if (bot->NearestPickup) {
                        bot->ClosestLootableLoc = bot->NearestPickup->K2_GetActorLocation();
                        bot->ClosestLootableType = ELootableType::Pickup;
                        bot->UpdateLootableReservation(bot->NearestPickup, false);
                    }
                }
            }

            if (bot->tick_counter % 30 == 0) {
                if (bot->BotState >= EBotState::Landed) {
                    if (HealingManager.CanHeal(bot)) {
                        //Log("CanHeal!");
                        bot->BotState = EBotState::Healing;
                    }
                }
            }

            if (bot->tick_counter % 2 == 0) {
                if (bot->bHasJumpedFromBus) {
                    bot->bPotentiallyUnderAttack = bot->PC->LineOfSightTo(bot->NearestPlayerPawn, bot->LastUpdatedBotLocation, true);
                    if (bot->bPotentiallyUnderAttack) {
                        if (bot->HasGun()) {
                            bot->BotState == EBotState::Combat;
                        }
                    }
                }
            }

            if (bot->tick_counter % 10 == 0) {
                bot->CurrentLootingFilter = GetCurrentLootingChoice(bot);

                if (bot->Pawn->bIsCrouched && UKismetMathLibrary::GetDefaultObj()->RandomBoolWithWeight(0.025f)) {
                    bot->Pawn->UnCrouch(false);
                }

                AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

                // Remove it every 10 ticks because the specific actions that require this should reset or clear after done
				bot->bHasCompletedCurrentAction = false;
                bot->bIsStressed = IsStressed(bot->Pawn);

                if (GameState->GamePhase >= EAthenaGamePhase::SafeZones) {
                    if (GameState->SafeZoneIndicator && !GameState->SafeZoneIndicator->NextCenter.IsZero()) {
                        float DistanceToZone = UKismetMathLibrary::GetDefaultObj()->Vector_Distance(bot->LastUpdatedBotLocation, GameState->SafeZoneIndicator->NextCenter);
                        bot->bIsInSafeZone = DistanceToZone < GameState->SafeZoneIndicator->NextRadius;
                        //Log("DistanceToZone: " + std::to_string(DistanceToZone) + " Radius: " + std::to_string(GameState->SafeZoneIndicator->NextRadius) + " bIsInSafeZone: " + std::to_string(bot->bIsInSafeZone));
                    }
                }
            }

            if ((bot->tick_counter % 90 == 0) && Speed >= 100 && bot->BotState >= EBotState::Landed) {
                bot->Run();
            }
        }
    };

    class BotsBTService_Warmup {
    public:
        void Tick(PlayerBot* bot) {
            if (bot->WarmupChoice == EBotWarmupChoice::MAX) {
                return;
			}

            if (bot->WarmupChoice == EBotWarmupChoice::Emote) {
                if (bot->tick_counter % 300 == 0) {
                    bot->Emote();
                }
            }
            else if (bot->WarmupChoice == EBotWarmupChoice::ApproachPlayersAndEmote) {
                if (bot->tick_counter % 200 == 0) {
					FVector BotPos = bot->Pawn->K2_GetActorLocation();
                    if (bot->NearestPlayerPawn) {
                        FVector Nearest = bot->NearestPlayerPawn->K2_GetActorLocation();
                        if (!Nearest.IsZero()) {
                            float Dist = UKismetMathLibrary::GetDefaultObj()->Vector_Distance(BotPos, Nearest);
                            if (Dist < 200.f + rand() % 300) {
                                bot->LookAt(bot->NearestPlayerPawn);
                                if (UKismetMathLibrary::GetDefaultObj()->RandomBool()) {
                                    bot->Emote();
                                }
                            }
                            else {
                                bot->LookAt(bot->NearestPlayerPawn);
                                bot->PC->MoveToActor(bot->NearestPlayerPawn, 100, true, false, true, nullptr, true);
                            }
                        }
                    }
                }
            }
            else if (bot->WarmupChoice == EBotWarmupChoice::LookAtRandomPlayers) {
                // 90 ticks = 3 seconds (GetMaxTickRate() == 30 == ticks 30 times per second * 3 = 90 == 3 seconds)
                if (bot->tick_counter % 90 == 0) {
					APawn* RandomPawn = bot->GetRandomPawn();
                    if (RandomPawn) {
						bot->LookAt(RandomPawn);
                    }
                }
            }
            else if (bot->WarmupChoice == EBotWarmupChoice::PickaxeOtherPlayers) {
                if (bot->tick_counter % 200 == 0) {
                    FVector BotPos = bot->Pawn->K2_GetActorLocation();
                    if (!bot->CurrentlyFocusedPawn) {
                        bot->CurrentlyFocusedPawn = bot->GetRandomPawn();
                    }
                    if (bot->CurrentlyFocusedPawn) {
                        FVector Nearest = bot->CurrentlyFocusedPawn->K2_GetActorLocation();
                        if (!Nearest.IsZero()) {
                            float Dist = UKismetMathLibrary::GetDefaultObj()->Vector_Distance(BotPos, Nearest);
                            if (Dist < 180.f) {
                                bot->LookAt(bot->CurrentlyFocusedPawn);
                                if (UKismetMathLibrary::GetDefaultObj()->RandomBool()) {
                                    bot->Pawn->PawnStartFire(0);
									//bot->bHasCompletedCurrentAction = true;
                                }
                            }
                            else {
                                bot->Pawn->PawnStopFire(0);
                                bot->LookAt(bot->CurrentlyFocusedPawn);
                                bot->PC->MoveToActor(bot->CurrentlyFocusedPawn, 100, true, false, true, nullptr, true);
                            }
                        }
                    }
                }
            }
            else if (bot->WarmupChoice == EBotWarmupChoice::FightOtherPlayers) {
                // Cant really test this until we have floorloot
                FVector BotPos = bot->Pawn->K2_GetActorLocation();
                if (!bot->HasGun()) {
                    bot->Pawn->PawnStopFire(0);
                    if (bot->NearestPickup) {
                        FVector Nearest = bot->NearestPickup->K2_GetActorLocation();
                        if (!Nearest.IsZero()) {
                            float Dist = UKismetMathLibrary::GetDefaultObj()->Vector_Distance(BotPos, Nearest);
                            if (Dist < 200.f) {
                                bot->LookAt(bot->CurrentlyFocusedPawn);
                                bot->Pickup(bot->NearestPickup);
                            }
                            else {
                                bot->LookAt(bot->CurrentlyFocusedPawn);
                                bot->PC->MoveToActor(bot->CurrentlyFocusedPawn, 100, true, false, true, nullptr, true);
                            }
                        }
                    }
                }
                else {
                    if (bot->IsPickaxeEquiped()) {
                        bot->SimpleSwitchToWeapon();
                    }
                    if (bot->NearestPlayerPawn) {
                        FVector Nearest = bot->NearestPlayerPawn->K2_GetActorLocation();
                        if (!((AFortPlayerStateAthena*)bot->NearestPlayerPawn->PlayerState)->bInAircraft || !Nearest.IsZero()) {
                            if (bot->PC->LineOfSightTo(bot->NearestPlayerPawn, BotPos, true)) {
                                bot->Pawn->PawnStartFire(0);
                                bot->LookAt(bot->NearestPlayerPawn);
                                bot->PC->MoveToActor(bot->NearestPlayerPawn, 300, true, false, true, nullptr, true);
                                bot->ForceStrafe(true);
                            }
                        }
                    }
                }
            }
            else if (bot->WarmupChoice == EBotWarmupChoice::BuildStructures) {
                bot->EquipBuildingItem(EBotBuildingType::Wall);
                bot->Pawn->BuildingState = EFortBuildingState::Placement;
                bot->Pawn->PawnStartFire(0);
                bot->Pawn->PawnStopFire(0);
            }
        }
    };

    class BotsBTService_AIDropZone {
    public:
        void ChooseDropZone(PlayerBot* bot) {
            if (BuildingFoundations.Num() == 0) return;

            AActor* DropZone = BuildingFoundations[UKismetMathLibrary::GetDefaultObj()->RandomIntegerInRange(0, BuildingFoundations.Num() - 1)];
            if (DropZone) {
                bot->TargetDropZone = DropZone->K2_GetActorLocation();
            }

            // makes it more realisetic cause they dont clutter together
            bot->TargetDropZone.X += UKismetMathLibrary::GetDefaultObj()->RandomFloatInRange(-800.f, 800.f);
            bot->TargetDropZone.Y += UKismetMathLibrary::GetDefaultObj()->RandomFloatInRange(-800.f, 800.f);
        }

    public:
        void Tick(PlayerBot* bot) {
            auto GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;
            auto Math = (UKismetMathLibrary*)UKismetMathLibrary::StaticClass()->DefaultObject;
            auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
            auto Statics = (UGameplayStatics*)UGameplayStatics::StaticClass()->DefaultObject;

            if (bot->TargetDropZone.IsZero()) {
                ChooseDropZone(bot);
                return;
            }

            if (bot->BotState == EBotState::PreBus) {
                if (!bot->bHasThankedBusDriver && GameState->GamePhase == EAthenaGamePhase::Aircraft && Math->RandomBoolWithWeight(0.0005f))
                {
                    bot->bHasThankedBusDriver = true;
                    bot->PlayerState->bThankedBusDriver = true;
                    bot->PlayerState->OnRep_ThankedBusDriver();
                    //bot->PC->ThankBusDriver();
                }
            }

            if (bot->BotState == EBotState::Bus) {
                bot->Pawn->SetShield(0);
                bot->CurrentlyFocusedPawn = nullptr;
                bot->bHasCompletedCurrentAction = false;
                bot->EquipPickaxe();
                if (bot->bHasJumpedFromBus) {
                    bot->BotState = EBotState::Skydiving;
                    return;
                }

                if (!bot->bHasThankedBusDriver && GameState->GamePhase == EAthenaGamePhase::Aircraft && Math->RandomBoolWithWeight(0.0005f))
                {
                    bot->bHasThankedBusDriver = true;
                    bot->PlayerState->bThankedBusDriver = true;
                    bot->PlayerState->OnRep_ThankedBusDriver();
                    //bot->PC->ThankBusDriver();
                }

                AActor* Bus = GameState->GetAircraft(0);
                if (!Bus) {
                    return;
                }

                FVector BusLocation = Bus->K2_GetActorLocation();
                FVector DropTarget = bot->TargetDropZone;
                DropTarget.Z = BusLocation.Z;

                if (GameState->GamePhase > EAthenaGamePhase::Aircraft) {
                    Log("Force Jump");
                    bot->Pawn->K2_TeleportTo(DropTarget, {});
                    bot->Pawn->BeginSkydiving(true);
                    bot->BotState = EBotState::Skydiving;

                    bot->bHasJumpedFromBus = true;

                    return;
                }

                float DistanceToDrop = Math->Vector_Distance(BusLocation, DropTarget);
                if (DistanceToDrop < bot->ClosestDistToDropZone) {
                    bot->ClosestDistToDropZone = DistanceToDrop;
                }
                else {
                    if (!bot->bHasThankedBusDriver && Math->RandomBoolWithWeight(0.5f)) {
                        bot->bHasThankedBusDriver = true;
                        bot->PlayerState->bThankedBusDriver = true;
                        bot->PlayerState->OnRep_ThankedBusDriver();
                        //bot->PC->ThankBusDriver();
                    }
                    if (Math->RandomBoolWithWeight(0.75)) {
                        bot->Pawn->K2_TeleportTo(GameState->GetAircraft(0)->K2_GetActorLocation(), {});
                        bot->Pawn->BeginSkydiving(true);
                        bot->BotState = EBotState::Skydiving;

                        bot->bHasJumpedFromBus = true;
                    }
                }

                return;
            }

            auto BotPos = bot->Pawn->K2_GetActorLocation();
            if (!bot->TargetDropZone.IsZero()) {
                bot->TargetDropZone.Z = BotPos.Z;
            }

            if (bot->BotState == EBotState::Skydiving) {
                if (!bot->Pawn->bIsSkydiving) {
                    bot->BotState = EBotState::Gliding;
                }

                if (!bot->TargetDropZone.IsZero()) {
                    // Dont know a better way to skydive
                    bot->Pawn->AddMovementInput(Math->NegateVector(bot->Pawn->GetActorUpVector()), 1, true);

                    float Dist = Math->Vector_Distance(BotPos, bot->TargetDropZone);
                    auto TestRot = Math->FindLookAtRotation(BotPos, bot->TargetDropZone);

                    bot->PC->SetControlRotation(TestRot);
                    bot->PC->K2_SetActorRotation(TestRot, true);

                    bot->PC->MoveToLocation(bot->TargetDropZone, 200.f, true, false, false, true, nullptr, true);
                }
            }
            else if (bot->BotState == EBotState::Gliding) {
                if (bot->Pawn->bIsSkydiving) {
                    bot->BotState = EBotState::Skydiving;
                }

                FVector Vel = bot->Pawn->GetVelocity();
                float Speed = Vel.Z;
                if (Speed == 0.f || bot->Pawn->bIsInWaterVolume) {
                    bot->PC->StopMovement();
                    bot->BotState = EBotState::Landed;
                }

                if (!bot->TargetDropZone.IsZero()) {
                    float Dist = Math->Vector_Distance(BotPos, bot->TargetDropZone);
                    if (Dist < 300.f) {
                        AActor* TargetLocationToTravel = nullptr;
                        if (bot->ClosestLootableType == ELootableType::Chest) {
                            TargetLocationToTravel = bot->NearestChest;
                        }
                        else {
                            TargetLocationToTravel = bot->NearestPickup;
                        }

                        if (TargetLocationToTravel) {
                            auto TestRot = Math->FindLookAtRotation(BotPos, TargetLocationToTravel->K2_GetActorLocation());

                            bot->PC->SetControlRotation(TestRot);
                            bot->PC->K2_SetActorRotation(TestRot, true);
                            bot->LookAt(TargetLocationToTravel);

                            bot->PC->MoveToLocation(TargetLocationToTravel->K2_GetActorLocation(), 200.f, true, false, false, true, nullptr, true);
                        }
                    }
                    else {
                        auto TestRot = Math->FindLookAtRotation(BotPos, bot->TargetDropZone);

                        bot->PC->SetControlRotation(TestRot);
                        bot->PC->K2_SetActorRotation(TestRot, true);

                        bot->PC->MoveToLocation(bot->TargetDropZone, 100.f, true, false, false, true, nullptr, true);
                    }
                }
            }
        }
    };

    class BotsBTService_Loot {
    public:
        void Tick(PlayerBot* bot, EBotLootingFilter LootingTarget = EBotLootingFilter::MAX) {
            BotsBTService_InventoryManager InventoryManager;

            auto Math = UKismetMathLibrary::GetDefaultObj();

            AActor* TargetLootable = bot->TargetLootable;

            if (TargetLootable) {
                FVector BotLoc = bot->Pawn->K2_GetActorLocation();
                if (!BotLoc.IsZero()) {
                    float Dist = Math->Vector_Distance(BotLoc, bot->ClosestLootableLoc);
                    auto TestRot = Math->FindLookAtRotation(BotLoc, bot->ClosestLootableLoc);

                    bot->PC->SetControlRotation(TestRot);
                    bot->PC->K2_SetActorRotation(TestRot, true);
                    bot->LookAt(TargetLootable);

                    float CurrentTime = UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld());

                    if (TargetLootable) {
                        if (bot->LootTargetStartTime == 0.f) {
                            bot->LootTargetStartTime = CurrentTime;
                            bot->LastLootTargetDistance = Dist;
                        }

                        float Elapsed = CurrentTime - bot->LootTargetStartTime;

                        // if the bot is not getting closer or stuck for more than 8 seconds then we should try go for another lootable
                        if ((Elapsed > 8.f && Dist > bot->LastLootTargetDistance - 100.f)) {
                            bot->Pawn->PawnStopFire(0);
                            if (!bot->Pawn->K2_TeleportTo(bot->ClosestLootableLoc, {})) {
                                FHitResult HitResult;
                                bot->Pawn->K2_SetActorLocation(bot->ClosestLootableLoc, false, &HitResult, true);
                            }
                            bot->LootTargetStartTime = 0.f;
                            return;
                        }

                        if (Dist < 300.f) {
                            bot->LootTargetStartTime = 0.f;
                        }

                        bot->LastLootTargetDistance = Dist;
                    }

                    if (Dist < 300.f) {
                        bot->PC->StopMovement();
                        bot->Pawn->PawnStopFire(0);
                        if (bot->ClosestLootableType == ELootableType::Pickup) {
                            //bot->PickupAllItemsInRange(400.f);
                            AFortPickup* Pickup = bot->NearestPickup;
                            if (Pickup)
                            {
                                bot->Pickup(Pickup);
                            }
                            bot->NearestPickup = nullptr;
                            bot->TimeToNextAction = 0;
                        }
                        else if (!bot->TimeToNextAction || !bot->Pawn->bStartedInteractSearch && bot->ClosestLootableType == ELootableType::Chest) {
                            bot->TimeToNextAction = UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld());
                            bot->Pawn->bStartedInteractSearch = true;
                            bot->Pawn->OnRep_StartedInteractSearch();
                        }
                        else if (UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld()) - bot->TimeToNextAction >= 1.5f && bot->ClosestLootableType == ELootableType::Chest) {
                            ABuildingContainer* Chest = (ABuildingContainer*)TargetLootable;
                            
                            BuildingContainer::SpawnLoot(Chest);
                            Chest->bAlreadySearched = true;
                            /*AFortPickup* Pickup = bot->GetNearestPickup();
                            if (Pickup)
                            {
                                bot->PickupAllItemsInRange();
                                bot->SimpleSwitchToWeapon();
                            }*/

                            bot->Pawn->bStartedInteractSearch = false;
                            bot->Pawn->OnRep_StartedInteractSearch();
                            bot->TimeToNextAction = 0;
                        } 
                        TargetLootable = nullptr;
                        return;
                    }
                    else if (Dist < 1000.f) {
                        bot->Pawn->PawnStartFire(0);
                        bot->PC->MoveToActor(TargetLootable, 50, true, false, true, nullptr, true);
                        //bot->Pawn->AddMovementInput(bot->Pawn->GetActorForwardVector(), 1.1f, true);
                    }
                    else {
                        bot->PC->MoveToActor(TargetLootable, 50, true, false, true, nullptr, true);
                        //bot->Pawn->AddMovementInput(bot->Pawn->GetActorForwardVector(), 1.1f, true);
                    }
                }
            }
        }
    };

    class BotsBTService_Combat {
    public:
        void Tick(PlayerBot* bot) {
            bot->PC->K2_ClearFocus();
            auto Math = UKismetMathLibrary::GetDefaultObj();

            if (!bot->HasGun()) {
                bot->BotState = EBotState::Looting;
            }
            if (bot->IsPickaxeEquiped()) {
                bot->SimpleSwitchToWeapon();
            }

            FVector BotLoc = bot->Pawn->K2_GetActorLocation();
            if (bot->NearestPlayerPawn) {
                FVector Nearest = bot->NearestPlayerPawn->K2_GetActorLocation();

                FRotator TestRot;
                FVector TargetPosmod = Nearest;

                if (!Nearest.IsZero()) {
                    float Dist = Math->Vector_Distance(BotLoc, Nearest);

                    if (bot->PC->LineOfSightTo(bot->NearestPlayerPawn, BotLoc, true)) {
                        if (true) {
                            float RandomXmod = Math->RandomFloatInRange(-150, 150);
                            float RandomYmod = Math->RandomFloatInRange(-150, 150);
                            float RandomZmod = Math->RandomFloatInRange(-150, 150);

                            FVector TargetPosMod{ Nearest.X + RandomXmod, Nearest.Y + RandomYmod, Nearest.Z + RandomZmod };

                            FRotator Rot = Math->FindLookAtRotation(BotLoc, TargetPosMod);

                            bot->PC->SetControlRotation(Rot);
                            bot->PC->K2_SetActorRotation(Rot, true);

                            //bot->PC->K2_SetFocalPoint(TargetPosMod);
                        }

                        if (UKismetMathLibrary::GetDefaultObj()->RandomBoolWithWeight(0.001)) {
                            TestRot = Math->FindLookAtRotation(BotLoc, Nearest);

                            bot->PC->SetControlRotation(TestRot);
                            bot->PC->K2_SetActorRotation(TestRot, true);
                        }

                        if (!bot->Pawn->bIsCrouched && Math->RandomBoolWithWeight(0.025f)) {
                            bot->Pawn->Crouch(false);
                        }

                        bot->ForceStrafe(true);

                        if (Dist < 1000) {
                            FVector BackVector = bot->Pawn->GetActorForwardVector() * -1.0f;
                            bot->Pawn->AddMovementInput(BackVector, 1.1f, true);
                        }

                        if (!bot->bIsStressed) {
                            bot->PC->MoveToActor(bot->NearestPlayerPawn, Math->RandomFloatInRange(400, 1500), true, false, true, nullptr, true);
                        }
                        else {
                            // Somewhat retreating
                            bot->Pawn->AddMovementInput((bot->Pawn->GetActorForwardVector() * -1.0f), 1.2f, true);
                        }

                        //bot->PC->StopMovement();
                        if (bot->PC->LineOfSightTo(bot->NearestPlayerPawn, BotLoc, true)) {
                            bot->Pawn->PawnStartFire(0);
                        }
                        else {
                            bot->Pawn->PawnStopFire(0);
                        }
                    }
                    else {
                        bot->BotState = EBotState::MovingToSafeZone;
                    }
                }
                else {}
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

			BotsBTService_AIEvaluator AIEvaluator;
			AIEvaluator.Tick(Bot);

            auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
            AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

            if (GameState->GamePhase == EAthenaGamePhase::Warmup) {
                Bot->BotState = EBotState::Warmup;
                if (Bot->WarmupChoice == EBotWarmupChoice::MAX) {
                    Bot->WarmupChoice = AIEvaluator.PickWarmupChoice();
                }
				BotsBTService_Warmup WarmupService;
                WarmupService.Tick(Bot);
				//Bot->Pawn->AddMovementInput(Bot->Pawn->GetActorForwardVector(), 1.0f, true);
            }
            else if (GameState->GamePhase == EAthenaGamePhase::Aircraft && !Bot->bHasJumpedFromBus) {
                if (GameState->GamePhaseStep == EAthenaGamePhaseStep::BusFlying) {
                    Bot->BotState = EBotState::Bus;
                }
                else {
                    Bot->BotState = EBotState::PreBus;
                }

                BotsBTService_AIDropZone DropZoneService;
                DropZoneService.Tick(Bot);
            }
            else if (Bot->BotState == EBotState::Skydiving || Bot->BotState == EBotState::Gliding) {
                BotsBTService_AIDropZone DropZoneService;
                DropZoneService.Tick(Bot);
            }
            else if (Bot->BotState == EBotState::Landed) {
                // Just do this for now i do more with this later
                Bot->BotState = EBotState::Looting;
            }
            else if (Bot->BotState == EBotState::Looting) {
                BotsBTService_Loot LootService;
                LootService.Tick(Bot, Bot->CurrentLootingFilter);
                if (Bot->bPotentiallyUnderAttack && Bot->HasGun()) {
                    Bot->BotState = EBotState::Combat;
                }
                else {
                    if (!Bot->bIsInSafeZone) {
                        Bot->BotState = EBotState::MovingToSafeZone;
                    }
                }
            }
            else if (Bot->BotState == EBotState::MovingToSafeZone) {
                Bot->Pawn->PawnStopFire(0);
                FVector BotLoc = Bot->Pawn->K2_GetActorLocation();
                if (Bot->NearestPlayerPawn) {
                    float Dist = UKismetMathLibrary::GetDefaultObj()->Vector_Distance(BotLoc, Bot->NearestPlayerPawn->K2_GetActorLocation());

                    if (Dist < 8000.f) {
                        if (Bot->PC->LineOfSightTo(Bot->NearestPlayerPawn, BotLoc, true) && Bot->HasGun()) {
                            Bot->BotState = EBotState::Combat;
                        }
                    }
                }

                if (Bot->bIsInSafeZone) {
                    Bot->BotState = EBotState::Looting;
                }

                if (GameState && GameState->SafeZoneIndicator)
                {
                    FVector TargetLoc = GameState->SafeZoneIndicator->NextCenter;
                    float TargetRadius = GameState->SafeZoneIndicator->NextRadius;

                    float CurrentTime = UGameplayStatics::GetDefaultObj()->GetTimeSeconds(UWorld::GetWorld());
                    float DistToZone = UKismetMathLibrary::GetDefaultObj()->Vector_Distance(Bot->LastUpdatedBotLocation, TargetLoc);

                    FRotator LookAtZone = UKismetMathLibrary::GetDefaultObj()->FindLookAtRotation(Bot->Pawn->K2_GetActorLocation(), TargetLoc);
                    Bot->PC->SetControlRotation(LookAtZone);
                    Bot->PC->K2_SetActorRotation(LookAtZone, true);

                    if (Bot->ZoneTargetStartTime == 0.f)
                    {
                        Bot->ZoneTargetStartTime = CurrentTime;
                        Bot->LastZoneTargetDistance = DistToZone;
                    }

                    float Elapsed = CurrentTime - Bot->ZoneTargetStartTime;

                    if (Elapsed > 10.f && DistToZone > Bot->LastZoneTargetDistance - 100.f)
                    {
                        Bot->Pawn->PawnStopFire(0);
                        Bot->LaunchIntoAir();

                        Bot->ZoneTargetStartTime = 0.f;
                        return;
                    }

                    if (DistToZone <= TargetRadius + 150.f) {
                        Bot->ZoneTargetStartTime = 0.f;
                    }

                    Bot->LastZoneTargetDistance = DistToZone;

                    Bot->PC->MoveToLocation(TargetLoc, TargetRadius, true, false, false, true, nullptr, true);
                }
            }
            else if (Bot->BotState == EBotState::Combat) {
                FVector BotLoc = Bot->Pawn->K2_GetActorLocation();
                if (Bot->NearestPlayerPawn) {
                    float Dist = UKismetMathLibrary::GetDefaultObj()->Vector_Distance(BotLoc, Bot->NearestPlayerPawn->K2_GetActorLocation());

                    if (Dist < 8000.f) {
                        if (!Bot->HasGun()) {
                            Bot->BotState = EBotState::Looting;
                        }
                        else {
                            if (Bot->PC->LineOfSightTo(Bot->NearestPlayerPawn, BotLoc, true)) {
                                Bot->BotState = EBotState::Combat;
                            }
                        }
                    }
                    else {
                        Bot->BotState = EBotState::MovingToSafeZone;
                    }
                }

                BotsBTService_Combat CombatService;
                CombatService.Tick(Bot);
            }
            else if (Bot->BotState == EBotState::Healing) {
                if (Bot->bPotentiallyUnderAttack) {
                    if (Bot->HasGun()) {
                        Bot->BotState = EBotState::Combat;
                    }
                    else {
                        Bot->BotState = EBotState::Looting;
                    }
                }

                BotsBTService_Healing HealingManager;

                HealingManager.StartHealing(Bot);
            }

			Bot->tick_counter++;
        }
    }
}