#pragma once
#include <stdint.h>
#include <vector>

namespace Runtime {
    namespace Offsets {
        inline uint64_t ImageBase = *(uint64_t*)(__readgsqword(0x60) + 0x10);
        inline uint64_t ReadyToStartMatch = ImageBase + 0x11d8640;
        inline uint64_t SpawnDefaultPawnFor = ImageBase + 0x11e1180;
        inline uint64_t ServerAcknowledgePossession = ImageBase + 0x382da0;
        inline uint64_t HandleStartingNewPlayer = ImageBase + 0x15bede0;
        inline uint64_t ServerExecuteInventoryItem = ImageBase + 0x382da0;
        inline uint64_t ServerAttemptAircraftJump = ImageBase + 0x1251a50;
        inline uint64_t CreateNetDriver = ImageBase + 0x347faf0;
        inline uint64_t InitHost = ImageBase + 0x6f5a30;
        inline uint64_t PauseBeaconRequests = ImageBase + 0x17f03d0;
        inline uint64_t InitListen = ImageBase + 0x6f5f90;
        inline uint64_t SetWorld = ImageBase + 0x31edf40;
        inline uint64_t WorldNetMode = ImageBase + 0x34d2140;
        inline uint64_t GIsClient = ImageBase + 0x637925b;
        inline uint64_t TickFlush = ImageBase + 0x31eecb0;
        inline uint64_t GetMaxTickRate = ImageBase + 0x3482550;
        inline uint64_t DispatchRequest = ImageBase + 0xbaed60;
        inline uint64_t StaticFindObject = ImageBase + 0x22fb1e0;
        inline uint64_t StaticLoadObject = ImageBase + 0x22fc4c0;
        inline uint64_t ApplyCharacterCustomization = ImageBase + 0x1a1f5f0;
        inline uint64_t InternalTryActivateAbility = ImageBase + 0x9367f0;
        inline uint64_t InternalServerTryActivateAbility = ImageBase + 0x382da0;
        inline uint64_t ServerReplicateActors = ImageBase + 0xa33e90;
        inline uint64_t Realloc = ImageBase + 0x2093d50;
        inline uint64_t ConstructAbilitySpec = ImageBase + 0x958f90;
        inline uint64_t InternalGiveAbility = ImageBase + 0x935010;
        inline uint64_t StartNewSafeZonePhase = ImageBase + 0x11e72c0;
        inline uint64_t PickTeam = ImageBase + 0x11d42b0;
        inline uint32_t ReadyToStartMatchVft = 0xfc;
        inline uint32_t SpawnDefaultPawnForVft = 0xc3;
        inline uint32_t ServerAcknowledgePossessionVft = 0x108;
        inline uint32_t HandleStartingNewPlayerVft = 0xc9;
        inline uint32_t ServerExecuteInventoryItemVft = 0x1fe;
        inline uint32_t ServerAttemptAircraftJumpVft = 0x28a;
        inline uint32_t ServerOnBeginResurrectionInteractionVft = 0x7C;
        inline uint32_t ServerOnInterruptResurrectionInteractionVft = 0x7A;
        inline uint32_t InternalServerTryActivateAbilityVft = 0xf6;
        inline uint64_t GameSessionPatch = ImageBase + 0x1223845;
        inline uint64_t GameSessionPatch2 = ImageBase + 0x17D9E5D;
        inline uint64_t EncryptionPatch = ImageBase + 0x34d4c6a;
        inline uint64_t GiveAbilityAndActivateOnce = ImageBase + 0x935130;

        inline std::vector<uint64_t> NullFuncs = { ImageBase + 0x2c94e30, ImageBase + 0x227d720, ImageBase + 0x10C56B0, ImageBase + 0x10C5BB0, ImageBase + 0x7A0DD0, ImageBase + 0x1250730 };
        inline std::vector<uint64_t> RetTrueFuncs = { ImageBase + 0x30b2b70, ImageBase + 0x31868c0, ImageBase + 0x22a30c0, WorldNetMode, };
    }
}