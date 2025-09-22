#include "../Public/FortPlayerControllerAthena.h"
#include "../../Inventory/Public/FortInventory.h"

void FortPlayerControllerAthena::InternalPickup(AFortPlayerControllerAthena* PlayerController, FFortItemEntry PickupEntry)
{
    auto MaxStack = PickupEntry.ItemDefinition->MaxStackSize;
    int ItemCount = 0;

    for (auto& Item : PlayerController->WorldInventory->Inventory.ReplicatedEntries)
    {
        if (FortInventory::GetQuickbar(Item.ItemDefinition) == EFortQuickBars::Primary) ItemCount += 1;
    }

    auto GiveOrSwap = [&]() {
        if (ItemCount == 5 && FortInventory::GetQuickbar(PickupEntry.ItemDefinition) == EFortQuickBars::Primary) {
            if (FortInventory::GetQuickbar(PlayerController->MyFortPawn->CurrentWeapon->WeaponData) == EFortQuickBars::Primary) {
                auto itemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([PlayerController](FFortItemEntry& entry)
                    { return entry.ItemGuid == PlayerController->MyFortPawn->CurrentWeapon->ItemEntryGuid; });
                FortInventory::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), *itemEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PlayerController->MyFortPawn);
                FortInventory::Remove(PlayerController, PlayerController->MyFortPawn->CurrentWeapon->ItemEntryGuid);
                FortInventory::GiveItem(PlayerController, PickupEntry.ItemDefinition, PickupEntry.LoadedAmmo, PickupEntry.Count, true);
            }
            else {
                FortInventory::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), (FFortItemEntry&)PickupEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PlayerController->MyFortPawn);
            }
        }
        else
            FortInventory::GiveItem(PlayerController, PickupEntry.ItemDefinition, PickupEntry.LoadedAmmo, PickupEntry.Count, true);
        };
    auto GiveOrSwapStack = [&](int32 OriginalCount) {
        if (PickupEntry.ItemDefinition->bAllowMultipleStacks && ItemCount < 5)
            FortInventory::GiveItem(PlayerController, PickupEntry.ItemDefinition, PickupEntry.LoadedAmmo, OriginalCount - MaxStack, true);
        else
            FortInventory::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), (FFortItemEntry&)PickupEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PlayerController->MyFortPawn, OriginalCount - MaxStack);
        };
    if (PickupEntry.ItemDefinition->IsStackable()) {
        auto itemEntry = PlayerController->WorldInventory->Inventory.ReplicatedEntries.Search([PickupEntry, MaxStack](FFortItemEntry& entry)
            { return entry.ItemDefinition == PickupEntry.ItemDefinition && entry.Count < MaxStack; });
        if (itemEntry) {
            auto State = itemEntry->StateValues.Search([](FFortItemEntryStateValue& Value)
                { return Value.StateType == EFortItemEntryState::ShouldShowItemToast; });
            if (!State) {
                FFortItemEntryStateValue Value{};
                Value.StateType = EFortItemEntryState::ShouldShowItemToast;
                Value.IntValue = true;
                itemEntry->StateValues.Add(Value);
            }
            else State->IntValue = true;

            if ((itemEntry->Count += PickupEntry.Count) > MaxStack) {
                auto OriginalCount = itemEntry->Count;
                itemEntry->Count = MaxStack;

                GiveOrSwapStack(OriginalCount);
            }
            FortInventory::ReplaceEntry(PlayerController, *itemEntry);
        }
        else {
            if (PickupEntry.Count > MaxStack) {
                auto OriginalCount = PickupEntry.Count;
                PickupEntry.Count = MaxStack;

                GiveOrSwapStack(OriginalCount);
            }
            GiveOrSwap();
        }
    }
    else {
        GiveOrSwap();
    }
}

