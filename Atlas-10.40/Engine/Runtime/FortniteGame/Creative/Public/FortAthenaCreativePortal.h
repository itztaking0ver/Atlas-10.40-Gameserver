#pragma once
#include "../Runtime.h"

namespace FortAthenaCreativePortal
{
	static void TeleportPlayerToLinkedVolume(AFortAthenaCreativePortal* Portal, FFrame& Stack);
	static void ServerTeleportToPlaygroundLobbyIsland(AFortPlayerControllerAthena* Controller);

	void Patch();
}