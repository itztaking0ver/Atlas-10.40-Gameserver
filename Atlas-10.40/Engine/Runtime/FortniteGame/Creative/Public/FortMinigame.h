#pragma once
#include "../Runtime.h"

namespace FortMinigame
{
    void EndGame(AFortMinigame* Minigame, AFortPlayerController* Controller, EFortMinigameEnd EndMethod);
    void Patch();
}