#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include "SDK/SDK.hpp"
#include <algorithm>
#include <numeric>
#include <map>
#include <intrin.h>
#include <array>
#include <tlhelp32.h>
#include <future>
#include <iomanip>  
#include "Minhook/MinHook.h"
#include "Offsets.h"
#include <set>
#include <vector>
#include <time.h>
#include <fstream>
#include <cmath>

using namespace SDK;

// Forward declarations
inline FQuat RotatorToQuat(FRotator Rotation);

template <class X>
using xset = std::set<X, TMemoryAllocator<X>>;
template <class X>
using xvector = std::vector<X, TMemoryAllocator<X>>;
template <class X, class Y>
using xmap = std::map<X, Y, std::less<X>, TMemoryAllocator<std::pair<const X, Y>>>;

inline float GetMaxTickRate() { return 30.0f; }
inline int RetTrue() { return true; }
inline void KickPlayer(AGameSession* GameSession, AFortPlayerControllerAthena* Controller) {};

// Global variables for bot system
static auto ImageBase = InSDKUtils::GetImageBase();

static UFortBuildingItemDefinition* Floor = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Floor.BuildingItemData_Floor");
static UFortBuildingItemDefinition* Roof = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_RoofS.BuildingItemData_RoofS");
static UFortBuildingItemDefinition* Stair = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Stair_W.BuildingItemData_Stair_W");
static UFortBuildingItemDefinition* Wall = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Wall.BuildingItemData_Wall");
static UFortEditToolItemDefinition* EditTool = UObject::FindObject<UFortEditToolItemDefinition>("FortEditToolItemDefinition EditTool.EditTool");

static std::vector<UAthenaCharacterItemDefinition*> Characters{};
static std::vector<UAthenaPickaxeItemDefinition*> Pickaxes{};
static std::vector<UAthenaBackpackItemDefinition*> Backpacks{};
static std::vector<UAthenaGliderItemDefinition*> Gliders{};
static std::vector<UAthenaSkyDiveContrailItemDefinition*> Contrails{};
inline std::vector<UAthenaDanceItemDefinition*> Dances{};

static TArray<AActor*> BuildingFoundations;
static TArray<AActor*> PlayerStarts;

// Function declarations for bot system
static void* (*StaticFindObjectOG)(UClass*, UObject* Package, const wchar_t* OrigInName, bool ExactClass) = decltype(StaticFindObjectOG)(ImageBase + 0x22FB1E0);
static void* (*StaticLoadObjectOG)(UClass* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* Filename, uint32_t LoadFlags, UObject* Sandbox, bool bAllowObjectReconciliation, void*) = decltype(StaticLoadObjectOG)(ImageBase + 0x22FC4C0);

static void(*GiveAbilityOG)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec) = decltype(GiveAbilityOG)(ImageBase + 0x935010);
static void (*AbilitySpecConstructor)(FGameplayAbilitySpec*, UGameplayAbility*, int, int, UObject*) = decltype(AbilitySpecConstructor)(ImageBase + 0x958F90);
static bool (*InternalTryActivateAbility)(UAbilitySystemComponent* AbilitySystemComp, FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData) = decltype(InternalTryActivateAbility)(ImageBase + 0x9367F0);
static FGameplayAbilitySpecHandle(*GiveAbilityAndActivateOnce)(UAbilitySystemComponent* ASC, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec) = decltype(GiveAbilityAndActivateOnce)(ImageBase + 0x935130);

template<typename T>
static inline T* StaticLoadObject(const std::string& Name)
{
    auto ConvName = std::wstring(Name.begin(), Name.end());
    T* Object = (T*)StaticFindObjectOG(T::StaticClass(), nullptr, ConvName.c_str(), false);
    if (!Object)
    {
        Object = (T*)StaticLoadObjectOG(T::StaticClass(), nullptr, ConvName.c_str(), nullptr, 0, nullptr, false, nullptr);
    }
    return Object;
}

inline FQuat RotatorToQuat(FRotator Rotation)
{
    FQuat Quat;
    const float DEG_TO_RAD = 3.14159f / 180.0f;
    const float DIVIDE_BY_2 = DEG_TO_RAD / 2.0f;

    float SP = sin(Rotation.Pitch * DIVIDE_BY_2);
    float CP = cos(Rotation.Pitch * DIVIDE_BY_2);
    float SY = sin(Rotation.Yaw * DIVIDE_BY_2);
    float CY = cos(Rotation.Yaw * DIVIDE_BY_2);
    float SR = sin(Rotation.Roll * DIVIDE_BY_2);
    float CR = cos(Rotation.Roll * DIVIDE_BY_2);

    Quat.X = static_cast<float>(CR * SP * SY - SR * CP * CY);
    Quat.Y = static_cast<float>(-CR * SP * CY - SR * CP * SY);
    Quat.Z = static_cast<float>(CR * CP * SY - SR * SP * CY);
    Quat.W = static_cast<float>(CR * CP * CY + SR * SP * SY);

    return Quat;
}

template<typename T>
inline T* SpawnActor(FVector Loc, FRotator Rot = FRotator(), AActor* Owner = nullptr, SDK::UClass* Class = T::StaticClass(), ESpawnActorCollisionHandlingMethod Handle = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)
{
    FTransform Transform{};
    Transform.Scale3D = FVector{ 1,1,1 };
    Transform.Translation = Loc;
    Transform.Rotation = RotatorToQuat(Rot);
    return (T*)UGameplayStatics::FinishSpawningActor(UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), Class, Transform, Handle, Owner), Transform);
}

template<typename T>
inline T* SpawnActorClass(FVector Loc, FRotator Rot, SDK::UClass* Class, AActor* Owner = nullptr, ESpawnActorCollisionHandlingMethod Handle = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)
{
    FTransform Transform{};
    Transform.Scale3D = FVector{ 1,1,1 };
    Transform.Translation = Loc;
    Transform.Rotation = RotatorToQuat(Rot);
    return (T*)UGameplayStatics::FinishSpawningActor(UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), Class, Transform, Handle, Owner), Transform);
}

inline void Log(const std::string& msg)
{
    static bool firstCall = true;

    if (firstCall)
    {
        std::ofstream logFile("Atlas_log.txt", std::ios::trunc);
        if (logFile.is_open())
        {
            logFile << "[ATLAS]: Log file initialized!" << std::endl;
            logFile.close();
        }
        firstCall = false;
    }

    std::ofstream logFile("Atlas_log.txt", std::ios::app);
    if (logFile.is_open())
    {
        logFile << "[ATLAS]: " << msg << std::endl;
        logFile.close();
    }

    std::cout << "[ATLAS]: " << msg << std::endl;
}


#define DefUHookOgRet(_Rt, _Name) static inline _Rt (*_Name##OG)(UObject*, FFrame&, _Rt*); static _Rt _Name(UObject *, FFrame&, _Rt*); 
#define DefHookOg(_Rt, _Name, ...) static inline _Rt (*_Name##OG)(##__VA_ARGS__); static _Rt _Name(##__VA_ARGS__); 