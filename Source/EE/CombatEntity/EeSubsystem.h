// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTemplateRegistry.h"

#include "EeSubsystem.generated.h"

enum class EDamageType : uint8;
struct FMassEntityHandle;
struct FMassEntityManager;

USTRUCT(BlueprintType)
struct FGridCellData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FMassEntityHandle> Handles = TArray<FMassEntityHandle>();
};

USTRUCT(BlueprintType)
struct FEntityHandleGridCellY
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<int32, FGridCellData> InnerMap = TMap<int32, FGridCellData>();
};

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
	UFUNCTION()
	TArray<FMassEntityHandle> EntitesAround(FIntVector2 InGrid, int32 SizeAround);
	UFUNCTION(BlueprintCallable)
	bool AttackLocation(FVector InLocation, EDamageType DamageType, float Damage, float Area, int32 Team);
	
	//Spawning Projectiles Functions
	UFUNCTION()
	bool SpawnProjectile(FMassEntityHandle Handle, FVector TargetLocation = FVector::ZeroVector);

	UPROPERTY()
	TArray<UInstancedStaticMeshComponent*> ProjectileMeshComponents;

protected:
	// UWorldSubsystem begin interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// UWorldSubsystem end interface

	FMassEntityManager* EeEntityManager;
	UE::Mass::FEntityBuilder* EntityBuilder;
	
private:
	UE_MT_DECLARE_RW_ACCESS_DETECTOR(AccessDetector);
	UPROPERTY()
	TMap<int32, FEntityHandleGridCellY> EntityHandleGrid;

	bool DebugEnable = false;
};

template<>
struct TMassExternalSubsystemTraits<UEeSubsystem> final
{
	enum
	{
		ThreadSafeRead = true,
		ThreadSafeWrite = false,
		GameThreadOnly = true
	};
};