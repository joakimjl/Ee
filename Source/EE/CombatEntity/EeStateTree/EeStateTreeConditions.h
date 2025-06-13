
#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "MassCommonFragments.h"
#include "MassNavigationTypes.h"
#include "MassStateTreeDependency.h"
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
struct FEeDistanceToMassLocationInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Input)
	FMassTargetLocation TargetLocation;

	UPROPERTY(EditAnywhere, Category = Input)
	float CheckedDistance;

	UPROPERTY(EditAnywhere, Category = Output)
	float Distance;
};

USTRUCT(meta = (DisplayName = "Distance To MassLocation"))
struct FEeDistanceToMassLocation : public FStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEeDistanceToMassLocationInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	virtual bool Link(FStateTreeLinker& Linker) override;
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	//virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;
};
