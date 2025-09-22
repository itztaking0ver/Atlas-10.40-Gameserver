#include "../Public/FortPlaysetItemDefinition.h"

__int64 (*LoadPlaysetOG)(UPlaysetLevelStreamComponent*) = decltype(LoadPlaysetOG)(__int64(Runtime::Offsets::ImageBase + 0x1A3A0A0));
void FortPlaysetItemDefinition::ShowPlayset(UFortPlaysetItemDefinition* Playset, AFortVolume* Volume)
{
	auto LevelStreamComponent = (UPlaysetLevelStreamComponent*)Volume->GetComponentByClass(UPlaysetLevelStreamComponent::StaticClass());
	if (!LevelStreamComponent) return;

	LevelStreamComponent->SetPlayset(Playset);
	LevelStreamComponent->OnRep_ClientPlaysetData();
	LoadPlaysetOG(LevelStreamComponent);
}