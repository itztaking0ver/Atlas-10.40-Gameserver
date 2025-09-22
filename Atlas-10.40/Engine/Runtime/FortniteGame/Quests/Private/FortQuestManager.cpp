#include "../Public/FortQuestManager.h"


AFortPlayerController* FortQuestManager::GetPlayerControllerFromQuestManager(UFortQuestManager* QuestManager)
{
    for (const auto& CurrentConnection : UWorld::GetWorld()->NetDriver->ClientConnections)
    {
        if (!CurrentConnection) continue;

        auto CurrentController = (AFortPlayerController*)CurrentConnection->PlayerController;
        if (!CurrentController) continue;

        if (CurrentController->GetQuestManager(ESubGame::Athena) == QuestManager) return CurrentController;
    }

    return nullptr;
}

void FortQuestManager::SendStatEvent(UFortQuestManager* QuestManager, UObject* TargetObject, FGameplayTagContainer SourceTags, FGameplayTagContainer TargetTags, FGameplayTagContainer ContextTags, int32 Count, EFortQuestObjectiveStatEvent StatEvent, UFortQuestItem** QuestItem, FName ForceStatRowName)
{
    auto GameMode = reinterpret_cast<AFortGameModeAthena*>(UWorld::GetWorld()->AuthorityGameMode);
    if (!GameMode) return;

    auto GameState = reinterpret_cast<AFortGameStateAthena*>(GameMode->GameState);
    if (!GameState) return;

    if (!QuestManager) return;
    if (Count < 1) return;
    if (StatEvent >= EFortQuestObjectiveStatEvent::EFortQuestObjectiveStatEvent_MAX || StatEvent < EFortQuestObjectiveStatEvent::Kill) return;

    FGameplayTagContainer PlayerSourceTags;
    FGameplayTagContainer PlayerContextTags;
    QuestManager->GetSourceAndContextTags(&PlayerSourceTags, &PlayerContextTags);

    ContextTags.AppendTags(GameState->CurrentPlaylistInfo.BasePlaylist->GameplayTagContainer);
    ContextTags.AppendTags(PlayerContextTags);
    SourceTags.AppendTags(PlayerSourceTags);

    TArray<UFortQuestItem*> CurrentQuests;
    QuestManager->GetCurrentQuests(&CurrentQuests);

    for (const auto& CurrentQuest : CurrentQuests)
    {
        if (!CurrentQuest) continue;

        if (CurrentQuest->HasCompletedQuest()) continue;

        UFortQuestItemDefinition* QuestDefinition = CurrentQuest->GetQuestDefinitionBP();
        if (!QuestDefinition) continue;

        if (QuestManager->HasCompletedQuest(QuestDefinition)) continue;

        for (const auto& Objective : QuestDefinition->Objectives)
        {
            if (QuestManager->HasCompletedObjectiveWithName(QuestDefinition, Objective.BackendName) ||
                QuestManager->HasCompletedObjective(QuestDefinition, Objective.ObjectiveStatHandle) ||
                CurrentQuest->HasCompletedObjectiveWithName(Objective.BackendName) ||
                CurrentQuest->HasCompletedObjective(Objective.ObjectiveStatHandle))
            {
                continue;
            }

            UDataTable* StatTable = Objective.ObjectiveStatHandle.DataTable;
            FName StatRowName = Objective.ObjectiveStatHandle.RowName;

            if (ForceStatRowName.IsValid())
            {
                StatRowName = ForceStatRowName;
            }

            if (StatTable && StatRowName.IsValid())
            {
                for (const auto& [RowName, Row] : (TMap<FName, FFortQuestObjectiveStatTableRow*>)StatTable->RowMap)
                {
                    if (!RowName.IsValid() || !Row) continue;
                    if (RowName != StatRowName) continue;
                    if (Row->Type != StatEvent) continue;

                    if (!TargetTags.HasAll(Row->TargetTagContainer)) continue;

                    if (!SourceTags.HasAll(Row->SourceTagContainer)) continue;

                    if (!ContextTags.HasAll(Row->ContextTagContainer)) continue;

                    if (!IsConditionMet(Row->Condition, TargetTags, SourceTags, ContextTags)) continue;

                    UpdateQuest(QuestManager, CurrentQuest, QuestDefinition, Objective, Count);
                    if (QuestItem)
                    {
                        *QuestItem = CurrentQuest;
                    }
                }
            }
        }
    }
}

