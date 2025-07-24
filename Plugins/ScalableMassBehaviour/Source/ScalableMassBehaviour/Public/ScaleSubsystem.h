// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ScalableMassBehaviour.h"
#include "MassSubsystemBase.h"
#include "ScaleFragments.h"
#include "TaskSyncManager.h"
#include "ScaleSubsystem.generated.h"


class AScalePhysicsManager;
class UMassAgentComponent;

struct FMassEntityHandle;

namespace UE::Mass
{
	struct FEntityBuilder;
}

struct FMassEntityManager;


USTRUCT(BlueprintType)
struct FSmbEntityData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Scale")
	int32 SerialNumber = -1;
	
	UPROPERTY(EditAnywhere, Category = "Scale")
	int32 Index = -1;
};

USTRUCT()
struct FProcessableReqArr
{
	GENERATED_BODY()
	
	TArray<EProcessable> TypeArr = TArray<EProcessable>();
	TArray<int32> AmountArr = TArray<int32>();
};

USTRUCT()
struct FResourceReqMap
{
	GENERATED_BODY()
	
	TMap<EProcessable, FProcessableReqArr> ReqMap = TMap<EProcessable, FProcessableReqArr>();
};

UCLASS()
class UGridCell : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FMassEntityHandle> Handles;
};

UCLASS()
class UGridCellY : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Smb")
	TMap<int32, UGridCell*> YCells;
};

UCLASS()
class UGrid : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<int32, UGridCellY*> XCells = TMap<int32, UGridCellY*>();
	UFUNCTION()
	TArray<FMassEntityHandle> GetAt(int32 X, int32 Y);
	UFUNCTION()
	TArray<FMassEntityHandle> RemoveAt(int32 X, int32 Y, FMassEntityHandle ToRemoveHandle);
	UFUNCTION()
	TArray<FMassEntityHandle> GetAround(int32 X, int32 Y, int32 Radius);

	UFUNCTION()
	void AddToGrid(int32 X, int32 Y, FMassEntityHandle Handle);
};

USTRUCT()
struct FProcessableArr
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVector> Locations = TArray<FVector>();

	UPROPERTY()
	TArray<FMassEntityHandle> Handles = TArray<FMassEntityHandle>();
};

USTRUCT()
struct FPhysicsManagerStruct
{
	GENERATED_BODY()

	FPhysicsManagerStruct() = default;
	
	UPROPERTY()
	AScalePhysicsManager* PhysicsManagerPtr = nullptr;

	UPROPERTY()
	FString MeshName = FString("none");
};

/**
 * 
 */
UCLASS()
class UScaleSubsystem : public UMassTickableSubsystemBase
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	//TStatid as needed:
	virtual TStatId GetStatId() const override;

public:
	UPROPERTY(BlueprintReadWrite, Category = "Scale")
	float TimeSinceRemoval = 0.f;
	UFUNCTION(BlueprintCallable, Category = "Scale")
	FVector2D VectorToCell(FVector Location);
	UFUNCTION(BlueprintCallable, Category = "Scale")
	bool RegisterResource(FVector Location, EProcessable Type, UMassAgentComponent* Component);
	UFUNCTION(BlueprintCallable, Category = "Scale")
	TArray<FVector> GetResources(EProcessable Type);
	UFUNCTION(Category = "Scale")
	FMassEntityHandle GetClosestResource(EProcessable Type, FVector Location);
	UFUNCTION(Category = "Scale")
	bool AddToEntity(FMassEntityHandle EntityHandle, EProcessable Type, int32 Amount) const;
	UFUNCTION(Category = "Scale")
	bool RemoveFromEntity(FMassEntityHandle EntityHandle, EProcessable Type, int32 Amount) const;
	UFUNCTION(Category = "Scale")
	FVector GetEntityLocation(FMassEntityHandle Handle);
	UFUNCTION(BlueprintCallable, Category = "Scale")
	int32 GetEntityResources(EProcessable Type, FVector Location);
	/* Deals Damage to Single Entity */
	UFUNCTION(BlueprintCallable, Category = "Scale")
	bool DealDamageToEnemy(FSmbEntityData TargetData, float DamageAmount, EDamageType DamageType);
	UFUNCTION(BlueprintCallable, Category = "Scale")
	FVector GetEntityDataLocation(FSmbEntityData TargetData);

	/* Deals Damage in an AOE */
	UFUNCTION(BlueprintCallable, Category = "Scale")
	bool DealDamageAoe(FVector InLocation, float Radius, float DamageAmount, EDamageType DamageType, int32 OwnTeam);

	UFUNCTION(BlueprintCallable, Category = "Scale")
	FSmbEntityData GetClosestEnemy(FVector Location, int32 TeamId, float Radius);
	//UFUNCTION()
	//TArray<FMassEntityHandle> GetNearbyUnits(FVector Location, float Radius);
	UFUNCTION()
	TArray<FMassEntityHandle> GetNumberClosestEntities(FVector Location, float Radius, int32 Amount);

	UFUNCTION()
	FVector RegisterToGrid(FVector NewLocation, FMassEntityHandle Handle, FVector OldLocation);

	UFUNCTION(BlueprintCallable, Category = "Scale")
	bool RegisterPhysicsManager(AScalePhysicsManager* InScalePhysicsManager, FString MeshName);
	UFUNCTION(BlueprintCallable, Category = "Scale")
	void NewDeath(FString MeshName, int32 TotalDeaths, TArray<FVector> Locations);

	UFUNCTION()
	void DestroyEntity(FMassEntityHandle Handle);
	

	UPROPERTY()
	FResourceReqMap ReqMap = FResourceReqMap();

	UPROPERTY()
	TMap<EProcessable, FProcessableArr> RegisteredResources;

	UPROPERTY(BlueprintReadWrite, Category = "Scale")
	TMap<EProcessable, int32> CarryingFree = TMap<EProcessable, int32>();

	UPROPERTY()
	TArray<FPhysicsManagerStruct> PhysicsManagers = TArray<FPhysicsManagerStruct>();
	
protected:

	UFUNCTION()
	void DestroyStalledEntity();
	
	UPROPERTY()
	UGrid* Grid = nullptr;

	TArray<FMassEntityHandle> ToDestroy = TArray<FMassEntityHandle>();
	
	FMassEntityManager* EntityManagerPtr = nullptr;
	UE::Mass::FEntityBuilder* EntityBuilder = nullptr;

	UPROPERTY()
	float CellSize = 50.f;
};
