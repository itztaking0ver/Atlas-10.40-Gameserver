#include "../Public/FortPlayerPawn.h"

void FortPlayerPawn::ServerSendZiplineState(UObject* Context, FFrame& Stack)
{
	FZiplinePawnState State;
	Stack.StepCompiledIn(&State);
	Stack.IncrementCode();
	auto Pawn = (AFortPlayerPawn*)Context;
	if (!Pawn) return;

	Pawn->ZiplineState = State;
	((void (*)(AFortPlayerPawn*))(Runtime::Offsets::ImageBase + 0x1967D30))(Pawn);

	if (State.bJumped)
	{
		auto Velocity = Pawn->CharacterMovement->Velocity;
		auto VelocityX = Velocity.X * -0.5f;
		auto VelocityY = Velocity.Y * -0.5f;
		Pawn->LaunchCharacterJump({ VelocityX >= -750 ? fminf(VelocityX, 750) : -750, VelocityY >= -750 ? fminf(VelocityY, 750) : -750, 1200 }, false, false, true, true);
	}
}

void FortPlayerPawn::Patch()
{
	Runtime::Exec("/Script/FortniteGame.FortPlayerPawn.ServerSendZiplineState", ServerSendZiplineState);
}
