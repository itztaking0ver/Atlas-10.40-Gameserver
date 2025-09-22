#pragma once
#include "../Runtime.h"

namespace FortQuestManager
{
    static AFortPlayerController* GetPlayerControllerFromQuestManager(UFortQuestManager* QuestManager);

    void SendStatEvent(UFortQuestManager*, UObject*, FGameplayTagContainer, FGameplayTagContainer, FGameplayTagContainer, int32, EFortQuestObjectiveStatEvent, UFortQuestItem** = nullptr, FName = FName());

    static void UpdateQuest(UFortQuestManager* QuestManager, UFortQuestItem* QuestItem, const UFortQuestItemDefinition* QuestDefinition, const FFortMcpQuestObjectiveInfo& Objective, int32 Count);
    static void SendObjectiveStat(AFortPlayerControllerAthena* PlayerController, const FName& BackendName, const UFortQuestItemDefinition* QuestDefinition, int32 Count);

    DefHookOg(void, SendComplexCustomStatEvent, UFortQuestManager*, UObject*, FGameplayTagContainer&, FGameplayTagContainer&, bool*, bool*, int32);

    void Patch();
}