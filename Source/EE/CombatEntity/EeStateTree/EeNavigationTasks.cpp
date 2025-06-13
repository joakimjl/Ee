// Fill out your copyright notice in the Description page of Project Settings.


#include "EeNavigationTasks.h"

#include "../EeCombatFragments.h"
#include "../EeSubsystem.h"
#include "Engine/World.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassStateTreeDependency.h"
#include "MassStateTreeTypes.h"
#include "MassCommonFragments.h"
#include "MassSignalSubsystem.h"

#include "MassAIBehaviorTypes.h"
#include "AI/NavigationSystemBase.h"

#define EE_SPAWNABLE  ECC_GameTraceChannel2

FEeFindRandomLocationInRadius::FEeFindRandomLocationInRadius()
{
	// This task should not react to Enter/ExitState when the state is reselected.
	bShouldStateChangeOnReselect = false;
}

bool FEeFindRandomLocationInRadius::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(EntityTransformHandle);
	return true;
}

void FEeFindRandomLocationInRadius::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite(MassSignalSubsystemHandle);
	Builder.AddReadOnly(EntityTransformHandle);
}

EStateTreeRunStatus FEeFindRandomLocationInRadius::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);

	const UWorld* World = Context.GetWorld();
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const float Radius = InstanceData.RadiusToCheck;
	const FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);

	
	FMassTargetLocation WalkToLocation;
	FVector TargetLocation = TransformFragment.GetTransform().GetLocation() + FVector(FMath::RandRange(-Radius, Radius), FMath::RandRange(-Radius, Radius), 0);
	FHitResult OutHit = FHitResult();
	if (!World->IsNavigationRebuilt()) return EStateTreeRunStatus::Running;
	bool Res = World->LineTraceSingleByChannel(OutHit,TargetLocation + FVector::UpVector*1000.f, TargetLocation + FVector::UpVector*-1000.f, EE_SPAWNABLE);
	if (!Res) return EStateTreeRunStatus::Running;
	
	WalkToLocation.EndOfPathPosition = OutHit.Location;
	WalkToLocation.EndOfPathIntent = EMassMovementAction::Stand;
	InstanceData.WalkToLocation = WalkToLocation;
	InstanceData.FoundLocation = true;
	
	return EStateTreeRunStatus::Succeeded;

	//return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FEeFindRandomLocationInRadius::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const UWorld* World = Context.GetWorld();
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const float Radius = InstanceData.RadiusToCheck;
	const FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);

	
	FMassTargetLocation WalkToLocation;
	FVector TargetLocation = TransformFragment.GetTransform().GetLocation() + FVector(FMath::RandRange(-Radius, Radius), FMath::RandRange(-Radius, Radius), 0);
	FHitResult OutHit = FHitResult();

	if (!World->IsNavigationRebuilt()) return EStateTreeRunStatus::Running;
	bool Res = World->LineTraceSingleByChannel(OutHit,TargetLocation + FVector::UpVector*1000.f, TargetLocation + FVector::UpVector*-1000.f, ECC_GameTraceChannel2);
	if (!Res) return EStateTreeRunStatus::Running;
	
	WalkToLocation.EndOfPathPosition = OutHit.Location;
	WalkToLocation.EndOfPathIntent = EMassMovementAction::Stand;
	InstanceData.WalkToLocation = WalkToLocation;
	InstanceData.FoundLocation = true;
	
	return EStateTreeRunStatus::Succeeded;
}

void FEeFindRandomLocationInRadius::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);

	//MASSBEHAVIOR_LOG(Verbose, TEXT("Cancelling pending SmartObject search on ExitState."));

}




FEeCheckHealth::FEeCheckHealth()
{
	// This task should not react to Enter/ExitState when the state is reselected.
	bShouldStateChangeOnReselect = false;
}

bool FEeCheckHealth::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(DefenceStatsHandle);
	return true;
}

void FEeCheckHealth::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite(MassSignalSubsystemHandle);
	Builder.AddReadOnly(DefenceStatsHandle);
}

