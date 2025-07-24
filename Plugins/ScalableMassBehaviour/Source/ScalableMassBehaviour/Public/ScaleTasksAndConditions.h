// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassNavigationTypes.h"
#include "MassStateTreeTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "StateTreeConditionBase.h"
#include "ScalableMassBehaviour.h"
#include "ScaleSubsystem.h"

#include "ScaleTasksAndConditions.generated.h"


struct FMassRepresentationFragment;
class UMassCrowdRepresentationSubsystem;
struct FResourceFragment;
class UMassSignalSubsystem;
class UScaleSubsystem;

/*
 * Possible options for Instance Data:
 * UENUM()
 * enum class EStateTreePropertyUsage : uint8
 * {
 * 	Invalid,
 * 	Context,
 * 	Input,
 * 	Parameter,
 * 	Output,
 * };
 */

struct FMassMoveTargetFragment;

USTRUCT()
struct FGetRandomLocationInRangeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Input)
	FVector InLocation;
	UPROPERTY(EditAnywhere, Category = Input)
	float Radius;
	UPROPERTY(EditAnywhere, Category = Output)
	FMassTargetLocation TargetLocation;
};

USTRUCT(meta = (DisplayName = "SMB Get Random Location Near InVector"))
struct FGetRandomLocationInRange : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FGetRandomLocationInRangeInstanceData;

	FGetRandomLocationInRange();

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FGetRandomLocationInRangeInstanceData::StaticStruct(); };
	
	TStateTreeExternalDataHandle<FAnimationFragment> AnimationFragmentHandle;
	TStateTreeExternalDataHandle<FDeathPhysicsFragment> DeathFragmentHandle;
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
};


USTRUCT()
struct FGetProcessableLocationInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Input)
	EProcessable ResourceType = EProcessable::Grass;
	UPROPERTY(EditAnywhere, Category = Input)
	float Radius = 50.f;
	UPROPERTY(EditAnywhere, Category = Output)
	FMassTargetLocation TargetLocation = FMassTargetLocation();
};

USTRUCT(meta = (DisplayName = "SMB Get Processable of Type"))
struct FGetProcessableLocation : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FGetProcessableLocationInstanceData;

	FGetProcessableLocation();

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FGetProcessableLocationInstanceData::StaticStruct(); };
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
	
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	TStateTreeExternalDataHandle<UScaleSubsystem> ScaleSubsystemHandle;
};


UENUM(BlueprintType)
enum class ETaskType : uint8
{
	Pickup,
	Drop,
	Process
};

USTRUCT()
struct FProcessResourceInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Input)
	EProcessable ResourceType = EProcessable::Grass;
	UPROPERTY(EditAnywhere, Category = Parameter)
	float ProcessDuration = 1.f;
	UPROPERTY(EditAnywhere, Category = Parameter)
	ETaskType TaskType = ETaskType::Pickup;
	UPROPERTY(EditAnywhere, Category = Output)
	FMassTargetLocation TargetLocation = FMassTargetLocation();
};

USTRUCT(meta = (DisplayName = "SMB Process Resource Task"))
struct FProcessResource : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FProcessResourceInstanceData;

	FProcessResource();

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FProcessResourceInstanceData::StaticStruct(); };
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
	
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	TStateTreeExternalDataHandle<FMassVelocityFragment> VelocityFragmentHandle;
	TStateTreeExternalDataHandle<FMassDesiredMovementFragment> DesiredFragmentHandle;
	TStateTreeExternalDataHandle<FResourceFragment> ResourceFragmentHandle;
	TStateTreeExternalDataHandle<UScaleSubsystem> ScaleSubsystemHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};


USTRUCT()
struct FNavDoneConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Output)
	bool IsDone;
};

USTRUCT(meta = (DisplayName = "SMB Is Nav Done"))
struct FNavDoneCondition : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FNavDoneConditionInstanceData;

	UPROPERTY(EditAnywhere, Category = Condition)
	bool bInvert = false;
	
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};


USTRUCT()
struct FIsCloseEnoughConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Input)
	FVector InLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = Input)
	FMassTargetLocation InMassLocation = FMassTargetLocation();
	UPROPERTY(EditAnywhere, Category = Input)
	float AcceptableDistance = 50.f;

	UPROPERTY(EditAnywhere, Category = Output)
	bool ConditionResult;
};

