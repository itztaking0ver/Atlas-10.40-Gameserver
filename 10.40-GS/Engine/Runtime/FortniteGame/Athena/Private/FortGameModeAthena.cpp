#include "../Public/FortGameModeAthena.h"
#include "../../../../Plugins/Crystal/Public/Crystal.h"
#include "../Public/FortPlaylistAthena.h"
#include "../../Inventory/Public/FortInventory.h"
#include "../../Creative/Public/FortPlaysetItemDefinition.h"
#include "../Public/FortAthenaVehicle.h"
#include "../../Building/Public/BuildingContainer.h"

void ShowFoundation(ABuildingFoundation* Foundation)
{
	if (!Foundation) return;

	Foundation->StreamingData.BoundingBox = Foundation->StreamingBoundingBox;
	Foundation->StreamingData.FoundationLocation = Foundation->GetTransform().Translation;
	Foundation->SetDynamicFoundationEnabled(true);
}

bool FortGameModeAthena::ReadyToStartMatch(UObject* Context, FFrame& Stack, bool* Result)
{
	Stack.IncrementCode();
	AFortGameModeAthena* GameMode = (AFortGameModeAthena*)Context;

	if (!GameMode) return *Result = callOGWithRet(GameMode, "/Script/Engine.GameMode", ReadyToStartMatch);
	if (!GameMode->bWorldIsReady) GameMode->bWorldIsReady = true;

	auto GameState = (AFortGameStateAthena*)GameMode->GameState;

	if (!UCrystal->bCreative) {
		if (!GameState->MapInfo) return false;
	}

	if (GameMode->CurrentPlaylistId == -1)
	{
		FortPlaylistAthena::AssignPlaylist(Runtime::StaticFindObject<UFortPlaylistAthena>(UCrystal->PlaylistID), GameMode);

		ShowFoundation(Runtime::StaticFindObject<ABuildingFoundation>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_StreamingTest16"));
		ShowFoundation(Runtime::StaticFindObject<ABuildingFoundation>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.SLAB_1"));

		if (!UCrystal->bCreative)
		{
			FortAthenaVehicle::SpawnVehicles();
			xmap<FName, FFortLootTierData*> LootTierDataTempArr;
			auto LootTierData = Runtime::StaticFindObject<UFortPlaylistAthena>(UCrystal->PlaylistID)->LootTierData.Get();
			if (!LootTierData)
				LootTierData = Runtime::StaticFindObject<UDataTable>("/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client");
			for (auto& [Key, Val] : (TMap<FName, FFortLootTierData*>) LootTierData->RowMap) {
				BuildingContainer::TierDataAllGroups.push_back(Val);
			}

			xmap<FName, FFortLootPackageData*> LootPackageTempArr;
			auto LootPackages = Runtime::StaticFindObject<UFortPlaylistAthena>(UCrystal->PlaylistID)->LootPackages.Get();
			if (!LootPackages) LootPackages = Runtime::StaticFindObject<UDataTable>("/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client");
			for (auto& [Key, Val] : (TMap<FName, FFortLootPackageData*>) LootPackages->RowMap) {
				BuildingContainer::LPGroupsAll.push_back(Val);
			}

			BuildingContainer::SpawnFloorLootForContainer(Runtime::StaticFindObject<UBlueprintGeneratedClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C"));
			BuildingContainer::SpawnFloorLootForContainer(Runtime::StaticFindObject<UBlueprintGeneratedClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C"));
		}

		GameMode->WarmupRequiredPlayerCount = 1;
		return *Result = false;
	}

	if (!UWorld::GetWorld()->NetDriver)
	{
		auto Starts = UCrystal->bCreative ? (TArray<AActor*>) Runtime::GetAll<AFortPlayerStartCreative>() : (TArray<AActor*>) Runtime::GetAll<AFortPlayerStartWarmup>();
		auto StartsNum = Starts.Num();

		Starts.Free();
		if (StartsNum == 0) {
			return false;
		}

		using CreateNetDriverType = UNetDriver * (*)(UEngine*, UWorld*, FName);
		using InitListenType = bool (*)(UNetDriver*, UWorld*, FURL&, bool, FString&);
		using SetWorldType = void (*)(UNetDriver*, UWorld*);

		static auto GND = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");
		auto NetDriver = ((CreateNetDriverType)Runtime::Offsets::CreateNetDriver)(UEngine::GetEngine(), UWorld::GetWorld(), GND);
		NetDriver->World = UWorld::GetWorld();
		NetDriver->NetDriverName = GND;

		UWorld::GetWorld()->NetDriver = NetDriver;
		for (auto& Collection : UWorld::GetWorld()->LevelCollections) Collection.NetDriver = NetDriver;

		FString Err;
		FURL URL{};
		URL.Port = 7777;

		((InitListenType)Runtime::Offsets::InitListen)(NetDriver, UWorld::GetWorld(), URL, false, Err);
		((SetWorldType)Runtime::Offsets::SetWorld)(NetDriver, UWorld::GetWorld());
		UCrystal->SetState("Listening");
	}

	return *Result = callOGWithRet(GameMode, "/Script/Engine.GameMode", ReadyToStartMatch);
}

APawn* FortGameModeAthena::SpawnDefaultPawnFor(UObject* Context, FFrame& Stack, APawn** Ret)
{
	AController* NewPlayer;
	AActor* StartSpot;
	Stack.StepCompiledIn(&NewPlayer);
	Stack.StepCompiledIn(&StartSpot);
	Stack.IncrementCode();

	auto GameMode = (AFortGameModeAthena*)Context;
	auto Transform = StartSpot->GetTransform();
	Transform.Translation.Z += 150.f;
	APawn* Pawn = GameMode->SpawnDefaultPawnAtTransform(NewPlayer, Transform);

	auto PlayerController = NewPlayer->Cast<AFortPlayerControllerAthena>();

	for (auto& StartingItem : ((AFortGameModeAthena*)GameMode)->StartingItems) {
		if (StartingItem.Count) {
			FortInventory::GiveItem(PlayerController, StartingItem.Item, StartingItem.Count);
		}
	}

	FortInventory::GiveItem(PlayerController, PlayerController->CosmeticLoadoutPC.Pickaxe->WeaponDefinition);

	return *Ret = Pawn;
}

EFortTeam FortGameModeAthena::PickTeam(AFortGameModeAthena* GameMode, uint8_t PreferredTeam, AFortPlayerControllerAthena* Controller)
{
	uint8_t ret = CurrentTeam;

	if (++PlayersOnCurTeam >= ((AFortGameStateAthena*)GameMode->GameState)->CurrentPlaylistInfo.BasePlaylist->MaxSquadSize) {
		CurrentTeam++;
		PlayersOnCurTeam = 0;
	}
	else
	{
		ret = (uint8_t)PickTeamOG(GameMode, PreferredTeam, Controller);
	}

	return EFortTeam(ret);
}

void FortGameModeAthena::OnAircraftExitedDropZone(UObject* Context, FFrame& Stack, void*)
{
	AFortAthenaAircraft* Aircraft = nullptr;
	Stack.StepCompiledIn(&Aircraft);
	Stack.IncrementCode();
	auto GameMode = static_cast<AFortGameModeAthena*>(Context);

	if (UCrystal->bLategame)
	{
		for (auto& Player : GameMode->AlivePlayers)
		{
			if (Player->IsInAircraft())
			{
				Player->ServerAttemptAircraftJump({});
			}
		}

		auto GameState = (AFortGameStateAthena*)GameMode->GameState;
		GameState->SafeZonesStartTime = 0.0001f;
	}

	callOG(GameMode, "/Script/FortniteGame.FortGameModeAthena", OnAircraftExitedDropZone, Aircraft);
}

static bool bFirstPlayer = false;
void FortGameModeAthena::HandleStartingNewPlayer(UObject* Context, FFrame& Stack) {
	AFortPlayerControllerAthena* NewPlayer;
	Stack.StepCompiledIn(&NewPlayer);
	Stack.IncrementCode();
	auto GameMode = (AFortGameModeAthena*)Context;
	auto GameState = (AFortGameStateAthena*)GameMode->GameState;
	if (!NewPlayer) return;
	AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)NewPlayer->PlayerState;
	if (!PlayerState) return;

	FGameMemberInfo Member;
	Member.MostRecentArrayReplicationKey = -1;
	Member.ReplicationID = -1;
	Member.ReplicationKey = -1;
	Member.TeamIndex = PlayerState->TeamIndex;
	Member.SquadId = PlayerState->SquadId;
	Member.MemberUniqueId = PlayerState->UniqueId;

	GameState->GameMemberInfoArray.Members.Add(Member);
	GameState->GameMemberInfoArray.MarkItemDirty(Member);

	if (!NewPlayer->MatchReport)
	{
		NewPlayer->MatchReport = reinterpret_cast<UAthenaPlayerMatchReport*>(UGameplayStatics::SpawnObject(UAthenaPlayerMatchReport::StaticClass(), NewPlayer));
	}

	if (UCrystal->bCreative && !bFirstPlayer)
	{
		AFortCreativePortalManager* PortalManager = GameState->CreativePortalManager;

		AFortAthenaCreativePortal* Portal = nullptr;
		for (int i = 0; i < PortalManager->AvailablePortals.Num(); i++)
		{
			auto CurrentPortal = PortalManager->AvailablePortals[i];
			PortalManager->AvailablePortals.Remove(0);
			PortalManager->UsedPortals.Add(CurrentPortal);

			Portal = CurrentPortal;
			break;
		}

		if (Portal)
		{
			Portal->OwningPlayer = PlayerState->UniqueId;
			Portal->OnRep_OwningPlayer();

			if (!Portal->bPortalOpen) {
				Portal->bPortalOpen = true;
				Portal->OnRep_PortalOpen();
			}

			Portal->PlayersReady.Add(PlayerState->UniqueId);
			Portal->OnRep_PlayersReady();

			Portal->bUserInitiatedLoad = true;
			Portal->bInErrorState = false;

			Portal->bIsPublishedPortal = false;
			Portal->OnRep_PublishedPortal();

			Portal->bUserInitiatedLoad = true;
			Portal->bInErrorState = false;

			Portal->IslandInfo.AltTitle = UKismetTextLibrary::Conv_StringToText(L"Island 1");
			Portal->IslandInfo.CreatorName = PlayerState->GetPlayerName();
			Portal->IslandInfo.Mnemonic = L"";

			if (Portal->bIsPublishedPortal) {
				Portal->IslandInfo.SupportCode = PlayerState->GetPlayerName();
				Portal->IslandInfo.Version = 1;
			}

			Portal->OnRep_IslandInfo();

			NewPlayer->OwnedPortal = Portal;
			NewPlayer->CreativePlotLinkedVolume = Portal->LinkedVolume;
			NewPlayer->OnRep_CreativePlotLinkedVolume();

			NewPlayer->CreativePlotLinkedVolume->bNeverAllowSaving = false;
			NewPlayer->CreativePlotLinkedVolume->VolumeState = EVolumeState::Ready;
			NewPlayer->CreativePlotLinkedVolume->OnRep_VolumeState();

			auto LevelStreamComponent = Portal->GetLinkedVolume()->GetComponentByClass(UPlaysetLevelStreamComponent::StaticClass())->Cast<UPlaysetLevelStreamComponent>();
			auto LevelSaveComponent = Portal->GetLinkedVolume()->GetComponentByClass(UFortLevelSaveComponent::StaticClass())->Cast<UFortLevelSaveComponent>();
			auto Playset = Runtime::StaticFindObject<UFortPlaysetItemDefinition>("/Game/Playsets/PID_Playset_105x105_Composed_Desert_02.PID_Playset_105x105_Composed_Desert_02");

			if (NewPlayer->CreativePlotLinkedVolume)
			{
				NewPlayer->CreativePlotLinkedVolume->SetCurrentPlayset(Playset);
				if (LevelSaveComponent)
				{
					LevelSaveComponent->AccountIdOfOwner = PlayerState->UniqueId;
					LevelSaveComponent->LoadedLinkData = Portal->IslandInfo;
					LevelSaveComponent->bIsLoaded = true;
					LevelSaveComponent->bLoadPlaysetFromPlot = true;
					LevelSaveComponent->bAutoLoadFromRestrictedPlotDefinition = true;


					if (LevelSaveComponent->LoadedPlot)
					{
						LevelSaveComponent->LoadedPlot->IslandTitle = L"Island 1";
					}

					LevelSaveComponent->OnRep_LoadedPlotInstanceId();
					LevelSaveComponent->OnRep_LoadedLinkData();
				}
			}

			NewPlayer->bBuildFree = true;

			FortPlaysetItemDefinition::ShowPlayset(Playset, Portal->GetLinkedVolume());
			AFortMinigameSettingsBuilding* Actor = Runtime::SpawnActorV3<AFortMinigameSettingsBuilding>(Portal->GetLinkedVolume()->K2_GetActorLocation(), FRotator(), Runtime::StaticLoadObject<UClass>("/Game/Athena/Items/Gameplay/MinigameSettingsControl/MinigameSettingsMachine.MinigameSettingsMachine_C"), Portal->GetLinkedVolume());
		}
	}

	return callOG(GameMode, "/Script/Engine.GameModeBase", HandleStartingNewPlayer, NewPlayer);
}

void FortGameModeAthena::StartNewSafeZonePhase(AFortGameModeAthena* GameMode, int a2)
{
	auto GameState = (AFortGameStateAthena*)GameMode->GameState;

	if (UCrystal->bLategame)
	{
		static auto RafiusOffset = GameMode->SafeZoneIndicator->GetSafeZoneRadius();
		static int LateGamePhase = GameMode->SafeZonePhase = 3;

		int Phase = GameMode->SafeZonePhase;
		static bool bFillDurations = false;

		auto SZD = GameState->MapInfo->SafeZoneDefinition;
		auto HoldDurationOffset = 0x1E8;
		auto ZoneDurationOffset = 0x1F8;

		auto SafeZoneDefMap = GameState->FindClass("SafeZoneDefinition");
		auto& HoldDurations = *(TArray<float>*)(__int64(SafeZoneDefMap) + HoldDurationOffset);
		auto& ZoneDurations = *(TArray<float>*)(__int64(SafeZoneDefMap) + ZoneDurationOffset);
		auto ShrinkTimeName = UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"Default.SafeZone.ShrinkTime");
		auto HoldShrinkTimeName = UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"Default.SafeZone.WaitTime");

		float HoldDuration = 0.0f;
		float ZoneDuration = 0.0f;

		auto GetDuration = [&](int minPhase, float defaultDuration, const TArray<float>& DurationsArray) -> float {
			return (Phase >= minPhase && Phase < DurationsArray.Num()) ? DurationsArray[Phase] : defaultDuration;
			};

		switch (Phase)
		{
		case 2:
			HoldDuration = GetDuration(80, 80.0f, HoldDurations);
			if (HoldDuration == 80.0f)
				ZoneDuration = GetDuration(70, 70.0f, ZoneDurations);
			if (ZoneDuration == 70.0f)
				break;
		case 3:
			HoldDuration = GetDuration(60, 60.0f, HoldDurations);
			if (HoldDuration == 60.0f)
				ZoneDuration = GetDuration(60, 60.0f, ZoneDurations);
			if (ZoneDuration == 60.0f)
				break;
		case 4:
		case 5:
		case 6:
			HoldDuration = GetDuration(30, 30.0f, HoldDurations);
			if (HoldDuration == 30.0f)
				ZoneDuration = GetDuration(60, 60.0f, ZoneDurations);
			if (ZoneDuration == 60.0f)
				break;
		case 7:
			ZoneDuration = GetDuration(55, 55.0f, ZoneDurations);
			if (ZoneDuration == 55.0f)
				break;
		case 8:
			ZoneDuration = GetDuration(44, 44.0f, ZoneDurations);
			if (ZoneDuration == 44.0f)
				break;
		case 9:
			ZoneDuration = GetDuration(90, 90.0f, ZoneDurations);
			if (ZoneDuration == 90.0f)
				break;
		default:
			break;
		}

		float CurrentTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
		GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = CurrentTime + HoldDuration;
		GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime + ZoneDuration;
		static FVector ZoneLocation = GameMode->SafeZoneLocations[4];
		GameMode->SafeZoneIndicator->NextCenter.X = ZoneLocation.X;
		GameMode->SafeZoneIndicator->NextCenter.Y = ZoneLocation.Y;
		GameMode->SafeZoneIndicator->NextCenter.Z = ZoneLocation.Z;

		StartNewSafeZonePhaseOG(GameMode, a2);

		static FVector_NetQuantize100 ZoneLocationQuantize = FVector_NetQuantize100{ ZoneLocation.X, ZoneLocation.Y, ZoneLocation.Z };

		if (GameMode->SafeZonePhase == 2 || GameMode->SafeZonePhase == 3)
		{
			if (GameMode->SafeZoneIndicator)
			{
				GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = GameState->GetServerWorldTimeSeconds();
				GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameState->GetServerWorldTimeSeconds() + 0.2;
			}
		}

		if (GameMode->SafeZonePhase == 4)
		{
			GameMode->SafeZoneIndicator->NextCenter = ZoneLocationQuantize;
			GameMode->SafeZoneIndicator->NextRadius = 10000.f;
			GameMode->SafeZoneIndicator->LastRadius = 20000.f;
		}

		if (GameMode->SafeZonePhase == 5)
		{
			GameMode->SafeZoneIndicator->NextCenter = ZoneLocationQuantize;
			GameMode->SafeZoneIndicator->NextRadius = 5000.f;
			GameMode->SafeZoneIndicator->LastRadius = 10000.f;
		}

		if (GameMode->SafeZonePhase == 6)
		{
			GameMode->SafeZoneIndicator->NextCenter = ZoneLocationQuantize;
			GameMode->SafeZoneIndicator->NextRadius = 2500.f;
			GameMode->SafeZoneIndicator->LastRadius = 5000.f;
		}

		if (GameMode->SafeZonePhase == 7)
		{
			GameMode->SafeZoneIndicator->NextCenter = ZoneLocationQuantize;
			GameMode->SafeZoneIndicator->NextRadius = 1650.2f;
			GameMode->SafeZoneIndicator->LastRadius = 2500.f;
		}

		if (GameMode->SafeZonePhase == 8)
		{
			GameMode->SafeZoneIndicator->NextCenter = ZoneLocationQuantize;
			GameMode->SafeZoneIndicator->NextRadius = 1090.12f;
			GameMode->SafeZoneIndicator->LastRadius = 1650.2f;
		}

		LateGamePhase++;
		return;
	}

	return StartNewSafeZonePhaseOG(GameMode, a2);
}

bool FortGameModeAthena::StartAircraftPhase(AFortGameModeAthena* GameMode, char a2)
{
	auto GameState = (AFortGameStateAthena*)GameMode->GameState;

	if (UCrystal->bLategame)
	{
		auto LocalAircraft = GameState->GetAircraft(0);
		if (!LocalAircraft) {
			std::cout << "no aircraft" << std::endl;
			return StartAircraftPhaseOG(GameMode, a2);
		}
		FVector SafeZoneCenter = GameMode->SafeZoneLocations[3];
		SafeZoneCenter.Z += 15000;

		LocalAircraft->K2_SetActorLocation(SafeZoneCenter, false, nullptr, true);

		auto MapInfo = GameState->MapInfo;
		if (MapInfo) {
			for (int i = 0; i < MapInfo->FlightInfos.Num(); i++) {
				auto& FlightInfo = MapInfo->FlightInfos[i];
				FlightInfo.FlightStartLocation = FVector_NetQuantize100(SafeZoneCenter);
				FlightInfo.FlightSpeed = 0.0f;
				FlightInfo.TimeTillFlightEnd = 0.0f;
				FlightInfo.TimeTillDropStart = 0.0f;
				FlightInfo.TimeTillDropEnd = 8.25f;
				LocalAircraft->FlightStartTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
				LocalAircraft->FlightEndTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + 8.25f;
				LocalAircraft->FlightInfo = FlightInfo;
				GameState->bGameModeWillSkipAircraft = true;
				MapInfo->AircraftDesiredDoorOpenTime.Value = 1;
				GameState->bAircraftIsLocked = true;
				GameState->SafeZonesStartTime = 0;
				GameState->OnRep_MapInfo();
			}
		}
	}

	return StartAircraftPhaseOG(GameMode, a2);
}

void FortGameModeAthena::OnAircraftEnteredDropZone(UObject* Context, FFrame& Stack)
{
	AFortAthenaAircraft* Aircraft;
	Stack.StepCompiledIn(&Aircraft);
	Stack.IncrementCode();

	auto GameMode = (AFortGameModeAthena*)Context;
	auto GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

	return callOG(GameMode, "/Script/FortniteGame.FortGameModeAthena", OnAircraftEnteredDropZone, Aircraft);
}

void FortGameModeAthena::Patch()
{
	Runtime::Exec("/Script/FortniteGame.FortGameModeAthena.OnAircraftExitedDropZone", OnAircraftExitedDropZone, OnAircraftExitedDropZoneOG);
	Runtime::Exec("/Script/Engine.GameMode.ReadyToStartMatch", ReadyToStartMatch, ReadyToStartMatchOG);
	Runtime::Exec("/Script/Engine.GameModeBase.HandleStartingNewPlayer", HandleStartingNewPlayer, HandleStartingNewPlayerOG);
	Runtime::Exec("/Script/Engine.GameModeBase.SpawnDefaultPawnFor", SpawnDefaultPawnFor, SpawnDefaultPawnForOG);

	Runtime::Exec("/Script/FortniteGame.FortGameModeAthena.OnAircraftEnteredDropZone", OnAircraftEnteredDropZone, OnAircraftEnteredDropZoneOG);

	if (UCrystal->bCreative) Runtime::Hook(Runtime::Offsets::ImageBase + 0x11d42b0, PickTeam, (void**)&PickTeamOG);
	Runtime::Hook(Runtime::Offsets::ImageBase + 0x11e4cb0, StartAircraftPhase, (void**)&StartAircraftPhaseOG);
	Runtime::Hook(Runtime::Offsets::StartNewSafeZonePhase, StartNewSafeZonePhase, (void**)&StartNewSafeZonePhaseOG);
}