#include "../Public/FortMinigame.h"

// we love ida right chat
void FortMinigame::EndGame(AFortMinigame* Minigame, AFortPlayerController* Controller, EFortMinigameEnd EndMethod)
{
	static bool IsMatchpointReached = false;
	if (!Controller) return;

	Minigame->TotalRounds = 5;

	bool bIsMatchpoint = false;
	if (EndMethod == EFortMinigameEnd::EndRound)
	{
		bIsMatchpoint = (Minigame->CurrentRound >= Minigame->TotalRounds);
	}

	Minigame->RoundWinnerDisplayTime = 5.f;
	Minigame->RoundScoreDisplayTime = 1.0f;

	Minigame->CurrentState = EFortMinigameState::PostGameEnd;
	Minigame->QueueAllAIForDespawn();
}

void FortMinigame::Patch()
{
	Runtime::Exec("/Script/FortniteGame.FortMinigame.EndGame", EndGame);
}
