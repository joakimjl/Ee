// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassNavigationTypes.h"
#include "MassNavMeshNavigationFragments.h"
#include "MassStateTreeExecutionContext.h"
#include "MassStateTreeTypes.h"

#include "EeNavigationTasks.generated.h"


struct FTransformFragment;

namespace UE::MassBehavior
{
	struct FStateTreeDependencyBuilder;
};

/**
 * Tasks to claim a smart object from search results and release it when done.
 */
USTRUCT()
struct FEeFindRandomLocationInstanceData
{
	GENERATED_BODY()

	/** Maximum distance away (Input) */
	UPROPERTY(VisibleAnywhere, Category = Input)
	float RadiusToCheck;

	/** Random Location in Radius (Input) */
	UPROPERTY(VisibleAnywhere, Category = Output)
	FMassTargetLocation WalkToLocation;

	/** Found a Location (Input) */
	UPROPERTY(VisibleAnywhere, Category = Output)
	bool FoundLocation = false;
};

USTRUCT(meta = (DisplayName = "Find Random Location in Radius"))
struct FEeFindRandomLocationInRadius : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEeFindRandomLocationInstanceData;

	FEeFindRandomLocationInRadius();

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
	
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;

	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;

	/** Retry cooldown until checking on ineligible location found */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float RetryCooldown = 1.f;
};