void FortPlayerControllerAthena::ClientOnPawnDied(AFortPlayerControllerAthena* PlayerController, FFortPlayerDeathReport& DeathReport)
{
    if (!PlayerController) return ClientOnPawnDiedOG(PlayerController, DeathReport);
    auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
    auto GameState = (AFortGameStateAthena*)GameMode->GameState;
    auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;


    if (!GameState->IsRespawningAllowed(PlayerState) && PlayerController->WorldInventory && PlayerController->MyFortPawn)
    {
        bool bHasMats = false;
        for (auto& entry : PlayerController->WorldInventory->Inventory.ReplicatedEntries)
        {
            if (!entry.ItemDefinition->IsA<UFortWeaponMeleeItemDefinition>() && (entry.ItemDefinition->IsA<UFortResourceItemDefinition>() || entry.ItemDefinition->IsA<UFortWeaponRangedItemDefinition>() || entry.ItemDefinition->IsA<UFortConsumableItemDefinition>() || entry.ItemDefinition->IsA<UFortAmmoItemDefinition>()))
            {
                FortInventory::SpawnPickup(PlayerController->MyFortPawn->K2_GetActorLocation(), entry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination, PlayerController->MyFortPawn);
            }
        }

        AFortAthenaMutator_ItemDropOnDeath* Mutator = (AFortAthenaMutator_ItemDropOnDeath*)GameState->GetMutatorByClass(GameMode, AFortAthenaMutator_ItemDropOnDeath::StaticClass());

        if (Mutator)
        {
            for (FItemsToDropOnDeath& Items : Mutator->ItemsToDrop)
            {
                FortInventory::SpawnPickup(PlayerState->DeathInfo.DeathLocation, Items.ItemToDrop, (int)Runtime::EvaluateScalableFloat(Items.NumberToDrop), 0, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination, PlayerController->MyFortPawn);
            }
        }
    }

    auto KillerPlayerState = (AFortPlayerStateAthena*)DeathReport.KillerPlayerState;
    auto KillerPawn = (AFortPlayerPawnAthena*)DeathReport.KillerPawn;

    PlayerState->PawnDeathLocation = PlayerController->MyFortPawn ? PlayerController->MyFortPawn->K2_GetActorLocation() : FVector();
    PlayerState->DeathInfo.bDBNO = false;
    PlayerState->DeathInfo.DeathLocation = PlayerState->PawnDeathLocation;
    PlayerState->DeathInfo.DeathTags = PlayerController->MyFortPawn ? *(FGameplayTagContainer*)(__int64(PlayerController->MyFortPawn) + 0x1428) : DeathReport.Tags;
    PlayerState->DeathInfo.DeathCause = AFortPlayerStateAthena::ToDeathCause(PlayerState->DeathInfo.DeathTags, PlayerState->DeathInfo.bDBNO);
    if (PlayerState->DeathInfo.bDBNO) PlayerState->DeathInfo.Downer = KillerPlayerState;
    PlayerState->DeathInfo.FinisherOrDowner = KillerPlayerState;
    PlayerState->DeathInfo.Distance = PlayerController->MyFortPawn ? (PlayerState->DeathInfo.DeathCause != EDeathCause::FallDamage ? (KillerPawn ? KillerPawn->GetDistanceTo(PlayerController->MyFortPawn) : 0) : PlayerController->MyFortPawn->Cast<AFortPlayerPawnAthena>()->LastFallDistance) : 0;
    PlayerState->DeathInfo.bInitialized = true;
    PlayerState->OnRep_DeathInfo();

    int playerCount = GameMode->AlivePlayers.Num() - 1; 

    if (playerCount == 5 || playerCount == 10 || playerCount == 25)
    {
        int points = 10;
        if (playerCount == 10)
            points = 15;

        for (auto& Player : GameMode->AlivePlayers)
        {
            auto PlayerState = (AFortPlayerStateAthena*)Player->PlayerState;
            auto PlayerName = PlayerState->GetPlayerName().ToString();

            auto Controller = (AFortPlayerControllerAthena*)Player;
            Controller->ClientReportTournamentPlacementPointsScored(5, points);
        }

    }

    if (KillerPlayerState && KillerPawn && KillerPawn->Controller && KillerPawn->Controller->IsA<AFortPlayerControllerAthena>() && KillerPawn->Controller != PlayerController)
    {
        KillerPlayerState->KillScore++;
        KillerPlayerState->OnRep_Kills();
        KillerPlayerState->TeamKillScore++;
        KillerPlayerState->OnRep_TeamKillScore();

        KillerPlayerState->ClientReportKill(PlayerState);
        KillerPlayerState->ClientReportTeamKill(KillerPlayerState->TeamKillScore);
    }

    if (!GameState->IsRespawningAllowed(PlayerState) && (PlayerController->MyFortPawn ? !PlayerController->MyFortPawn->IsDBNO() : true))
    {
        PlayerState->Place = GameState->PlayersLeft;
        PlayerState->OnRep_Place();
    _Dead:
        FAthenaMatchStats& Stats = PlayerController->MatchReport->MatchStats;
        FAthenaMatchTeamStats& TeamStats = PlayerController->MatchReport->TeamStats;

        Stats.Stats[3] = PlayerState->KillScore;
        Stats.Stats[8] = PlayerState->SquadId;
        PlayerController->ClientSendMatchStatsForPlayer(Stats);

        TeamStats.Place = PlayerState->Place;
        TeamStats.TotalPlayers = GameState->TotalPlayers;
        PlayerController->ClientSendTeamStatsForPlayer(TeamStats);


        AFortWeapon* DamageCauser = nullptr;
        if (auto Projectile = DeathReport.DamageCauser ? DeathReport.DamageCauser->Cast<AFortProjectileBase>() : nullptr)
            DamageCauser = Projectile->GetOwner()->Cast<AFortWeapon>();
        else if (auto Weapon = DeathReport.DamageCauser ? DeathReport.DamageCauser->Cast<AFortWeapon>() : nullptr)
            DamageCauser = Weapon;

        ((void (*)(AFortGameModeAthena*, AFortPlayerController*, APlayerState*, AFortPawn*, UFortWeaponItemDefinition*, EDeathCause, char))(Runtime::Offsets::ImageBase + 0x11D95E0))(GameMode, PlayerController, KillerPlayerState == PlayerState ? nullptr : KillerPlayerState, KillerPawn, DamageCauser ? DamageCauser->WeaponData : nullptr, PlayerState->DeathInfo.DeathCause, 0);

        PlayerController->ClientSendEndBattleRoyaleMatchForPlayer(true, PlayerController->MatchReport->EndOfMatchResults);

        if (PlayerController->MyFortPawn && KillerPlayerState && KillerPawn && KillerPawn->Controller != PlayerController)
        {
            auto Handle = KillerPlayerState->AbilitySystemComponent->MakeEffectContext();
            FGameplayTag Tag;
            static auto Cue = FName(L"GameplayCue.Shield.PotionConsumed");
            Tag.TagName = Cue;
            KillerPlayerState->AbilitySystemComponent->NetMulticast_InvokeGameplayCueAdded(Tag, FPredictionKey(), Handle);
            KillerPlayerState->AbilitySystemComponent->NetMulticast_InvokeGameplayCueExecuted(Tag, FPredictionKey(), Handle);

            auto Health = KillerPawn->GetHealth();
            auto Shield = KillerPawn->GetShield();

            if (Health == 100)
            {
                Shield += Shield + 50;
            }
            else if (Health + 50 > 100)
            {
                Health = 100;
                Shield += (Health + 50) - 100;
            }
            else if (Health + 50 <= 100)
            {
                Health += 50;
            }

            KillerPawn->SetHealth(Health);
            KillerPawn->SetShield(Shield);
        }

        if (PlayerController->MyFortPawn && ((KillerPlayerState && KillerPlayerState->Place == 1) || PlayerState->Place == 1))
        {
            if (PlayerState->Place == 1)
            {
                KillerPlayerState = PlayerState;
                KillerPawn = (AFortPlayerPawnAthena*)PlayerController->MyFortPawn;
            }
            auto KillerPlayerController = (AFortPlayerControllerAthena*)KillerPlayerState->Owner;
            auto KillerWeapon = DamageCauser ? DamageCauser->WeaponData : nullptr;

            KillerPlayerController->PlayWinEffects(KillerPawn, KillerWeapon, PlayerState->DeathInfo.DeathCause, false);
            KillerPlayerController->ClientNotifyWon(KillerPawn, KillerWeapon, PlayerState->DeathInfo.DeathCause);
            KillerPlayerController->ClientNotifyTeamWon(KillerPawn, KillerWeapon, PlayerState->DeathInfo.DeathCause);

            if (KillerPlayerState != PlayerState)
            {
                KillerPlayerController->ClientSendEndBattleRoyaleMatchForPlayer(true, KillerPlayerController->MatchReport->EndOfMatchResults);

                FAthenaMatchStats& KillerStats = KillerPlayerController->MatchReport->MatchStats;
                FAthenaMatchTeamStats& KillerTeamStats = KillerPlayerController->MatchReport->TeamStats;


                KillerStats.Stats[3] = KillerPlayerState->KillScore;
                KillerStats.Stats[8] = KillerPlayerState->SquadId;
                KillerPlayerController->ClientSendMatchStatsForPlayer(KillerStats);

                KillerTeamStats.Place = KillerPlayerState->Place;
                KillerTeamStats.TotalPlayers = GameState->TotalPlayers;
                KillerPlayerController->ClientSendTeamStatsForPlayer(KillerTeamStats);
            }

            GameState->WinningTeam = KillerPlayerState->TeamIndex;
            GameState->OnRep_WinningTeam();
            GameState->WinningPlayerState = KillerPlayerState;
            GameState->OnRep_WinningPlayerState();
        }
    }

    return ClientOnPawnDiedOG(PlayerController, DeathReport);
}

