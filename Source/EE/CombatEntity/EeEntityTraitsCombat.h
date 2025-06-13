// DefensiveEntityTrait.h
#pragma once

#include "CoreMinimal.h"
#include "EeCombatFragments.h"
#include "MassEntityTraitBase.h"
#include "EeEntityTraitsCombat.generated.h"

UCLASS()
class UDefensiveEntityTrait : public UMassEntityTraitBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category="Ee")
    FDefenceStatsParams DefenceStatsParams;

    UPROPERTY(EditAnywhere, Category="Ee")
    FDeadFragment DeathFragment;

    virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

UCLASS()
class UAttackTrait : public UMassEntityTraitBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category="Ee")
    FOffensiveStatsBase OffensiveStatsBase;

    UPROPERTY(EditAnywhere, Category="Ee")
    FOffensiveStatsParams OffensiveStatsParams;

    virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

UCLASS()
class UEeEntityTrait : public UMassEntityTraitBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category="Ee")
    FTeamFragment InTeam;

    virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

UCLASS()
class UProjectileTrait : public UMassEntityTraitBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category="Ee")
    int32 InTeam;

    UPROPERTY(EditAnywhere, Category="Ee")
    FProjectileParams ProjectileParams;
    
    UPROPERTY(EditAnywhere, Category="Ee")
    FProjectileVis ProjectileVis;
    
    virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
