// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScalableMassBehaviour.h"
#include "MassEntityElementTypes.h"

#include "ScaleFragments.generated.h"

struct FMassEntityHandle;

USTRUCT()
struct FResourceFragment : public FMassFragment
{
	GENERATED_BODY()

	FResourceFragment() = default;

	UPROPERTY(EditAnywhere, Category = "Scale")
	TMap<EProcessable, float> BonusMap = TMap<EProcessable, float>();

	UPROPERTY(EditAnywhere, Category = "Scale")
	TMap<EProcessable, int32> Carrying = TMap<EProcessable, int32>();
};

/** Animation States */
UENUM(BlueprintType)
enum class EAnimationState : uint8
{
	Idle UMETA(ToolTip = "Unit is currently Standing"),
	Attacking UMETA(ToolTip = "Unit is currently Attacking"),
	Running UMETA(ToolTip = "Unit is currently Running"),
	Dead UMETA(ToolTip = "Unit is currently Dead")
};

USTRUCT()
struct FAnimationFragment : public FMassFragment
{
	GENERATED_BODY()

	FAnimationFragment() = default;

	FAnimationFragment GetValidated() const
	{
		FAnimationFragment Copy = *this;
		Copy.AnimationUnitScale = FMath::Max(Copy.AnimationUnitScale, KINDA_SMALL_NUMBER);
		Copy.AnimOffsetTime = FMath::Max(Copy.AnimOffsetTime, KINDA_SMALL_NUMBER);
		Copy.AnimationSpeed = FMath::Max(Copy.AnimationSpeed, KINDA_SMALL_NUMBER);

		return Copy;
	}
	/* Current State as given from StateTree Tasks (Change this for new Task Started) */
	UPROPERTY()
	EAnimationState CurrentState = EAnimationState::Idle;

	/* Previous State that is being transitioned from (Only to be changed from Animation Processor) */
	UPROPERTY()
	EAnimationState PreviousState = EAnimationState::Idle;
	
	/* To scale Vertex Animation. Should be the same scale as unit Mesh. */
	UPROPERTY(EditAnywhere, Category = "Scale")
	float AnimationUnitScale = 1.f;
	/* Offset Timing i.e., changes timing so ISM material instance starts on the correct time */
	UPROPERTY(EditAnywhere, Category = "Scale")
	float AnimOffsetTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Scale")
	float TimeInCurrentAnimation = 0.f;

	UPROPERTY(EditAnywhere, Category = "Scale")
	float LerpAlpha = 0.f;
	
	UPROPERTY()
	FName AnimationName = FName("None");

	UPROPERTY()
	float CurrentAnimationFrame = 0.f;

	UPROPERTY()
	float PreviousAnimationFrame = 0.f;

	UPROPERTY()
	float PrevStart = 0.f;

	UPROPERTY()
	float PrevEnd = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "Scale")
	float AnimationSpeed = 1.f;
};


USTRUCT()
struct FVertexAnimations : public FMassSharedFragment
{
	GENERATED_BODY()

	FVertexAnimations() = default;

	FVertexAnimations GetValidated() const
	{
		FVertexAnimations Copy = *this;

		return Copy;
	}

	UPROPERTY(EditAnywhere, Category = "Scale")
	TMap<EAnimationState,UAnimSequence*> AnimSequences = TMap<EAnimationState,UAnimSequence*>();
};

/** Types of Armour */
UENUM(BlueprintType)
enum class EArmourType : uint8
{
	HeavyArmour UMETA(ToolTip = "Unit is using Heavy Armour, weak to piercing"),
	LightArmour UMETA(ToolTip = "Unit is using Light Armour, weak to slashing"),
	MediumArmour UMETA(ToolTip = "Unit is using Medium Armour, weak to blunt"),
	None UMETA(ToolTip = "Unit is using Normal Armour")
};

USTRUCT()
struct FDefenceFragment : public FMassFragment
{
	GENERATED_BODY()

	FDefenceFragment() = default;

	FDefenceFragment GetValidated() const
	{
		FDefenceFragment Copy = *this;
		Copy.MaxHP = FMath::Max(Copy.MaxHP, KINDA_SMALL_NUMBER);
		Copy.HP = FMath::Max(Copy.HP, KINDA_SMALL_NUMBER);

		return Copy;
	}

	/* Max Health */
	UPROPERTY(EditAnywhere, Category = "Scale")
	float MaxHP = 20.f;

	/* Current Health */
	UPROPERTY(EditAnywhere, Category = "Scale")
	float HP = 20.f;

	/* Armor Type */
	UPROPERTY(EditAnywhere, Category = "Scale")
	EArmourType UnitArmour = EArmourType::MediumArmour;
};

USTRUCT()
struct FTeamFragment : public FMassFragment
{
	GENERATED_BODY()

	FTeamFragment() = default;

	FTeamFragment GetValidated() const
	{
		FTeamFragment Copy = *this;
		Copy.TeamID = FMath::Max(Copy.TeamID, -1);

		return Copy;
	}

	/* Team ID */
	UPROPERTY(EditAnywhere, Category = "Scale")
	int32 TeamID = 0;
};


UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Normal,
	Piercing,
	Slashing,
	Blunt
};

