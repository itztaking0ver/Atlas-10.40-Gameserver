#include "../Public/FortCreativeMoveTool.h"

void FortCreativeMoveTool::ServerSetAllowGravity(AFortCreativeMoveTool* Tool, bool bAllow)
{
	Tool->bAllowGravityOnPlace = bAllow;
	Tool->OnRep_AllowGravityOnPlace();
}

void FortCreativeMoveTool::ComputeSelectionSetTransformAndBounds(AFortCreativeMoveTool* Tool, FTransform& OutTransform, FBox& OutBounds)
{
	if (!Tool->SelectedActors.Num()) return;

	auto SelectedActor = Tool->SelectedActors[0].Actor;

	if (SelectedActor)
	{
		OutTransform = SelectedActor->GetTransform();
	}
}

void FortCreativeMoveTool::ServerStartInteracting(AFortCreativeMoveTool* Tool, TArray<AActor*>& Actors, FTransform DragStart)
{
	Tool->SelectedActors.Free();
	Tool->NewlyPlacedActors.Free();

	if (!Tool->ActiveMovementMode) {
		Tool->ActiveMovementMode = Tool->InteractionBehaviors[0];
	}

	for (auto& Actor : Actors)
	{
		FCreativeSelectedActorInfo SelectedActorInfo{};
		SelectedActorInfo.Actor = Actor;
		SelectedActorInfo.ActorToSelectionAtDragStart = DragStart;

		Tool->SelectedActors.Add(SelectedActorInfo);
	}

	FTransform Transform;
	FBox Bounds;

	ComputeSelectionSetTransformAndBounds(Tool, Transform, Bounds);
	Tool->ClientStartInteracting(Tool->ActiveMovementMode, Tool->SelectedActors, Transform, Bounds);

	Tool->ActiveMovementMode->StartCreativeInteractionOnServer();
}

void FortCreativeMoveTool::ServerDuplicateStartInteracting(UObject* Context, FFrame& Stack)
{
	TArray<AActor*> Actors;
	FTransform DragStart;

	Stack.StepCompiledIn(&Actors);
	Stack.StepCompiledIn(&DragStart);
	Stack.IncrementCode();

	AFortCreativeMoveTool* Tool = (AFortCreativeMoveTool*)Context;
	TArray<AActor*> InteractionActors;

	for (auto& Actor : Actors)
	{
		if (!Actor) return;

		if (Actor->IsA<ABuildingActor>())
		{
			auto SpawnedActor = Runtime::SpawnActor<ABuildingActor>(Actor->GetTransform(), Actor->Class);

			if (!SpawnedActor) return;

			SpawnedActor->InitializeKismetSpawnedBuildingActor(SpawnedActor, NULL, false);

			InteractionActors.Add(SpawnedActor);
		}
	}

	ServerStartInteracting(Tool, InteractionActors, DragStart);
}

void FortCreativeMoveTool::ServerSpawnActorWithTransform(UObject* Context, FFrame& Stack)
{
	AActor* ActorToSpawn;
	FTransform TargetTransform;
	bool bAllowOverlap;
	bool bAllowGravity;
	bool bIgnoreStructuralIssues;
	bool bForPreviewing;

	Stack.StepCompiledIn(&ActorToSpawn);
	Stack.StepCompiledIn(&TargetTransform);
	Stack.StepCompiledIn(&bAllowOverlap);
	Stack.StepCompiledIn(&bAllowGravity);
	Stack.StepCompiledIn(&bIgnoreStructuralIssues);
	Stack.StepCompiledIn(&bForPreviewing);
	Stack.IncrementCode();

	AFortCreativeMoveTool* Tool = (AFortCreativeMoveTool*)Context;
	for (auto& NewlyPlacedActor : Tool->NewlyPlacedActors)
	{
		if (NewlyPlacedActor.OriginalActor == ActorToSpawn) return;
	}

	if (ActorToSpawn->IsA<ABuildingActor>()) {
		auto SpawnedActor = Runtime::SpawnActor<ABuildingActor>(TargetTransform, ActorToSpawn->Class);
		Tool->bClientNeedsToProcessNewlyPlacedActors = true;
		Tool->NewlyPlacedActors.Add(FOriginalAndSpawnedPair{ ActorToSpawn, SpawnedActor });
		Tool->OnRep_NewlyPlacedActors();
	}
}

void FortCreativeMoveTool::Patch()
{
	Runtime::Exec("/Script/FortniteGame.FortCreativeMoveTool.ServerDuplicateStartInteracting", ServerDuplicateStartInteracting);
	Runtime::Exec("/Script/FortniteGame.FortCreativeMoveTool.ServerSetAllowGravity", ServerSetAllowGravity);
	Runtime::Exec("/Script/FortniteGame.FortCreativeMoveTool.ServerSpawnActorWithTransform", ServerSpawnActorWithTransform);
}