void FortQuestManager::UpdateQuest(UFortQuestManager* QuestManager, UFortQuestItem* QuestItem, const UFortQuestItemDefinition* QuestDefinition, const FFortMcpQuestObjectiveInfo& Objective, int32 Count)
{
    if (!QuestManager || !QuestItem || !QuestDefinition || Count <= 0) return;

    auto Controller = reinterpret_cast<AFortPlayerControllerAthena*>(QuestItem->GetOwningController());
    if (!Controller) return;

    auto PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(Controller->PlayerState);
    if (!PlayerState) return;

    auto PlayerTeam = PlayerState->PlayerTeam;
    if (!PlayerTeam) return;

    for (const auto& TeamMemberController : PlayerTeam->TeamMembers)
    {
        auto TeamMemberPlayerController = reinterpret_cast<AFortPlayerControllerAthena*>(TeamMemberController);
        if (!TeamMemberPlayerController) continue;

        if (TeamMemberPlayerController->IsA(AFortAthenaAIBotController::StaticClass())) continue;

        auto CurrentQuestManager = TeamMemberPlayerController->GetQuestManager(ESubGame::Athena);
        const AFortPlayerState* AssistingPlayerState = (TeamMemberPlayerController == Controller) ? nullptr : PlayerState;
        bool ObjectiveCompleted = Count >= Objective.Count;
        bool QuestCompleted = QuestItem->GetNumObjectivesComplete() == QuestItem->Objectives.Num() && ObjectiveCompleted;

        QuestItem->CurrentStage++;
        if (QuestCompleted)
        {
            QuestItem->quest_state = EFortQuestState::Completed;
            QuestItem->bAllObjectivesComplete = true;
        }

        const_cast<UFortQuestItemDefinition*>(QuestDefinition)->ObjectiveCompletionCount = Count;
        SendObjectiveStat(TeamMemberPlayerController, Objective.BackendName, QuestDefinition, Count);

        if (TeamMemberController == Controller)
        {
            int32 DeltaChange = Count;
            CurrentQuestManager->SelfCompletedUpdatedQuest(TeamMemberPlayerController, QuestDefinition, Objective.BackendName, Count, DeltaChange, AssistingPlayerState, ObjectiveCompleted, QuestCompleted);
        }
        else
        {
            int32 DeltaChange = Count;
            CurrentQuestManager->AssistedPlayerUpdatedQuest(TeamMemberPlayerController, QuestDefinition, Objective.BackendName, Count, DeltaChange, AssistingPlayerState, ObjectiveCompleted, QuestCompleted);
        }
    }
}

void FortQuestManager::SendObjectiveStat(AFortPlayerControllerAthena* Controller, const FName& BackendName, const UFortQuestItemDefinition* QuestDefinition, int32 Count)
{
    if (!Controller) return;
    if (!QuestDefinition) return;

    for (auto& UpdatedObjectiveStat : Controller->UpdatedObjectiveStats)
    {
        if (UpdatedObjectiveStat.BackendName == BackendName)
        {
            UpdatedObjectiveStat.Quest = const_cast<UFortQuestItemDefinition*>(QuestDefinition);
            UpdatedObjectiveStat.CurrentStage++;
            UpdatedObjectiveStat.StatDelta = Count;
            UpdatedObjectiveStat.StatValue += Count;
            Controller->OnRep_UpdatedObjectiveStats();
            return;
        }
    }

    FFortUpdatedObjectiveStat NewStat;
    NewStat.BackendName = BackendName;
    NewStat.Quest = const_cast<UFortQuestItemDefinition*>(QuestDefinition);
    NewStat.CurrentStage = 1;
    NewStat.StatDelta = Count;
    NewStat.StatValue = Count;

    Controller->UpdatedObjectiveStats.Add(NewStat);
    Controller->OnRep_UpdatedObjectiveStats();
}

void FortQuestManager::SendComplexCustomStatEvent(UFortQuestManager* QuestManager, UObject* TargetObject, FGameplayTagContainer& AdditionalSourceTags, FGameplayTagContainer& TargetTags, bool* QuestActive, bool* QuestCompleted, int32 Count)
{
    FGameplayTagContainer SourceTags;
    FGameplayTagContainer ContextTags;
    if (!QuestManager) return;

    QuestManager->GetSourceAndContextTags(&SourceTags, &ContextTags);
    SourceTags.AppendTags(AdditionalSourceTags);

    UFortQuestItem* QuestItem = nullptr;
    FortQuestManager::SendStatEvent(QuestManager, TargetObject, SourceTags, TargetTags, ContextTags, Count, EFortQuestObjectiveStatEvent::ComplexCustom, &QuestItem);
    if (QuestItem && QuestActive && QuestCompleted)
    {
        auto QuestDefinition = QuestItem->GetQuestDefinitionBP();
        *QuestActive = true;
        *QuestCompleted = QuestDefinition ? (QuestItem->GetPercentageComplete() >= 100.f) : QuestItem->HasCompletedQuest();
    }

    return FortQuestManager::SendComplexCustomStatEventOG(QuestManager, TargetObject, AdditionalSourceTags, TargetTags, QuestActive, QuestCompleted, Count);
}

void FortQuestManager::Patch()
{
    Runtime::Hook(Runtime::Offsets::ImageBase + 0x1A47BC0, SendComplexCustomStatEvent, (void**)&SendComplexCustomStatEventOG);
}