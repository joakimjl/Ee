// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "EeResources.generated.h"


struct EeResources
{
	
};

/*
 *
 * Resource Enum to indicate if Wood, Rock, Sun fragment...
 **/
UENUM(BlueprintType, meta = (Bitflags), DisplayName = "Resource")
enum class EeResource : uint8
{
	Wood,
	Rock,
	Sun
};