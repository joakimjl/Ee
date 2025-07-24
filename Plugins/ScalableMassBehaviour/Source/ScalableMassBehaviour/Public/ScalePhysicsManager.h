// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "ScalePhysicsManager.generated.h"

UCLASS()
class AScalePhysicsManager : public AActor
{
	GENERATED_BODY()

public:
	AScalePhysicsManager();

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	bool RegisteredSelf = false;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Smb")
	UNiagaraComponent* NiagaraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Smb")
	TObjectPtr<UStaticMesh> StaticMeshParticle = TObjectPtr<UStaticMesh>();

	UPROPERTY(BlueprintReadWrite, Category = "Smb")
	int32 TotalDead = 0;

	UFUNCTION(BlueprintCallable, Category = "Smb")
	void AddPhysicsParticles(TArray<FVector> InVectors, int32 SpawnCount);
};