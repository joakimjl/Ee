// DefensiveEntityTrait.h
#pragma once

#include "CoreMinimal.h"
#include "CombatFragments.h"
#include "MassEntityTraitBase.h"
#include "EeEntityTraitsCombat.generated.h"

UCLASS()
class UDefensiveEntityTrait : public UMassEntityTraitBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category="Ee")
    FDefenceStatsParams DefenceStatsParams;

    virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};