#pragma once
#include "framework.h"
#include "PlayerBots.h"
#include "Bots.h"

#include "BotNames.h"

namespace AIController
{
	void (*OnActorBumpOG)(UPathFollowingComponent* Comp, AActor* SelfActor, AActor* OtherActor, const FVector& NormalImpulse, FHitResult& Hit);
	void OnActorBump(UFortAthenaAIBotPathFollowingComponent* Comp, AActor* SelfActor, AActor* OtherActor, const FVector& NormalImpulse, FHitResult& Hit)
	{
		if (OtherActor) {
			for (PlayerBots::PlayerBot* bot : PlayerBots::PlayerBotArray)
			{
				if (bot->PC == Comp->BotController)
				{
					if (bot->BotState < PlayerBots::EBotState::Landed) {
						break;
					}

					if (bot->bPotentiallyUnderAttack) {
						if (!bot->Pawn->bIsCrouched && UKismetMathLibrary::GetDefaultObj()->RandomBoolWithWeight(0.025f)) {
							bot->Pawn->Crouch(false);
						}

						if (UKismetMathLibrary::GetDefaultObj()->RandomBoolWithWeight(0.05f)) {
							bot->Pawn->UnCrouch(false);
							bot->Pawn->Jump();
						}
						break;
					}
					bot->bPauseTicking = true;
					if (!bot->IsPickaxeEquiped()) {
						bot->EquipPickaxe();
					}
					bot->Pawn->PawnStartFire(0);

					if (!bot->Pawn->bIsCrouched && UKismetMathLibrary::GetDefaultObj()->RandomBoolWithWeight(0.025f)) {
						bot->Pawn->Crouch(false);
					}

					if (UKismetMathLibrary::GetDefaultObj()->RandomBoolWithWeight(0.05f)) {
						bot->Pawn->UnCrouch(false);
						bot->Pawn->Jump();
					}

					bot->ForceStrafe(true);

					bot->Pawn->PawnStopFire(0);
					bot->bPauseTicking = false;
					break;
				}
			}
		}

		return OnActorBumpOG(Comp, SelfActor, OtherActor, NormalImpulse, Hit);
	}

	void HookAll() {
		HookVTable<UPathFollowingComponent>(0x8D, OnActorBump, (LPVOID*)&OnActorBumpOG);

		Log("Hooked AIController!");
	}
}