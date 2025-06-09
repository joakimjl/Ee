// Fill out your copyright notice in the Description page of Project Settings.


#include "EeNavigationTasks.h"

#include "CombatFragments.h"
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