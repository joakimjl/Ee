// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatFragments.h"
#include "EeStructs.h"
#include "MassNavigationTypes.h"
#include "MassNavMeshNavigationFragments.h"
#include "MassStateTreeExecutionContext.h"
#include "MassStateTreeTypes.h"
#include "MassEQS/Public/MassEQSTypes.h"

#include "EeNavigationTasks.generated.h"


class UEeSubsystem;
struct FDefenceStatsBase;
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

USTRUCT(meta = (DisplayName = "Ee Find Random Location in Radius"))
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



USTRUCT()
struct FEeCheckHealthInstanceData
{
	GENERATED_BODY()

	/** Health Amount of Unit */
	UPROPERTY(VisibleAnywhere, Category = Output)
	float Health = -1.f;
};

USTRUCT(meta = (DisplayName = "Ee Check Health of Unit"))
struct FEeCheckHealth : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEeCheckHealthInstanceData;

	FEeCheckHealth();

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
	
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;

	TStateTreeExternalDataHandle<FDefenceStatsBase> DefenceStatsHandle;
};





USTRUCT()
struct FEeCheckForEnemiesInstanceData
{
	GENERATED_BODY()

	/** Found Closest Enemy */
	UPROPERTY(VisibleAnywhere, Category = Output)
	FEeTargetData TargetData;
};

USTRUCT(meta = (DisplayName = "Ee Check for Enemies"))
struct FEeCheckForEnemies : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEeCheckForEnemiesInstanceData;

	FEeCheckForEnemies();

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
	
	

	TStateTreeExternalDataHandle<FTransformFragment> FTransformFragmentHandle;
	TStateTreeExternalDataHandle<FTeamFragment> FTeamHandle;
};




USTRUCT()
struct FEeEntityToMassLocationInstanceData
{
	GENERATED_BODY()

	/** Enemy to walk towards */
	UPROPERTY(VisibleAnywhere, Category = Input)
	FEeTargetData TargetData;

	/** Enemy's Location */
	UPROPERTY(VisibleAnywhere, Category = Output)
	FMassTargetLocation TargetLocation;
};

USTRUCT(meta = (DisplayName = "Ee Finds Location of Entity"))
struct FEeEntityToMassLocation : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEeEntityToMassLocationInstanceData;

	FEeEntityToMassLocation();

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
	
	

	TStateTreeExternalDataHandle<FTransformFragment> FTransformFragmentHandle;
	TStateTreeExternalDataHandle<FTeamFragment> FTeamHandle;
};




USTRUCT()
struct FEeAttackTowardsEntityInstanceData
{
	GENERATED_BODY()

	/** Enemy Target */
	UPROPERTY(VisibleAnywhere, Category = Input)
	FEeTargetData TargetData;
};

USTRUCT(meta = (DisplayName = "Ee Attack Towards Entity"))
struct FEeAttackTowardsEntity : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEeAttackTowardsEntityInstanceData;

	FEeAttackTowardsEntity();

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
	
	

	TStateTreeExternalDataHandle<FTransformFragment> FTransformFragmentHandle;
	TStateTreeExternalDataHandle<FTeamFragment> FTeamHandle;
	TStateTreeExternalDataHandle<FOffensiveStatsBase> FOffensiveStatsBaseHandle;
	TStateTreeExternalDataHandle<FOffensiveStatsParams> FOffensiveStatsParamsHandle;
};

