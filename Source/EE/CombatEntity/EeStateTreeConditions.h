
#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "MassCommonFragments.h"
#include "MassNavigationTypes.h"
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

protected:
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
