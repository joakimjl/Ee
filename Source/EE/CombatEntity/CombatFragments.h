#pragma once


#include "MassEntityTypes.h"
#include "MassEntityElementTypes.h"

#include "CombatFragments.generated.h"

USTRUCT()
struct FCombatFragment : public FMassFragment
{
	//FMassEntityQuery EntityQuery;
	GENERATED_BODY()

	
};

USTRUCT()
struct FProjectileFragment : public FMassFragment
{
	//FMassEntityQuery EntityQuery;
	GENERATED_BODY()
	
	FVector Velocity = FVector::ZeroVector;

	
};

USTRUCT()
struct FProjectileParams : public FMassSharedFragment
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

	
};