USTRUCT()
struct FAttackFragment : public FMassFragment
{
	GENERATED_BODY()

	FAttackFragment() = default;

	FAttackFragment GetValidated() const
	{
		FAttackFragment Copy = *this;
		Copy.AttackDamage = FMath::Max(Copy.AttackDamage, KINDA_SMALL_NUMBER);
		Copy.AttackRate = FMath::Max(Copy.AttackRate, KINDA_SMALL_NUMBER);
		Copy.AttackRange = FMath::Max(Copy.AttackRange, KINDA_SMALL_NUMBER);
		Copy.AnimationDelayUntilDamage = FMath::Max(Copy.AnimationDelayUntilDamage, KINDA_SMALL_NUMBER);
		Copy.AttackRecoveryTime = FMath::Max(Copy.AttackRecoveryTime, KINDA_SMALL_NUMBER);

		return Copy;
	}

	/* Attack damage from unit standard attack */
	UPROPERTY(EditAnywhere, Category = "Scale")
	float AttackDamage = 20.f;

	/* How long in seconds until it deals damage */
	UPROPERTY(EditAnywhere, Category = "Scale")
	float AnimationDelayUntilDamage = 0.2f;

	/* Attack Speed, in attacks per second */
	UPROPERTY(EditAnywhere, Category = "Scale")
	float AttackRate = 1.f;

	/* How long after hit does animation last (Attack Speed timer starts after this) */
	UPROPERTY(EditAnywhere, Category = "Scale")
	float AttackRecoveryTime = 0.5f;

	/* Will allow more than one damage instance (Currently WIP) */
	UPROPERTY(EditAnywhere, Category = "Scale")
	int32 DamageInstances = 1;

	UPROPERTY()
	int32 CurDamageInstance = 0;

	/* How long until it can attack again currently */
	UPROPERTY()
	float TimeLeftToAttack = 0.f;

	UPROPERTY()
	float CurrentTimeIntoTheAttack = 0.f;

	/* Attack range, as radius from location */ 
	UPROPERTY(EditAnywhere, Category = "Scale")
	float AttackRange = 30.f;

	/* Damage Type */
	UPROPERTY(EditAnywhere, Category = "Scale")
	EDamageType DamageType = EDamageType::Normal;
};



USTRUCT()
struct FProjectileFragment : public FMassFragment
{
	GENERATED_BODY()
	
	FVector Velocity = FVector::ZeroVector;
};


USTRUCT()
struct FProjectileParams : public FMassConstSharedFragment
{
	GENERATED_BODY();

	FProjectileParams GetValidated() const
	{
		FProjectileParams Copy = *this;
		Copy.InitialSpeed = FMath::Max(0.f,Copy.InitialSpeed);
		Copy.InitialDirection = Copy.InitialDirection.GetSafeNormal();
		return Copy;
	}

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float InitialSpeed = 0;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	FVector InitialDirection = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, Category = "Projectile")
	EDamageType DamageType = EDamageType::Normal;
};

USTRUCT()
struct FProjectileVis : public FMassSharedFragment
{
	GENERATED_BODY();

	/* Projectile Instanced Static Mesh */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TObjectPtr<UInstancedStaticMeshComponent> ProjectileMeshComponent = TObjectPtr<UInstancedStaticMeshComponent>();

	//Mesh SoftPointer
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSoftObjectPtr<UStaticMesh> ProjectileMesh;
};

USTRUCT()
struct FProjectileTag : public FMassTag
{
	GENERATED_BODY()
};


USTRUCT()
struct FLocationDataFragment : public FMassFragment
{
	GENERATED_BODY()

	FLocationDataFragment() = default;

	FLocationDataFragment GetValidated() const
	{
		FLocationDataFragment Copy = *this;
		Copy.BaseRefresh = FMath::Max(Copy.BaseRefresh, 0.f);

		return Copy;
	}

	/* Old Entity Location In Grid */
	UPROPERTY()
	FVector OldLocation = FVector::ZeroVector;

	/* Time Since Last */
	UPROPERTY()
	float TimeSince = 0.f;

	/* How frequently should entity update position in the grid (Base only larger if lower LOD) */
	UPROPERTY(EditAnywhere, Category = "Scale")
	float BaseRefresh = 1.f;
};

USTRUCT()
struct FCollisionDataFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Scale")
	float CheckDelay = 1.f;
	UPROPERTY()
	float TimeSinceLastCheck = 0.f;

	UPROPERTY(EditAnywhere, Category = "Scale")
	int32 MaxEntitiesToCheck = 5;

	UPROPERTY()
	TArray<FMassEntityHandle> ClosestEntities;

	/* Weight to collide with */
	UPROPERTY(EditAnywhere, Category = "Scale")
	float CollisionMass = 10.f;
};

USTRUCT()
struct FDeathPhysicsFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	FDeathPhysicsFragment GetValidated() const
	{
		FDeathPhysicsFragment Copy = *this;
		Copy.TotalDeaths = FMath::Max(Copy.TotalDeaths, 0);

		return Copy;
	}

	UPROPERTY()
	TArray<FVector> DeathLocations = TArray<FVector>();

	/* Total Deaths (Doesn't need to be changed) */
	UPROPERTY(EditAnywhere, Category = "Scale")
	int32 TotalDeaths = 0;
};