void FortPlayerControllerAthena::ServerUpdateActorOptions(AFortPlayerControllerAthena* Controller, AActor* OptionsTarget, TArray<FString>& OptionsKeys, TArray<FString>& OptionsValues)
{
    if (!Controller) return;
}

void FortPlayerControllerAthena::ServerStartLoadingVolume(AFortPlayerControllerAthena* Controller, AFortVolume* VolumeToLoad)
{
    if (!Controller) return;
}

void FortPlayerControllerAthena::ServerSendClientProgressUpdate(AFortPlayerControllerAthena* Controller, int32 ClientProgressState, TArray<uint64>& ClientProgressUpdate)
{
    if (!Controller) return;
}

void FortPlayerControllerAthena::ServerEndMinigame(AFortPlayerControllerAthena* Controller, FFrame& Stack)
{
    Stack.IncrementCode();
    ServerEndMinigameOG(Controller, Stack);

    AFortMinigame* Minigame = Controller->GetMinigame();
    if (!Minigame) return;

    auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
    if (!GameMode) return;

    auto GameState = (AFortGameStateAthena*)GameMode->GameState;
    if (!GameState) return;

    auto GamePhase = GameState->GamePhase;

    Minigame->TotalRounds = 5;

    if (Minigame->TotalRounds >= 0) {
        Minigame->CurrentState = EFortMinigameState::PostRoundEnd;
        Minigame->CurrentRound++;

        Minigame->RoundWinnerDisplayTime = 5.f;
        Minigame->RoundScoreDisplayTime = 1.0f;
    }

    GamePhase = EAthenaGamePhase::EndGame;

    std::thread([Minigame]() {
        while (Minigame->CurrentState == EFortMinigameState::PostGameAbandon) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        Minigame->CurrentState = EFortMinigameState::PreGame;
        Minigame->OnRep_CurrentState();

        }).detach();
}

