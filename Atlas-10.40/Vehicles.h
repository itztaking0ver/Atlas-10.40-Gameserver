#pragma once
#include "framework.h"

namespace Vehicles {
    void SpawnVehicles()
    {
        // Only spawn vehicles if lategame is off
        if (UAtlas && UAtlas->bLategame) {
            Log("Vehicles disabled - lategame is on");
            return;
        }

        if (!UAtlas || !UAtlas->bVehiclesEnabled) {
            Log("Vehicles disabled - not enabled in configuration");
            return;
        }

        UGameplayStatics* Statics = UGameplayStatics::GetDefaultObj();

        TArray<AActor*> Spawners;

        UClass* Class = AFortAthenaVehicleSpawner::StaticClass();

        Statics->GetAllActorsOfClass(UWorld::GetWorld(), Class, &Spawners);

        for (int i = 0; i < Spawners.Num(); i++)
        {
            AFortAthenaVehicleSpawner* Vehicle = (AFortAthenaVehicleSpawner*)Spawners[i];
            SpawnActorClass<AFortAthenaVehicleSpawner>(Vehicle->K2_GetActorLocation(), Vehicle->K2_GetActorRotation(), Vehicle->GetVehicleClass());
        }

        Spawners.Free();
    }

    void EnableVehicles() {
        if (UAtlas) {
            UAtlas->bVehiclesEnabled = true;
            Log("Vehicles enabled");
        }
    }

    void DisableVehicles() {
        if (UAtlas) {
            UAtlas->bVehiclesEnabled = false;
            Log("Vehicles disabled");
        }
    }
}