EStateTreeRunStatus FEeCheckHealth::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FDefenceStatsBase& DefenceStats = Context.GetExternalData(DefenceStatsHandle);
	if (DefenceStats.CurHealth <= -1) return EStateTreeRunStatus::Failed;

	InstanceData.Health = DefenceStats.CurHealth;
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FEeCheckHealth::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FDefenceStatsBase& DefenceStats = Context.GetExternalData(DefenceStatsHandle);
	if (DefenceStats.CurHealth <= -1) return EStateTreeRunStatus::Failed;

	InstanceData.Health = DefenceStats.CurHealth;

	//UE_LOG(LogTemp, Display, TEXT("Found Health %f"), DefenceStats.CurHealth);
	
	return EStateTreeRunStatus::Running;
}

void FEeCheckHealth::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	
}


FEeCheckForEnemies::FEeCheckForEnemies()
{
	// This task should not react to Enter/ExitState when the state is reselected.
	bShouldStateChangeOnReselect = false;
}

bool FEeCheckForEnemies::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(FTransformFragmentHandle);
	Linker.LinkExternalData(FTeamHandle);
	Linker.LinkExternalData(EeSubsystemHandle);
	return true;
}

void FEeCheckForEnemies::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadOnly(FTransformFragmentHandle);
	Builder.AddReadOnly(FTeamHandle);
	Builder.AddReadOnly(EeSubsystemHandle);
}

EStateTreeRunStatus FEeCheckForEnemies::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UEeSubsystem& EeSubsystem = Context.GetExternalData(EeSubsystemHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(FTransformFragmentHandle);
	const FTeamFragment& TeamFragment = Context.GetExternalData(FTeamHandle);
	FIntVector2 GridLoc = EeSubsystem.VectorToGrid(TransformFragment.GetTransform().GetLocation());
	TArray<FMassEntityHandle> Enemies = EeSubsystem.EnemiesAround(GridLoc, 8, TeamFragment.Team);

	//UE_LOG(LogTemp, Display, TEXT("Found: %i enemies"), Enemies.Num())

	if (Enemies.Num() == 0) return EStateTreeRunStatus::Running;
	//TODO fix correct transform (not self) and closest enemy target with optimisation in EeSubsystem
	//UE_LOG(LogTemp, Display, TEXT("Found Number: %llu with serial: %i enemy at 0 for: %i around was: %i"), Enemies[0].AsNumber(), Enemies[0].SerialNumber, TeamFragment.Team, Enemies.Num());
	InstanceData.TargetData = FEeTargetData(Enemies[0].AsNumber(),Enemies[0].SerialNumber,TransformFragment.GetMutableTransform());
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FEeCheckForEnemies::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UEeSubsystem& EeSubsystem = Context.GetExternalData(EeSubsystemHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(FTransformFragmentHandle);
	const FTeamFragment& TeamFragment = Context.GetExternalData(FTeamHandle);
	FIntVector2 GridLoc = EeSubsystem.VectorToGrid(TransformFragment.GetTransform().GetLocation());
	TArray<FMassEntityHandle> Enemies = EeSubsystem.EnemiesAround(GridLoc, 8, TeamFragment.Team);

	//UE_LOG(LogTemp, Display, TEXT("Found: %i enemies in tick"), Enemies.Num())

	if (Enemies.Num() == 0) return EStateTreeRunStatus::Running;
	//TODO fix correct transform (not self) and closest enemy target with optimisation in EeSubsystem
	//UE_LOG(LogTemp, Display, TEXT("Found Number: %llu with serial: %i enemy at 0 for: %i around was: %i"), Enemies[0].AsNumber(), Enemies[0].SerialNumber, TeamFragment.Team, Enemies.Num());
	InstanceData.TargetData = FEeTargetData(Enemies[0].AsNumber(),Enemies[0].SerialNumber,TransformFragment.GetMutableTransform());
	
	return EStateTreeRunStatus::Running;
}

void FEeCheckForEnemies::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	
}




