// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "EeObservers.generated.h"

/**
 * Processor that observes when ProjectileFragment is added to entities
 */
UCLASS()
class UProjectileObserverProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UProjectileObserverProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

UCLASS()
class UDamageObserverProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UDamageObserverProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

