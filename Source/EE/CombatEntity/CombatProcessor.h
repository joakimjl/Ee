// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/MassEntity/Public/MassProcessor.h"
#include "CombatProcessor.generated.h"

/**
 * 
 */
UCLASS()
class EE_API UCombatProcessor : public UMassProcessor
{
	GENERATED_BODY()

	public:
		UCombatProcessor();

	protected:
		virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
		virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	private:
		FMassEntityQuery EntityQuery;
};

