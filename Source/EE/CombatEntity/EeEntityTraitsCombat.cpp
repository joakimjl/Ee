// DefensiveEntityTrait.cpp

#include "EeEntityTraitsCombat.h"

#include "CombatFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"

void UDefensiveEntityTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
    BuildContext.RequireFragment<FTransformFragment>();
    BuildContext.AddFragment<FDefenceStatsBase>();
    BuildContext.AddFragment<FCombatFragment>();
    BuildContext.AddFragment<FDamageFragment>();
    FDeadFragment& DeadFragRef = BuildContext.AddFragment_GetRef<FDeadFragment>();
    DeadFragRef = DeathFragment;

    FMassEntityManager& MassEntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

    const FDefenceStatsParams SharedDefenceParams = DefenceStatsParams.GetValidated();
    const FConstSharedStruct& SharedDefenceParamsFragment = MassEntityManager.GetOrCreateConstSharedFragment(SharedDefenceParams);
    BuildContext.AddConstSharedFragment(SharedDefenceParamsFragment);
}

void UEeEntityTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
    FTeamFragment& TeamFrag = BuildContext.AddFragment_GetRef<FTeamFragment>();
    TeamFrag.Team = InTeam;
    
    BuildContext.AddFragment<FTransformDistChecker>();
    BuildContext.RequireFragment<FTransformFragment>();
}

void UProjectileTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
    FMassEntityManager& MassEntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
    
    const FProjectileParams InsideProjectileParams = ProjectileParams.GetValidated();
    const FConstSharedStruct& SharedProjectileParamsFragment = MassEntityManager.GetOrCreateConstSharedFragment(InsideProjectileParams);
    BuildContext.AddConstSharedFragment(SharedProjectileParamsFragment);

    const FProjectileVis ProjectileVisIn = ProjectileVis;
    const FSharedStruct& SharedProjectileVisFragment = MassEntityManager.GetOrCreateSharedFragment(ProjectileVisIn);
    BuildContext.AddSharedFragment(SharedProjectileVisFragment);

    BuildContext.AddFragment<FProjectileFragment>();
    FTeamFragment& TeamFrag = BuildContext.AddFragment_GetRef<FTeamFragment>();
    TeamFrag.Team = InTeam;
    
    BuildContext.RequireFragment<FTransformFragment>();
}