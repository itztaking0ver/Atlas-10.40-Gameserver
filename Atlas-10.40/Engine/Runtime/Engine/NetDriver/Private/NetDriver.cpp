#include "../Public/NetDriver.h"

void NetDriver::TickFlush(UNetDriver* NetDriver, float DeltaSeconds, bool bAllowFrameRateSmoothing)
{
	if (!NetDriver) return TickFlushOG(NetDriver, DeltaSeconds, bAllowFrameRateSmoothing);

	AFortGameModeAthena* GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
	AFortGameStateAthena* GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;

	static void (*ServerReplicateActors)(UReplicationDriver*) = decltype(ServerReplicateActors)((uintptr_t)GetModuleHandleW(0) + 0xA33E90);
	if (NetDriver->ReplicationDriver) ServerReplicateActors(NetDriver->ReplicationDriver);

	if (GetKeyState(VK_F2)) {
		UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"startaircraft", nullptr);
	}

    return TickFlushOG(NetDriver, DeltaSeconds, bAllowFrameRateSmoothing);
}

void NetDriver::Patch()
{
    Runtime::Hook(Runtime::Offsets::TickFlush, TickFlush, (void**)&TickFlushOG);
}
