// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/MassEntity/Public/MassProcessor.h"
#include "EeTransformProcessor.generated.h"

/**
 * 
 */
UCLASS()
class EE_API UTransformProcessor : public UMassProcessor
{
	GENERATED_BODY()

	public:
		UTransformProcessor();

	protected:
		virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
		virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	private:
		FMassEntityQuery EntityQuery;
};