USTRUCT(meta = (DisplayName = "SMB Close Enough to Target"))
struct FIsCloseEnoughCondition : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FIsCloseEnoughConditionInstanceData;

	UPROPERTY(EditAnywhere, Category = Condition)
	bool bInvert = false;
	UPROPERTY(EditAnywhere, Category = Condition)
	bool bUseMassTargetLocation = false;
	
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	virtual bool Link(FStateTreeLinker& Linker) override;
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
};



USTRUCT()
struct FFindClosestEnemyInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Input)
	float MaxDistance = 1000.f;
	
	UPROPERTY(EditAnywhere, Category = Output)
	FSmbEntityData EntityTarget;
	UPROPERTY(EditAnywhere, Category = Output)
	bool FoundEntityTarget;
};

USTRUCT(meta = (DisplayName = "SMB Find Closest Enemy"))
struct FFindClosestEnemy : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FFindClosestEnemyInstanceData;

	FFindClosestEnemy();

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FFindClosestEnemyInstanceData::StaticStruct(); };
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;

	UPROPERTY(EditAnywhere, Category = "Smb")
	bool bTickingFind = false;
	UPROPERTY(EditAnywhere, Category = "Smb")
	float TickDelay = 1.f;
	
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	TStateTreeExternalDataHandle<FAttackFragment> AttackFragmentHandle;
	TStateTreeExternalDataHandle<FTeamFragment> TeamFragmentHandle;
	TStateTreeExternalDataHandle<UScaleSubsystem> ScaleSubsystemHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};

USTRUCT()
struct FProcessAttackInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Input)
	FSmbEntityData EntityTarget;
};

USTRUCT(meta = (DisplayName = "SMB Attack Enemy"))
struct FProcessAttack : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FProcessAttackInstanceData;

	FProcessAttack();

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FProcessAttackInstanceData::StaticStruct(); };
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
	
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	TStateTreeExternalDataHandle<FAttackFragment> AttackFragmentHandle;
	TStateTreeExternalDataHandle<FAnimationFragment> AnimationFragmentHandle;
	TStateTreeExternalDataHandle<FMassDesiredMovementFragment> DesiredMovementHandle;
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FMassMovementParameters> MovementParamHandle;
	TStateTreeExternalDataHandle<UScaleSubsystem> ScaleSubsystemHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};



USTRUCT()
struct FWalkToEntityInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Input)
	FSmbEntityData EntityTarget;

	UPROPERTY(EditAnywhere, Category = Output)
	FMassTargetLocation LocationTarget;
};

USTRUCT(meta = (DisplayName = "SMB Get Entity Location"))
struct FWalkToEntity : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FWalkToEntityInstanceData;

	FWalkToEntity();

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FWalkToEntityInstanceData::StaticStruct(); };
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
	
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	TStateTreeExternalDataHandle<FAttackFragment> AttackFragmentHandle;
	TStateTreeExternalDataHandle<FAnimationFragment> AnimationFragmentHandle;
	TStateTreeExternalDataHandle<UScaleSubsystem> ScaleSubsystemHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};




USTRUCT()
struct FCheckOwnHealthInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Output)
	float EntityHealth;
};

USTRUCT(meta = (DisplayName = "SMB Check own health"))
struct FCheckOwnHealth : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCheckOwnHealthInstanceData;

	FCheckOwnHealth();

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FCheckOwnHealthInstanceData::StaticStruct(); };
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	TStateTreeExternalDataHandle<FDefenceFragment> DefenceFragmentHandle;
	TStateTreeExternalDataHandle<FAnimationFragment> AnimationFragmentHandle;
	TStateTreeExternalDataHandle<FDeathPhysicsFragment> DeathPhysicsHandle;
	TStateTreeExternalDataHandle<FMassRepresentationFragment> MassRepFragmentHandle;
	TStateTreeExternalDataHandle<UMassCrowdRepresentationSubsystem> MassCrowdRepHandle;
	TStateTreeExternalDataHandle<UScaleSubsystem> ScaleSubsystemHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};