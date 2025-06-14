﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityElementTypes.h"

#include "EeCombatFragments.generated.h"


UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Idle,
	Attacking,
	Moving,
	Dead
};

UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Physical,
	Fire,
	Frost,
	Poison,
	Electric,
	Holy,
	Dark
};

USTRUCT()
struct FCombatFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Combat")
	ECombatState State = ECombatState::Idle;
};

USTRUCT()
struct FDefenceStatsParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	FDefenceStatsParams GetValidated() const
	{
		//Sets all Defence values to given ones
		FDefenceStatsParams Copy = *this;
		Copy.BaseHealth = FMath::Max(SMALL_NUMBER, BaseHealth);
		Copy.BaseHealthRegenRate = FMath::Max(SMALL_NUMBER, BaseHealthRegenRate);
		
		Copy.BaseShield = FMath::Max(SMALL_NUMBER, BaseShield);
		Copy.BaseShieldRegenRate = FMath::Max(SMALL_NUMBER, BaseShieldRegenRate);
		
		Copy.BaseArmor = FMath::Max(SMALL_NUMBER, BaseArmor);

		Copy.BaseMana = FMath::Max(SMALL_NUMBER, BaseMana);
		Copy.BaseManaRegenRate = FMath::Max(SMALL_NUMBER, BaseManaRegenRate);
		Copy.BaseManaCost = FMath::Max(SMALL_NUMBER, BaseManaCost);
		return Copy;
	}

	UPROPERTY(EditAnywhere, Category = "DefenceStat")
	float BaseHealth = 100.f;
	UPROPERTY(EditAnywhere, Category = "DefenceStat")
	float BaseHealthRegenRate = 0.5f;
	UPROPERTY(EditAnywhere, Category = "DefenceStat")
	float BaseShield = 0.f;
	UPROPERTY(EditAnywhere, Category = "DefenceStat")
	float BaseShieldRegenRate = 0.0f;
	UPROPERTY(EditAnywhere, Category = "DefenceStat")
	float BaseArmor = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "DefenceStat")
	float BaseMana = 50.f;
	UPROPERTY(EditAnywhere, Category = "DefenceStat")
	float BaseManaRegenRate = 1.0f;
	UPROPERTY(EditAnywhere, Category = "DefenceStat")
	float BaseManaCost = 1.0f;
};


USTRUCT()
struct FDefenceStatsBase : public FMassFragment
{
	GENERATED_BODY()

	float CurHealth = 100.f;
	float MaxHealth = 100.f;
	float HealthRegenRate = 0.5f;

	float CurShield = 0.f;
	float MaxShield = 0.f;
	float ShieldRegenRate = 0.0f;

	float CurArmor = 5.f;

	float CurMana = 50.f;
	float MaxMana = 50.f;
	float ManaRegenRate = 1.0f;
	
};

USTRUCT()
struct FOffensiveStatsParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	FOffensiveStatsParams GetValidated() const
	{
		//Sets all Attack values to given ones
		FOffensiveStatsParams Copy = *this;
		Copy.AttackRange = FMath::Max(SMALL_NUMBER, AttackRange);
		Copy.AttackDamage = FMath::Max(SMALL_NUMBER, AttackDamage);
		Copy.AttackSpeed = FMath::Max(SMALL_NUMBER, AttackSpeed);
		Copy.AttackAoe = FMath::Max(SMALL_NUMBER, AttackAoe);
		return Copy;
	}

	UPROPERTY(EditAnywhere, Category = "AttackStat")
	float AttackRange = 50.f;
	UPROPERTY(EditAnywhere, Category = "AttackStat")
	float AttackDamage = 10.f;
	UPROPERTY(EditAnywhere, Category = "AttackStat")
	float AttackSpeed = 0.75f;
	UPROPERTY(EditAnywhere, Category = "AttackStat")
	float AttackAoe = 30.f;
	
	UPROPERTY(EditAnywhere, Category = "AttackStat")
	EDamageType DamageType = EDamageType::Physical;
};

USTRUCT()
struct FOffensiveStatsBase : public FMassFragment
{
	GENERATED_BODY()

	FOffensiveStatsBase GetValidated() const
	{
		FOffensiveStatsBase Copy = *this;
		Copy.AttackSpeedMult = FMath::Max(SMALL_NUMBER, Copy.AttackSpeedMult);
		Copy.AttackDamageMult = FMath::Max(SMALL_NUMBER, Copy.AttackDamageMult);
		Copy.AttackRangeMult = FMath::Max(SMALL_NUMBER, Copy.AttackRangeMult);
		Copy.AttackAoeMult = FMath::Max(SMALL_NUMBER, Copy.AttackAoeMult);
		return Copy;
	}

	UPROPERTY(EditAnywhere, Category = "AttackStat")
	float AttackSpeedMult = 1.f;
	UPROPERTY(EditAnywhere, Category = "AttackStat")
	float AttackDamageMult = 1.f;
	UPROPERTY(EditAnywhere, Category = "AttackStat")
	float AttackRangeMult = 1.f;
	UPROPERTY(EditAnywhere, Category = "AttackStat")
	float AttackAoeMult = 1.f;
	
};

USTRUCT()
struct FTeamFragment : public FMassFragment
{
	GENERATED_BODY()

	FTeamFragment GetValidated() const
	{
		FTeamFragment Copy = *this;
		Copy.Team = FMath::Max(-1, Copy.Team);
		
		return Copy;
	}

	
	UPROPERTY(EditAnywhere, Category = "Team")
	int32 Team = -1;
};

USTRUCT()
struct FTransformDistChecker : public FMassFragment
{
	GENERATED_BODY()

	float DistSinceLocMark = 999999.f;
	FVector LastLocation = FVector::ZeroVector;
	bool FullyLerped = true;
	float TargetHeight = 0.f;
	FQuat TargetRotation = FQuat::Identity;

	float DistSinceZCheck = 999999.f;
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
	EDamageType DamageType = EDamageType::Physical;
};

USTRUCT()
struct FProjectileVis : public FMassSharedFragment
{
	GENERATED_BODY();

	/* Projectile Instanced Static Mesh */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	UInstancedStaticMeshComponent* ProjectileMeshComponent = nullptr;

	//Mesh SoftPointer
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSoftObjectPtr<UStaticMesh> ProjectileMesh;
};

//Projectile Tag
USTRUCT()
struct FProjectileTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FDamageFragment : public FMassFragment
{
	GENERATED_BODY();

	UPROPERTY()
	TMap<EDamageType, float> DamageMap = TMap<EDamageType, float>();
	
	UPROPERTY()
	FVector Impulse = FVector::ZeroVector;
};

USTRUCT()
struct FDamageTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FDeadTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FDeadFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Dead")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Dead")
	float DeathTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Dead")
	float Weight = 10.f;
};

