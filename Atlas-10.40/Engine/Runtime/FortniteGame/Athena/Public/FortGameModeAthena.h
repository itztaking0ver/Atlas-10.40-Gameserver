#pragma once
#include "../Runtime.h"

namespace FortGameModeAthena
{
    inline uint8_t CurrentTeam = 1;
    inline uint8_t PlayersOnCurTeam = 1;

    DefUHookOgRet(bool, ReadyToStartMatch);
    DefUHookOgRet(APawn*, SpawnDefaultPawnFor);
    DefUHookOgRet(void, OnAircraftExitedDropZone);

    DefHookOg(void, HandleStartingNewPlayer, UObject* Context, FFrame& Stack);
    DefHookOg(bool, StartAircraftPhase, AFortGameModeAthena*, char);
    DefHookOg(void, StartNewSafeZonePhase, AFortGameModeAthena*, int);
    DefHookOg(void, OnAircraftEnteredDropZone, UObject* Context, FFrame& Stack);

    HookOG(EFortTeam, PickTeam, (AFortGameModeAthena* GameMode, uint8_t PreferredTeam, AFortPlayerControllerAthena* Controller));

    void Patch();
}