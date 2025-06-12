// Fill out your copyright notice in the Description page of Project Settings.

#include "EeStateTreeConditions.h"
#include "StateTreeExecutionContext.h"

bool FEeLocationValidCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	// Check if the location is valid (not zero vector)
	if (InstanceData.TargetLocation.EndOfPathPosition)
	{
		return false;
	}
	
	return true;
}