static bool bTextChanged = false;
void FortPlayerControllerAthena::ServerStartMinigame(AFortPlayerControllerAthena* Controller, FFrame& Stack)
{
    Stack.IncrementCode();

    ServerStartMinigameOG(Controller, Stack);

    AFortMinigame* Minigame = Controller->GetMinigame();
    if (!Minigame) return;

    struct FortMinigame_GetParticipatingPlayers final
    {
    public:
        TArray<class AFortPlayerState*> OutPlayers;
    } ret{};

    Minigame->GetParticipatingPlayers(&ret.OutPlayers);
    TArray<class AFortPlayerState*> Players = ret.OutPlayers;

    AFortPlayerControllerAthena* PlayerController = nullptr;

    for (int i = 0; i < Players.Num(); i++) {
        auto Player = (AFortPlayerStateAthena*)Players[i];
        PlayerController = (AFortPlayerControllerAthena*)Player->GetOwner();

        PlayerController->MyFortPawn->ForceKill(FGameplayTag(UKismetStringLibrary::Conv_StringToName(L"DeathCause.BanHammer")), PlayerController, nullptr);
    }

    auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
    if (!GameMode) return;

    auto GameState = (AFortGameStateAthena*)GameMode->GameState;
    if (!GameState) return;

    auto GamePhase = GameState->GamePhase;

    if (GamePhase == EAthenaGamePhase::Warmup) {
        GameMode->SafeZonePhase++;
        GamePhase = EAthenaGamePhase::SafeZones;
        Minigame->OnGamePhaseChanged(GamePhase);
    }

    std::thread([Minigame]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        Minigame->AdvanceState();
        Minigame->HandleMinigameStarted();
        Minigame->HandleVolumeEditModeChange(false);
        }).detach();
}

