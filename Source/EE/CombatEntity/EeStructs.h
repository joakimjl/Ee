// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityHandle.h"

#include "EeStructs.generated.h"

USTRUCT(Blueprintable)
struct FEeTargetData
{
	GENERATED_BODY()

	FEeTargetData() = default;
	FEeTargetData(int32 Index, int32 SerialNumber, const FTransform& Transform);

	UPROPERTY()
	int32 EntityNumber = 0;
	
	UPROPERTY()
	int32 EntitySerial = 0;
	
	UPROPERTY()
	FTransform TargetTransform;
};