FEeEntityToMassLocation::FEeEntityToMassLocation()
{
	bShouldStateChangeOnReselect = true;
	bShouldCallTick = false;
}

bool FEeEntityToMassLocation::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(FTransformFragmentHandle);
	Linker.LinkExternalData(FTeamHandle);
	Linker.LinkExternalData(EeSubsystemHandle);
	return true;
}

void FEeEntityToMassLocation::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadOnly(FTransformFragmentHandle);
	Builder.AddReadOnly(FTeamHandle);
	Builder.AddReadOnly(EeSubsystemHandle);
}

EStateTreeRunStatus FEeEntityToMassLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UEeSubsystem& EeSubsystem = Context.GetExternalData(EeSubsystemHandle);

	if (!EeSubsystem.EntityIsValid(InstanceData.TargetData)) return EStateTreeRunStatus::Failed;

	FVector TargetLocation = EeSubsystem.GetEntityLocation(InstanceData.TargetData).GetLocation();

	FMassTargetLocation WalkToLocation;
	WalkToLocation.EndOfPathPosition = TargetLocation;
	WalkToLocation.EndOfPathIntent = EMassMovementAction::Stand;
	InstanceData.TargetLocation = WalkToLocation;

	//Verbose Log FoundLocation (Visual logger)
	MASSBEHAVIOR_LOG(Verbose, TEXT("Found Location: %s"), *TargetLocation.ToString());
	
	return EStateTreeRunStatus::Succeeded;
}

void FEeEntityToMassLocation::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	
}




FEeAttackTowardsEntity::FEeAttackTowardsEntity()
{
	bShouldStateChangeOnReselect = true;
	bShouldCallTick = false;
}

bool FEeAttackTowardsEntity::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(FTransformFragmentHandle);
	Linker.LinkExternalData(FOffensiveStatsBaseHandle);
	Linker.LinkExternalData(FOffensiveStatsParamsHandle);
	Linker.LinkExternalData(FTeamHandle);
	Linker.LinkExternalData(EeSubsystemHandle);
	return true;
}

void FEeAttackTowardsEntity::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadOnly(FTransformFragmentHandle);
	Builder.AddReadOnly(FOffensiveStatsBaseHandle);
	Builder.AddReadOnly(FOffensiveStatsParamsHandle);
	Builder.AddReadOnly(FTeamHandle);
	Builder.AddReadOnly(EeSubsystemHandle);
}

EStateTreeRunStatus FEeAttackTowardsEntity::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UEeSubsystem& EeSubsystem = Context.GetExternalData(EeSubsystemHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(FTransformFragmentHandle);
	const FOffensiveStatsBase& OffensiveStats = Context.GetExternalData(FOffensiveStatsBaseHandle);
	const FOffensiveStatsParams& OffensiveStatsParams = Context.GetExternalData(FOffensiveStatsParamsHandle);
	const FTeamFragment& TeamFragment = Context.GetExternalData(FTeamHandle);
	FTransform EnemyTransform = EeSubsystem.GetEntityLocation(InstanceData.TargetData);
	if (EnemyTransform.Equals(FTransform::Identity)) return EStateTreeRunStatus::Failed;
	FVector OwnLocation = TransformFragment.GetTransform().GetLocation();

	FVector VectorToEnemy = EnemyTransform.GetLocation() - OwnLocation;
	float Range = OffensiveStatsParams.AttackRange*OffensiveStats.AttackRangeMult;
	EeSubsystem.AttackLocation(OwnLocation + VectorToEnemy.GetSafeNormal()*Range,
		OffensiveStatsParams.DamageType,
		OffensiveStatsParams.AttackDamage*OffensiveStats.AttackDamageMult,
		OffensiveStatsParams.AttackAoe*OffensiveStats.AttackAoeMult,
		TeamFragment.Team);

	//UE_LOG(LogTemp, Display, TEXT("Attacked at: %s"), *OwnLocation.ToString());
	
	return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FEeAttackTowardsEntity::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return EStateTreeRunStatus::Running;
}

void FEeAttackTowardsEntity::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	
}