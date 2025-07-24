// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "ScaleProcessors.generated.h"

#define SCALE_API SCALABLEMASSBEHAVIOUR_API

UCLASS()
class UAnimationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	SCALE_API UAnimationProcessor();

protected:
	SCALE_API virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;

	SCALE_API virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;

	float TimeAccumulator = 0.f;
};

UCLASS()
class SCALABLEMASSBEHAVIOUR_API URegisterProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URegisterProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;
};


UCLASS()
class SCALABLEMASSBEHAVIOUR_API UCollisionProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UCollisionProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;
};


/*
UCLASS()
class SCALABLEMASSBEHAVIOUR_API UScaleProcessors : public UMassProcessor
{
	GENERATED_BODY()

public:
	UScaleProcessors();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;
};
*/

