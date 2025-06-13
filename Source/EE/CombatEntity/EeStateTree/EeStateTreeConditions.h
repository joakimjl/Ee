
#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "MassCommonFragments.h"
#include "MassNavigationTypes.h"
#include "MassStateTreeDependency.h"
#include "EE/CombatEntity/EeStructs.h"
#include "EeStateTreeConditions.generated.h"

/**
 * Condition that checks if a target location is valid
 */
USTRUCT()
struct FEeLocationValidConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Input)
	FMassTargetLocation TargetLocation;

	UPROPERTY(EditAnywhere, Category = Output)
	bool IsValid;
};

USTRUCT(meta = (DisplayName = "Is Location Valid"))
struct FEeLocationValidCondition : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEeLocationValidConditionInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};



USTRUCT()
struct FEeOutsideRadiusInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Input)
	FMassTargetLocation TargetLocation;

	UPROPERTY(EditAnywhere, Category = Input)
	float Radius;

	UPROPERTY(EditAnywhere, Category = Output)
	float Distance;
};

USTRUCT(meta = (DisplayName = "Outside of Radius from Point"))
struct FEeOutsideRadius : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEeOutsideRadiusInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	UPROPERTY(EditAnywhere, Category = Condition)
	bool bInvert = false;

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	virtual bool Link(FStateTreeLinker& Linker) override;
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	//virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
};





USTRUCT()
struct FEeTargetValidInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Input)
	FEeTargetData TargetData;
};

USTRUCT(meta = (DisplayName = "Target Data Valid"))
struct FEeTargetValid : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEeTargetValidInstanceData;
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	UPROPERTY(EditAnywhere, Category = Condition)
	bool bInvert = false;

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};


USTRUCT()
struct FEeFEeNavMeshDoneInstanceData
{
	GENERATED_BODY()
};

USTRUCT(meta = (DisplayName = "Is NavMesh Finished"))
struct FEeNavMeshDone : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEeFEeNavMeshDoneInstanceData;
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	UPROPERTY(EditAnywhere, Category = Condition)
	bool bInvert = false;

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};