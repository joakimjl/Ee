// DefensiveEntityTrait.cpp

#include "EeEntityTraitsCombat.h"

#include "CombatFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"

void UDefensiveEntityTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
    BuildContext.RequireFragment<FTransformFragment>();
    BuildContext.RequireFragment<FDefenceStatsBase>();
    BuildContext.AddFragment<FCombatFragment>();

    FMassEntityManager& MassEntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

    const FDefenceStatsParams SharedDefenceParams = DefenceStatsParams.GetValidated();
    const FConstSharedStruct& SharedDefenceParamsFragment = MassEntityManager.GetOrCreateConstSharedFragment(SharedDefenceParams);
    BuildContext.AddConstSharedFragment(SharedDefenceParamsFragment);
}