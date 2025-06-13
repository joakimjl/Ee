// Fill out your copyright notice in the Description page of Project Settings.

#include "EeStateTreeConditions.h"
#include "StateTreeExecutionContext.h"

#include "Engine/World.h"
#include "StateTreeLinker.h"
#include "MassCommonFragments.h"
#include "EE/CombatEntity/EeSubsystem.h"


bool FEeLocationValidCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	// Check if the location is valid (not zero vector)
	if (!InstanceData.TargetLocation.EndOfPathPosition.IsSet() || InstanceData.TargetLocation.EndOfPathPosition.GetValue().IsNearlyZero())
	{
		return false;
	}
	
	return true;
}


bool FEeOutsideRadius::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EntityTransformHandle);
	return true;
}

bool FEeOutsideRadius::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);

	//if (!InstanceData.TargetLocation.EndOfPathPosition.IsSet()) return true;
	FVector SelfToTarget = TransformFragment.GetTransform().GetLocation() - InstanceData.TargetLocation.EndOfPathPosition.GetValue();

	float DistanceToTarget = SelfToTarget.Size();

	if (bInvert)
	{
		//IE is inside radius
		if (DistanceToTarget < InstanceData.Radius)
		{
			return true;
		}
		return false;
	}
	if (DistanceToTarget >= InstanceData.Radius)
	{
		return true;
	}
	return false;
}

bool FEeTargetValid::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	bool IsValid = Context.GetWorld()->GetSubsystem<UEeSubsystem>()->EntityIsValid(InstanceData.TargetData)^bInvert;

	return IsValid;
}

bool FEeNavMeshDone::TestCondition(FStateTreeExecutionContext& Context) const
{
	return Context.GetWorld()->IsNavigationRebuilt() ^bInvert;
}

