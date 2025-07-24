// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FScalableMassBehaviourModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

UENUM(BlueprintType)
enum class EProcessable : uint8
{
	Gold,
	Food,
	Grass,
	Wood,
	Stone,
	Water,
	Storage,
	None
};