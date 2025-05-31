// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EeSubsystem.generated.h"

struct FMassEntityManager;
struct FMassEntityHandle;
/**
 * 
 */
UCLASS()
class EE_API UEeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:

	//Grid Functions
	UFUNCTION()
	FVector MoveSelfInGrid(FMassEntityHandle Handle, FVector PreviousLocation);
	UFUNCTION()
	FVector AddSelfToGrid(FMassEntityHandle Handle);
	UFUNCTION(BlueprintCallable)
	FIntVector2 VectorToGrid(FVector Vec);

	
	//Spawning Projectiles Functions
	UFUNCTION(BlueprintCallable)
	bool SpawnProjectile(FMassEntityHandle Handle);

protected:
	// UWorldSubsystem begin interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// UWorldSubsystem end interface
	
private:
	UE_MT_DECLARE_RW_ACCESS_DETECTOR(AccessDetector);
	UPROPERTY()
	TMap<int32, TMap<int32, TArray<FMassEntityHandle>>> EntityHandleGrid;
	UPROPERTY()
	FMassEntityManager* EeEntityManager;
	
};

template<>
struct TMassExternalSubsystemTraits<UEeSubsystem> final
{
	enum
	{
		ThreadSafeRead = true,
		ThreadSafeWrite = false,
		GameThreadOnly = false
	};
};
