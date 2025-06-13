// Fill out your copyright notice in the Description page of Project Settings.

#include "EeStateTreeConditions.h"
#include "StateTreeExecutionContext.h"

#include "Engine/World.h"
#include "StateTreeLinker.h"
#include "MassCommonFragments.h"


bool FEeLocationValidCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	// Check if the location is valid (not zero vector)
	if (!InstanceData.TargetLocation.EndOfPathPosition)
	{
		return false;
	}
	
	return true;
}


bool FEeDistanceToMassLocation::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EntityTransformHandle);
	return true;
}

bool FEeDistanceToMassLocation::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);

	if (!InstanceData.TargetLocation.EndOfPathPosition.IsSet()) return true;
	FVector SelfToTarget = TransformFragment.GetTransform().GetLocation() - InstanceData.TargetLocation.EndOfPathPosition.GetValue();
	
	if (SelfToTarget.Size() >= InstanceData.Distance)
	{
		return false;
	}
	
	return true;
}

