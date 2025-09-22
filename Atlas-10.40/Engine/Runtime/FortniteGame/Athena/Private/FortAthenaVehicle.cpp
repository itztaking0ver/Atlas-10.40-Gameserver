#include "../Public/FortAthenaVehicle.h"

void FortAthenaVehicle::SpawnVehicles()
{
	auto Spawners = Runtime::GetAll<AFortAthenaVehicleSpawner>();
	xmap<UClass*, int> EvaledVehicles;
	xmap<UClass*, int> VehicleCounts;
	xmap<UClass*, TArray<AFortAthenaVehicleSpawner*>> Vehicles;
	EEvaluateCurveTableResult idk{};

	for (auto& Spawner : Spawners)
	{
		auto VehicleClass = Spawner->GetVehicleClass();
		if (VehicleCounts[VehicleClass]) VehicleCounts[VehicleClass]++;
		else VehicleCounts[VehicleClass] = 1;
	}

	for (auto& Spawner : Spawners)
	{
		auto VehicleClass = Spawner->GetVehicleClass();
		if (!EvaledVehicles[VehicleClass]) {
			auto ClassDetails = ((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->MapInfo->VehicleClassDetails.Search([&](FVehicleClassDetails& Spawner) {
				return VehicleClass->DerivativeOf(Spawner.VehicleClass);
				});

			float Min, Max;
			UDataTableFunctionLibrary::EvaluateCurveTableRow(ClassDetails->VehicleMinSpawnPercent.Curve.CurveTable, ClassDetails->VehicleMinSpawnPercent.Curve.RowName, (float)0, &idk, &Min, FString());
			UDataTableFunctionLibrary::EvaluateCurveTableRow(ClassDetails->VehicleMaxSpawnPercent.Curve.CurveTable, ClassDetails->VehicleMaxSpawnPercent.Curve.RowName, (float)0, &idk, &Max, FString());

			auto NumToWipe = (int)(Max - Min) == 0 ? 0 : VehicleCounts[VehicleClass] * (rand() % (int)(Max - Min));
			NumToWipe += VehicleCounts[VehicleClass] * (100 - (int)Max) / 100;
			EvaledVehicles[VehicleClass] = NumToWipe;
		}

		if (EvaledVehicles[VehicleClass] < VehicleCounts[VehicleClass]) Vehicles[VehicleClass].Add(Spawner);
	}

	for (auto& [VehicleClass, VehicleArray] : Vehicles) {
		for (int i = 0; i < EvaledVehicles[VehicleClass]; i++) {
			auto x = rand() % VehicleCounts[VehicleClass];
			VehicleArray.Remove(x);
		}
	}

	for (auto& [VehicleClass, VehicleArray] : Vehicles) {
		for (auto& Vehicle : VehicleArray) {
			Runtime::SpawnActor<AFortAthenaVehicle>(Vehicle->K2_GetActorLocation(), Vehicle->K2_GetActorRotation(), VehicleClass);
			Vehicle->K2_DestroyActor();
		}
	}
	Spawners.Free();
}

void FortAthenaVehicle::ServerMove(UObject* Context, FFrame& Stack) {
	FReplicatedPhysicsPawnState State;
	Stack.StepCompiledIn(&State);
	Stack.IncrementCode();
	auto Pawn = (AFortPhysicsPawn*)Context;

	USkeletalMeshComponent* Mesh = (USkeletalMeshComponent*)Pawn->RootComponent;

	State.Rotation.X -= 2.5f;
	State.Rotation.Y /= 0.3f;
	State.Rotation.Z -= -2.0f;
	State.Rotation.W /= -1.2f;

	auto Rotation = State.Rotation.Rotator();
	Pawn->ReplicatedMovement.AngularVelocity = State.AngularVelocity;
	Pawn->ReplicatedMovement.LinearVelocity = State.LinearVelocity;
	Pawn->ReplicatedMovement.Location = State.Translation;
	Pawn->ReplicatedMovement.Rotation = Rotation;
	Pawn->OnRep_ReplicatedMovement();

	Mesh->SetAllPhysicsLinearVelocity(State.LinearVelocity, false);
	Mesh->SetAllPhysicsAngularVelocityInRadians(State.AngularVelocity, false);
	Mesh->K2_SetWorldLocationAndRotation(State.Translation, Rotation, false, nullptr, true);
}

void FortAthenaVehicle::Patch()
{
	Runtime::Exec("/Script/FortniteGame.FortPhysicsPawn.ServerMove", ServerMove);
}