void FortPlayerControllerAthena::ServerGiveCreativeItem(AFortPlayerControllerAthena* Controller, const FFortItemEntry& CreativeItem)
{
    if (!Controller) return;
    auto WeaponStats = CreativeItem.ItemDefinition->IsA<UFortWeaponItemDefinition>() ? FortInventory::GetStats((UFortWeaponItemDefinition*)CreativeItem.ItemDefinition) : nullptr;
    FortInventory::GiveItem(Controller, CreativeItem.ItemDefinition, CreativeItem.Count, WeaponStats ? WeaponStats->ClipSize : 0, CreativeItem.Level);
}

void FortPlayerControllerAthena::ServerSaveIslandCheckpoint(AFortPlayerControllerAthena* Controller, AFortAthenaCreativePortal* Portal)
{
    if (!Controller) return;

    //FortVolume::SaveIsland(Controller, Controller->PlayerState->GetPlayerName().ToString() + " Island");
}

void FortPlayerControllerAthena::UpdateCreativePlotData(AFortPlayerControllerAthena* Controller, AFortVolume* VolumeToPublish, FCreativeIslandInfo& MyInfo)
{
    if (!Controller) return;

    FCreativeIslandData Data{};
    Data.IslandName = UKismetTextLibrary::Conv_StringToText(MyInfo.IslandTitle);
    Data.bIsDeleted = false;

    Controller->CreativeIslands.Add(Data);
    Controller->OnRep_CreativeIslands();

    //FortVolume::SaveIsland(Controller, Controller->PlayerState->GetPlayerName().ToString() + " Island");
}

void FortPlayerControllerAthena::ServerLoadPlotForPortal(AFortPlayerControllerAthena* Controller, AFortAthenaCreativePortal* Portal, FString& PlotItemId)
{
}

void FortPlayerControllerAthena::Patch()
{
    Runtime::Exec("/Script/FortniteGame.FortPlayerControllerAthena.ServerSaveIslandCheckpoint", ServerSaveIslandCheckpoint);
    Runtime::Exec("/Script/FortniteGame.FortPlayerControllerAthena.UpdateCreativePlotData", UpdateCreativePlotData);
    Runtime::Exec("/Script/FortniteGame.FortPlayerControllerAthena.ServerStartMinigame", ServerStartMinigame, ServerStartMinigameOG);
    Runtime::Exec("/Script/FortniteGame.FortPlayerControllerAthena.ServerEndMinigame", ServerEndMinigame, ServerEndMinigameOG);

    Runtime::_HookVT(AFortPlayerControllerAthena::GetDefaultObj()->VTable, uint32(0x440), ServerLoadPlotForPortal);
    Runtime::_HookVT(AFortPlayerControllerAthena::GetDefaultObj()->VTable, uint32(0x458), ServerGiveCreativeItem);

    Runtime::Hook(Runtime::Offsets::ImageBase + 0x1f34e50, ClientOnPawnDied, (void**)&ClientOnPawnDiedOG);
}
