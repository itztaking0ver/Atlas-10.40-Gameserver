#include "../Public/FortMinigameSettingsBuilding.h"

void FortMinigameSettingsBuilding::BeginPlay(AFortMinigameSettingsBuilding* Minigame)
{
	Minigame->SettingsVolume = (AFortVolume*)Minigame->GetOwner();
}

void FortMinigameSettingsBuilding::Patch()
{
	Runtime::Hook(Runtime::Offsets::ImageBase + 0x1C96A40, BeginPlay);
}
